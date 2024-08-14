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


static_assert( ast_graph::mk_graph_calc_size( factory{}, test_fields{} ) == 2 );
static_assert( ast_graph::mk_graph( factory{}, test_fields{} ).size() == 2 );
static_assert( ast_graph::mk_graph( factory{}, mk_test_fields(2) ).size() == 6 );
static_assert( ast_graph::mk_graph( factory{}, mk_test_fields(2) ).front().base->children.size() == 1 );
static_assert( ast_graph::mk_graph( factory{}, mk_test_fields(2) ).front().base->is_array() == false );
static_assert( ast_graph::mk_graph( factory{}, mk_test_fields(2) ).front().base->children[0].vertex->is_array() == true );
static_assert( []{
	auto g = ast_graph::mk_graph( factory{}, mk_test_fields(2) );
	return
	  (g.front().base->children[0].vertex->parent == g.front().base) +
	2*(g.front().base->children[0].name == "leafs") +
	4*(g.front().base->children[0].vertex->children.size() == 2) +
	8*(g.front().base->children[0].vertex->children[0].name == "")
	;
}() == 15 );
static_assert( []{
	auto src = mk_test_fields(2);
	src.leafs[1].ff = 3;
	src.leafs[0].ff = 2;
	src.leafs[0].vl.emplace<1>().v2f = 7;
	auto g = ast_graph::mk_graph( factory{}, src );
	return
	  (g.front().base->children[0].vertex->size() == 2) +
	2*(g.front().base->children[0].vertex->children[1].vertex->field("ff") == factory::data_type{3}) +
	4*(g.front().base->children[0].vertex->children[0].vertex->children[0].name == "vl") +
	8*(g.front().base->children[0].vertex->children[0].vertex->children[0].vertex->field("v2f") == factory::data_type{7})
	;
}() == 15 );
static_assert( []{
	auto src = mk_test_fields(2);
	auto g = ast_graph::mk_graph(factory{}, src);
	return
	  (g.front().base->children[0].name == "leafs") +
	2*(g.front().base->children[0].vertex->size()==2)
	;
}() == 3 );

int main(int,char**) {
	auto src = mk_test_fields(2);
	auto g = ast_graph::mk_graph(factory{}, src);
	std::cout << "debug: " << g.front().base->children[0].name << std::endl;
	std::cout << "debug: " << g.front().base->children[0].vertex->debug_info() << std::endl;
	return 0;
}
