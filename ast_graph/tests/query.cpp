/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "ast_graph/query.hpp"
#include "factory.hpp"

namespace test {

struct no_child {
	int field1 = 1;
	int field2 = 2;
};

struct top {
	int field1 = 1;
	no_child child1;
};

} // namespace test

using ast_graph_tests::factory;

static_assert( []{
	test::top src;
	src.field1 = 42;
	auto res = ast_graph::query(factory{}, src, "" );
	return static_cast<const test::top*>(res.root->data)->field1;
}() == 42 );

static_assert( []{
	test::top src;
	src.child1.field1 = 42;
	auto res = ast_graph::query(factory{}, src, "");
	auto* found = child(res, res.root, 0);
	//return static_cast<const test::no_child*>(found->data)->field1 * (found->index == index_of<test::no_child>(*found));
	return exec(*found, [](auto* v){return v->field1;});
}() == 42 );

int main(int,char**) {
	return 0;
}