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
	//TODO: cannot use node{}.value() with the unique_ptr
	//      the copy ctor is deleted and it cannot to be copy to
	//      value() result's variant
	//std::unique_ptr<pointer_leaf> pl{};

	//constexpr test_leaf() =default ;
	//constexpr test_leaf(int ff) : ff(ff) {}
};
static_assert( tref::gs::size<test_leaf> == 2 );

struct test_fields {
	int field_1=1;
	int field_2=2;
	std::vector<test_leaf> leafs;
};


static_assert( ast_graph::mk_graph_calc_size( factory{}, test_fields{} ) == 1 );
static_assert( ast_graph::mk_graph( factory{}, test_fields{} ).size() == 1 );
static_assert( ast_graph::mk_graph( factory{}, test_fields{0,1,{{},{}}} ).size() == 5 );
static_assert( ast_graph::mk_graph( factory{}, test_fields{0,1,{{},{}}} ).front().base->children.size() == 2 );
static_assert( []{
	auto g = ast_graph::mk_graph( factory{}, test_fields{0,1,{{},{}}} );	
	return
	  (g.front().base->children[0].vertex->parent == g.front().base) +
	2*(g.front().base->children[0].name == "leafs") +
	4*(g.front().base->children.size() == 2) +
	8*(g.front().base->children[1].name == "leafs")
	;
}() == 15 );
static_assert( []{
	test_fields src{0, 1, {{2,{}}, {3,{}}}};
	auto g = ast_graph::mk_graph( factory{}, src );	
	return
	  (g.front().base->children[1].name == "leafs") +
	2*(g.front().base->children[1].vertex->field("ff").is_int()) +
	4*(g.front().base->children[1].vertex->field("ff") == factory::data_type{3}) +
	8*(g.front().base->children[0].vertex->field("ff") == factory::data_type{2})
	;
}() == 15 );
/*
*/

int main(int,char**) {
	return 0;
}
