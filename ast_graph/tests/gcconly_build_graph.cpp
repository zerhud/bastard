/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include "factory.hpp"
#include "ast_graph/graph.hpp"

#include <memory>

using ast_graph_tests::factory;

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

constexpr auto mk_test_fields(int cnt) {
	test_fields ret{ 0, 1, {} };
	ret.leafs.resize(cnt);
	for(auto i=0;i<cnt;++i) ret.leafs[i].ff = i+1;
	return ret;
}

constexpr auto mk_test_graph(auto&&... args) {
	auto g =  ast_graph::mk_graph(factory{}, std::forward<decltype(args)>(args)...);
	auto v = g.create_view();
	return std::tuple( std::move(g), std::move(v) );
}


static_assert( ast_graph::mk_graph_calc_size( factory{}, test_fields{} ) == 2 );
static_assert( ast_graph::mk_graph( factory{}, test_fields{} ).size() == 2 );
static_assert( ast_graph::mk_graph( factory{}, mk_test_fields(2) ).size() == 6 );
static_assert( []{
	auto[ g, v ] = mk_test_graph(mk_test_fields(2));
	return children_of(v, v.root()).size();
}() == 1 );
static_assert( ast_graph::mk_graph( factory{}, mk_test_fields(2) ).create_view().root()->is_array() == false );
static_assert( []{
	auto[ g, v ] = mk_test_graph(mk_test_fields(2));
	return children_of(v, v.root())[0]->is_array();
}() == true);
static_assert( []{
	auto[ g, v ] = mk_test_graph(mk_test_fields(2));
	auto children = ast_links_of(v, v.root());
	auto& c0 = children[0];
	auto cn = ast_links_of(v, c0.child);
	return
	  (c0.parent == v.root()) +
	2*(c0.name == "leafs") +
	4*(cn.size() == 2) +
	8*(cn[0].name == "")
	;
}() == 15);
static_assert( []{
	auto src = mk_test_fields(2);
	src.leafs[1].ff = 3;
	src.leafs[0].ff = 2;
	src.leafs[0].vl.emplace<1>().v2f = 7;

	auto[ g, v ] = mk_test_graph(src);
	auto children = ast_links_of(v, v.root());
	auto& c0 = children[0];
	auto cn = ast_links_of(v, c0.child);

	return
	  (cn.size() == 2) +
	2*(cn[1].child->field("ff") == factory::data_type{3}) +
	4*(ast_links_of(v, cn[0].child)[0].name == "vl") +
	8*(ast_links_of(v, cn[0].child)[0].child->field("v2f") == factory::data_type{7})
	;
}() == 15);

static_assert( []{
	auto[ g, v ] = mk_test_graph(mk_test_fields(2));
	return v == v;
}(), "can compare graph view with operator==" );
static_assert( []{
	auto src = mk_test_fields(2);
	auto[ g1, v1 ] = mk_test_graph(src);
	auto[ g2, v2 ] = mk_test_graph(src);
	return v1 == v2;
}(), "can compare graph view with operator== on same source" );
static_assert( []{
	auto[ g1, v1 ] = mk_test_graph(mk_test_fields(2));
	auto[ g2, v2 ] = mk_test_graph(mk_test_fields(2));
	return v1 != v2;
}(), "can compare graph view with operator!=" );

int main(int,char**) {
	auto src = mk_test_fields(2);
	auto g = ast_graph::mk_graph(factory{}, src);
	std::cout << "debug: " << g.root().base->children[0].name << std::endl;
	std::cout << "debug: " << g.root().base->children[0].vertex->debug_info() << std::endl;
	return 0;
}
