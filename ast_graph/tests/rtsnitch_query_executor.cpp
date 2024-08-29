/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#define SNITCH_IMPLEMENTATION
#if __has_include(<snitch_all.hpp>)
#include <snitch_all.hpp>
#else
#include <snitch/snitch.hpp>
#endif

#include <vector>
#include <variant>

#include "absd/iostream_op.hpp"
#include "ast_graph/query_executor.hpp"
#include "ast_graph/graph.hpp"
#include "ast_graph/graph_absd.hpp"
#include "factory.hpp"
#include "ascip.hpp"

namespace test_data {
struct variant_leaf1 {
	int v1f = 0;
};
struct variant_leaf2 {
	int v2f = 1;
};
struct pointer_leaf {
	int pf = 3;
};
struct test_leaf {
	int ff = 3;
	std::variant<variant_leaf1, variant_leaf2> vl;
	std::unique_ptr<pointer_leaf> pl;
};
static_assert(tref::gs::size<test_leaf> == 3);

struct test_fields {
	int field_1 = 1;
	int field_2 = 2;
	std::vector<test_leaf> leafs;
};

} // namespace test_data

struct factory : ast_graph_tests::query_factory{
	using data_type = ast_graph_tests::factory::data_type;
	using parser = ascip<std::tuple>;
};

constexpr auto parse(const auto& orig, auto&& src) {
}

using data_type = typename factory::data_type;
using graph_type = decltype(ast_graph::mk_graph(factory{}, test_data::test_fields{}));

namespace absd {

template<typename factory>
bool append(snitch::small_string_span ss, const data<factory>& obj) {
	std::string buf;
	back_insert_format(std::back_inserter(buf), obj);
	return append(ss, buf);
}

template<typename factory>
constexpr auto to_str(const data<factory>& obj) {
	std::string buf;
	back_insert_format(std::back_inserter(buf), obj);
	return buf;
}

} // namespace abds

struct parse_fixture {
	factory f;
	test_data::test_fields src_st;
	graph_type graph = ast_graph::mk_graph(f, src_st);

	void update_graph() {
		graph = ast_graph::mk_graph(f, src_st);
	}

	auto mk_executor() {
		return ast_graph::query_executor( f, src_st );
	}
	auto mk_obj(auto* g) {
		return data_type::mk(ast_graph::graph_absd(f, g));
	}

	auto mk_obj() {
		return mk_obj(graph[0].base);
	}

	auto mk_graph_str() {
		return to_str(mk_obj());
	}
};

TEST_CASE_METHOD(parse_fixture, "can_compare", "[graph][query]") {
	CHECK( mk_obj() == mk_obj() );
	graph_type graph2 = ast_graph::mk_graph(f, src_st);
	REQUIRE( mk_obj() == mk_obj(graph2[0].base) );
}

TEST_CASE_METHOD(parse_fixture, "empty_query_is_whole_graph", "[graph][query]") {
	auto q = mk_executor();
	//auto r = q("{}->{}");
//	REQUIRE( mk_obj() == mk_obj(r[0].base) );
}

/*
TEST_CASE_METHOD(parse_fixture, "all_nodes_without_children", "[graph][query]") {
	src_st.leafs.emplace_back();
	update_graph();
	auto q = mk_executor();
	auto r = q("{}");
	CHECK( mk_obj(r[0].base).keys().size() == 2 );
	REQUIRE( mk_obj() != mk_obj(r[0].base) );
}
*/
