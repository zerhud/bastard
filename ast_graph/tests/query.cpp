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
	std::vector<no_child> child_vec;
};

} // namespace test

struct factory : ast_graph_tests::factory{
	using field_name_type = std::string_view;
	using data_type = int;
};


int main(int,char**) {

#if (__cplusplus > 202400)
	static_assert( []{
		test::top src;
		src.field1 = 42;
		auto res = ast_graph::query(factory{}, src, "" );
		return static_cast<const test::top*>(res.root->data)->field1;
	}() == 42 );
#endif

	CTRT( []{
		test::top src;
		src.child1.field1 = 42;
		auto res = ast_graph::query(factory{}, src, "");
		auto* found = child(res, res.root, 0);
		return found->info.field("field1") ;//* ( found->info.field("field1") == static_cast<const test::no_child*>(found->data)->field1 );
	}() == 42 );

	CTRT( []{
		test::top src;
		src.child_vec.emplace_back(42, 43);
		auto res = ast_graph::query(factory{}, src, "");
		auto* found = child(res, child(res, res.root, 1), 0);
		return found->info.field("field2");
	}() == 43 );

	return 0;
}