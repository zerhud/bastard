/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include "tests/factory.hpp"

#include "absd.hpp"
#include "ast_graph/graph.hpp"

struct inner_factory : tests::factory {
	using data_type = absd::data<inner_factory>;
};

struct variant_leaf1 { int v1f = 0; };
struct variant_leaf2 { int v2f = 1; };
struct pointer_leaf { int pf = 3; };
struct test_leaf{
	int ff=3;
	std::variant<variant_leaf1, variant_leaf2> vl;
	std::unique_ptr<pointer_leaf> pl;
};
static_assert( tref::gs::size<test_leaf> == 3 );

struct test_fields {
	int field_1=1;
	int field_2=2;
	std::vector<test_leaf> leafs;
};

struct fixture {
	using graph_type = decltype(ast_graph::mk_graph(inner_factory{}, test_fields{}));
	using view_type = decltype(std::declval<graph_type>().create_view());

	inner_factory f;
	test_fields src;
	graph_type g;
	view_type v = g.create_view();

	constexpr fixture() : g(ast_graph::mk_graph(f, src)) {}

	constexpr static auto create_test_fields(int cnt) {
		test_fields ret{ 0, 1, {} };
		ret.leafs.resize(cnt);
		for(auto i=0;i<cnt;++i) ret.leafs[i].ff = i+1;
		return ret;
	}
	constexpr auto& mk_test_fields(int cnt) {
		src = create_test_fields(cnt);
		return *this;
	}

	constexpr auto mk_test_graph() {
		g = ast_graph::mk_graph(f, src);
		v = g.create_view();
		return std::tie(g,v);
	}
};

template<typename test>
struct test_fixture : fixture, test{
	using test::operator();
	constexpr test_fixture(test&& t) : test(std::forward<decltype(t)>(t)) {}
	constexpr auto operator()() {
		return (*this)(*this);
	}
};


static_assert(ast_graph::mk_graph_calc_size(inner_factory{}, test_fields{} ) == 2 );
static_assert(ast_graph::mk_graph(inner_factory{}, test_fields{} ).size() == 2 );
static_assert(ast_graph::mk_graph(inner_factory{}, fixture{}.mk_test_fields(2).src ).size() == 6 );
static_assert( test_fixture{[](auto& f){ return f.v.size(); }}() == 2 );
static_assert( test_fixture{[](auto& f){
	auto [_,v] = f.mk_test_fields(2).mk_test_graph();
	return v.size();
}}() == 6 );
static_assert( test_fixture{[](auto& f){
	auto [g,v] = f.mk_test_fields(2).mk_test_graph();
	return v.ast_links_of(g.root().base).size();
}}() == 1 );
static_assert(ast_graph::mk_graph(inner_factory{}, fixture{}.mk_test_fields(2).src ).create_view().root()->is_array() == false );
static_assert( test_fixture{[](auto& f){
	auto [g,v] = f.mk_test_fields(2).mk_test_graph();
	return v.ast_links_of(g.root().base)[0].child->is_array();
}}() == true );
static_assert( test_fixture{[](auto& f){
	auto [_,v] = f.mk_test_fields(2).mk_test_graph();
	auto children = v.ast_links_of(v.root());
	auto& c0 = children[0];
	auto cn = v.ast_links_of(c0.child);
	return
	  (c0.parent == v.root()) +
	2*(c0.name == "leafs") +
	4*(cn.size() == 2) +
	8*(cn[0].name == "")
	;
}}() == 15 );
static_assert( test_fixture{[](auto& f){
	f.mk_test_fields(2);
	f.src.leafs[1].ff = 3;
	f.src.leafs[0].ff = 3;
	f.src.leafs[0].vl.template emplace<1>().v2f = 7;

	auto[ g, v ] = f.mk_test_graph();
	auto children = v.ast_links_of(v.root());
	auto& c0 = children[0];
	auto cn = v.ast_links_of(c0.child);

	return
	  (cn.size() == 2) +
	2*(cn[1].child->field("ff") == inner_factory::data_type{3}) +
	4*(v.ast_links_of(cn[0].child)[0].name == "vl") +
	8*(v.ast_links_of(cn[0].child)[0].child->field("v2f") == inner_factory::data_type{7})
	;
}}() == 15 );

static_assert( test_fixture{[](auto& f){
	auto [_,v] = f.mk_test_fields(2).mk_test_graph();
	return v == v;
}}(), "can compare graph view with operator==" );
static_assert( test_fixture{[](auto& f){
	auto& src = f.mk_test_fields(2).src;
	auto g1 = ast_graph::mk_graph(f.f, src);
	auto g2 = ast_graph::mk_graph(f.f, src);
	return g1.create_view() == g2.create_view();
}}(), "can compare graph view with operator== on same source");
static_assert( test_fixture{[](auto& f){
	auto src1 = f.create_test_fields(2);
	auto src2 = f.create_test_fields(2);
	auto g1 = ast_graph::mk_graph(f.f, src1);
	auto g2 = ast_graph::mk_graph(f.f, src2);
	return g1.create_view() != g2.create_view();
}}(), "can compare graph view with operator!=" );
static_assert( test_fixture{[](auto& f){
	auto [g,_] = f.mk_test_fields(2).mk_test_graph();
	return g.links_of(g.links_of(g.root().base)[0].child).size();
}}() == 2);
static_assert( test_fixture{[](auto& f){
	auto [g,v] = f.mk_test_fields(2).mk_test_graph();
	auto vs = g.create_empty_view();
	vs.add_vertex(g.root().base);
	auto& leafs = vs.add_vertex(g.links_of(g.root().base)[0].child);
	vs.add_vertex(g.links_of(&leafs)[0].child);
	return (vs.size() == 3) + 2*(vs.ast_links_of(&leafs).size() == 1);
}}() == 3, "get vertices only added to view, not all present in graph");

static_assert( test_fixture{[](auto& f){
	auto [g,v] = f.mk_test_fields(2).mk_test_graph();
	auto vt = g.create_empty_view();
	vt += v;
	return v.size() == vt.size();
}}() == true );
static_assert( test_fixture{[](auto& f){
	auto [g,v] = f.mk_test_fields(2).mk_test_graph();
	auto vt = g.create_empty_view();
	vt += v;
	return v.size() == vt.size();
}}() == true, "test operator+=" );
static_assert( test_fixture{[](auto& f){
	auto [g,v] = f.mk_test_fields(2).mk_test_graph();
	auto vt = g.create_empty_view();
	auto orig_size = v.size();
	vt.add_vertex(g.root().base);
	v -= vt;
	return v.size() == orig_size-1;
}}() == true, "test operator-=" );

int main(int,char**) {
	auto src = fixture{}.create_test_fields(2);
	auto g = ast_graph::mk_graph(inner_factory{}, src);
	auto v = g.create_view();
	std::cout << "debug: " << v.ast_links_of(v.root())[0].name << std::endl;
	std::cout << "debug: " << v.ast_links_of(v.root())[0].child->debug_info() << std::endl;
	return 0;
}
