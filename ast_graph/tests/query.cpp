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

#include <optional>

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

	template<typename type> using forward_ast = std::unique_ptr<type>;
	template<typename type> using optional = std::optional<type>;
	using string_t = std::string;
	using string_type = std::string;
};

int main(int,char**) {

	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "{:ident}");
		return get<0>(result.data).data[0].value;
	}() == "ident");
	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "3{:ident}[a:b]");
		return (result.input_number==3) + 2*(get<0>(result.data).data[0].value=="ident") + 4*(get<1>(result.next->data).data[0].value=="b") + 8*(get<1>(result.next->data).data[0].name=="a");
	}() == 15);
	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "3{*}[*]");
		return (result.input_number==3) + 2*(get<0>(result.data).data.empty()) + 4*(get<1>(result.next->data).data.empty()) + 8*(get<1>(result.next->data).data.empty());
	}() == 15);
	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "3{:ident}[a:b,c:'::']");
		return (get<1>(result.next->data).data[0].value=="b") + 2*(get<1>(result.next->data).data[0].name=="a") + 4*(get<1>(result.next->data).data[1].name=="c") + 8*(get<1>(result.next->data).data[1].value=="::");
	}() == 15);

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
