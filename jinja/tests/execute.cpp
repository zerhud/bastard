/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"
#include "ascip.hpp"

#include "jinja.hpp"

#include <iostream>
#include <charconv>

using namespace std::literals;

struct test_context ;

template<auto base = 10>
constexpr auto symbols_count(auto v) {
	unsigned int len = v >= 0 ? 1 : 2;
	for (auto n = v < 0 ? -v : v; n; ++len, n /= base);
	return ((len-1)*(v!=0)) + (len*(v==0));
}

template<auto base = 10>
constexpr std::string test_to_string(auto v) {
	std::string ret;
	ret.resize(symbols_count<base>(v));
	std::to_chars(ret.data(), ret.data() + ret.size(), v);
	return ret;
}

static_assert(symbols_count(0) == 1);
static_assert(symbols_count(10) == 2);
static_assert(symbols_count(-1) == 2);
static_assert(test_to_string(10) == "10");
static_assert(test_to_string(-1) == "-1");

struct test_expr : std::variant<int, bool> {
	constexpr static auto mk_parser() {
		using p = ascip<std::tuple>;
		return p::int_ | (as<true>(p::template lit<"true">)|as<false>(p::template lit<"false">));
	}
};

struct factory : tests::factory {
	using parser = ascip<std::tuple>;
	using jinja_expression = test_expr;
	using jinja_context = test_context;
};

using parser = factory::parser;

struct test_context {
	factory f;
	std::vector<std::string> holder;
	constexpr void append_content(auto&& val) {
		holder.emplace_back(std::forward<decltype(val)>(val));
	}
	constexpr void append_output(const auto& left_trim, auto&& val, const auto& right_trim) {
		holder.emplace_back(std::forward<decltype(val)>(val));
	}
};

constexpr std::string jinja_to_string(const factory&, const test_expr& e) {
	return test_to_string(e.index()) + ": '"
	+ visit([](int e){ return test_to_string(e); }, e) + "'"
	;
}

static_assert( []{
	using op_type = jinja_details::content<factory>;
	op_type r1;
	parse(r1.mk_parser(), +parser::space, parser::make_source("12 << test_text"), r1);
	op_type::context_type ctx;
	r1.execute(ctx);
	return
		  (ctx.holder.size()) +
		2*(ctx.holder[0] == "12 << test_text")
		;
}() == 3, "content appended to context");
static_assert( []{
	using op_type = jinja_details::expression_operator<factory>;
	op_type r1;
	parse(r1.mk_parser(), +parser::space, parser::make_source("<= 3 =>"), r1);
	op_type::context_type ctx;
	r1.execute(ctx);
	return (ctx.holder.size() == 1) + 2*(ctx.holder[0]=="0: '3'");
}() == 3, "the expression operator appends to context the result of the expression" );


int main(int,char**) {
	return 0;
}