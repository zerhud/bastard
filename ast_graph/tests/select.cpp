/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "ast_graph/query.hpp"
#include "factory.hpp"
#include "ascip.hpp"
#include "ast_graph/absd_object.hpp"

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

struct factory : ast_graph_tests::query_factory{ };

constexpr auto parse(auto&& src) {
	return ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, std::forward<decltype(src)>(src));
}

static_assert( []{
	auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "{:42}");
	return 1;
}() == 1 );

int main(int,char**) {
	ast_graph::details::query<factory> result;
	//auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "{:42}");
	return 0;
}
