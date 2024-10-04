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
#include "jiexpr.hpp"
#include "jiexpr/default_operators.hpp"
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

struct factory : ast_graph_tests::query_factory {
	using data_type = ast_graph_tests::factory::data_type;
	using jiexpr_test = jiexpr<data_type, jiexpr_details::expr_operators_simple, factory>;
	using parser = ascip<std::tuple>;

	template<typename... types> using variant_t = std::variant<types...>;
	template<typename type> using ast_forwarder = std::unique_ptr<type>;
	template<typename type> using vec_type = std::vector<type> ;

	constexpr auto mk_fwd(auto& v) const {
		using v_type = std::decay_t<decltype(v)>;
		static_assert( !std::is_pointer_v<v_type>, "the result have to be a unique_ptr like type" );
		static_assert( !std::is_reference_v<v_type>, "the result have to be a unique_ptr like type" );
		v = std::make_unique<typename v_type::element_type>();
		return v.get();
	}
	constexpr auto mk_result(auto&& v) const {
		using expr_t = std::decay_t<decltype(v)>;
		return std::make_unique<expr_t>(std::move(v));
	}
	constexpr auto mk_str() const {
		return std::string{};
	}
	constexpr auto back_inserter(auto& v) const {
		return std::back_inserter(v);
	}
};

using data_type = typename factory::data_type;
using graph_type = decltype(ast_graph::mk_graph(factory{}, test_data::test_fields{}));

struct parser_factory {
	data_type env;
	factory::jiexpr_test* jiexpr;

	constexpr friend const auto& vertex_expression(const parser_factory& f) { return *f.jiexpr; }
	constexpr friend auto solve_vertex(parser_factory& self, const auto& f, const auto& expr, const auto* graph) {
		data_type env;
		self.jiexpr->env = &env;
		env.mk_empty_object();
		env.put(data_type{"v"}, data_type::mk(ast_graph::graph_absd(f, graph)));
		return (*self.jiexpr)(expr);
	}
};

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
	data_type jiexpr_env;
	factory::jiexpr_test jiexpr;

	parser_factory pf;

	constexpr parse_fixture() {
		pf.jiexpr = &jiexpr;
	}

	constexpr void update_graph() {
		graph = ast_graph::mk_graph(f, src_st);
	}

	constexpr auto mk_executor() {
		jiexpr.env = &jiexpr_env;
		return ast_graph::query_executor( f, &pf, src_st );
	}
	constexpr auto mk_obj(auto* g) {
		return data_type::mk(ast_graph::graph_absd(f, g));
	}

	constexpr auto mk_obj() {
		return mk_obj(graph.root().base);
	}

	constexpr auto mk_graph_str() {
		return to_str(mk_obj());
	}
};
static_assert( parse_fixture{}.mk_executor()("{v.field_1==1}").size() == 1 );

TEST_CASE_METHOD(parse_fixture, "can_compare", "[graph][query]") {
	CHECK( mk_obj() == mk_obj() );
	graph_type graph2 = ast_graph::mk_graph(f, src_st);
	REQUIRE( mk_obj() == mk_obj(graph2.root().base) );
}

TEST_CASE_METHOD(parse_fixture, "empty_query_is_whole_graph", "[graph][query]") {
	auto q = mk_executor();
	auto r = q("{}");
	REQUIRE( r.size() > 0 );
	REQUIRE( mk_obj() == mk_obj(r.root()) );
}
TEST_CASE_METHOD(parse_fixture, "query_false_returns_nothing", "[graph][query]") {
	auto empty =  mk_executor()("{false}");
	CHECK(empty.size() == 0 );
}
TEST_CASE_METHOD(parse_fixture, "query_compare_field_value", "[graph][query]") {
	auto cur =  mk_executor()("{v.field_1==1}");
	CHECK(cur.size() == 1 );
//	REQUIRE( mk_obj() == mk_obj(cur[0].base) );
//	CHECK( mk_obj(cur[0].base).keys().size() == 3 );
}
/*
*/
/*
//STEP:
//      заюзать в определители поля jiexpr
//        текущие поле - node
//        нужен решатель с контекстом, в который можно добавить поле node
//        regexpr taken from jiexpr
//      tests:
//        node by field name
//        node by field_name field_value pair
//        node by field_value only
//        node with path (filter)
//          any children
//          children by name
//        node expressions
TEST_CASE_METHOD(parse_fixture, "select_single_node", "[graph][query]") {
	src_st.leafs.emplace_back();
	auto e = mk_executor();
	auto r = e("{:field_1}");
	CHECK(r.size() == 1);
	auto r2 = e("{'test_data::test_fields':field_1}");
	//REQUIRE( r2.size() > 0 );
	//CHECK( mk_obj(r[0].base) == mk_obj(r2[0].base) );
	//auto r_regexp = e("{:'field_[0-9]'}");
	//REQUIRE( r_regexp.size() > 0 );
}
*/
