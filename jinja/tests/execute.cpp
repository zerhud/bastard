/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"
#include <absd/absd.hpp>
#include "ascip.hpp"

#include "jinja.hpp"

#include <charconv>

using namespace std::literals;

template<typename...> struct test_type_list {};

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
static_assert(symbols_count(10'000) == 5);
static_assert(symbols_count(-10'000) == 6);
static_assert(symbols_count(-1) == 2);
static_assert(test_to_string(10) == "10");
static_assert(test_to_string(-1) == "-1");

struct test_expr : std::variant<std::string, int, bool> {
	constexpr static auto mk_parser() {
		using p = ascip<std::tuple>;
		return p::quoted_string | p::int_ | (as<true>(p::template lit<"true">)|as<false>(p::template lit<"false">));
	}
};

struct factory : tests::factory {
	using extra_types = tests::test_type_list<jinja_details::environment<factory>>;
	using parser = ascip<std::tuple>;
	using jinja_expression = test_expr;
	using data_type = absd::data<factory>;
	template<typename... types> using data_type_tmpl = absd::data<types...>;
};

using parser = factory::parser;
using jinja_env = jinja_details::environment<factory>;
using data = jinja_env::data_type;

constexpr std::string jinja_to_string(const factory&, const test_expr& e) {
	return test_to_string(e.index()) + ": '"
	+ visit([](auto e) {
		if constexpr (std::is_same_v<decltype(e), std::string>) return e;
		else return test_to_string((int)e);
	}, e) + "'"
	;
}

constexpr data jinja_to_data(const factory& f, const auto& env, const auto& data) {
	if (data.index()==0) return env.at(factory::data_type{get<0>(data)});
	return visit([](auto& v){return factory::data_type{v};}, data);
}

static_assert( absd::details::as_object<jinja_env, data>, "for check the reason if we cannot" );
static_assert( jinja_env{}.mk_context_data().is_object(), "context is data object" );
static_assert( [] {
	jinja_env env;
	data d = env.mk_context_data();
	env.add_global(data{"foo"}, data{3});
	auto keys = d.keys();
	return (d[data{"foo"}]==data{3})
	+ 2*d[data{"bar"}].is_none()
	+ 4*d.contains(data{"foo"})
	+ 8*d.contains("foo")
	+ 16*(d.size()==1)
	+ 32*(keys.size()==1)
	;
}() == 63, "can store global variables" );
static_assert( [] {
	jinja_env env;
	data d = env.mk_context_data();
	env.add_global(data{"glob"}, data{3});
	data::integer_t in_block=0, in_block_glob=0, in_block_glob2=0;
	{
		auto holder = env.push_frame();
		env.add_local(data{"name"}, data{7});
		in_block = (data::integer_t)d[data{"name"}];
		in_block_glob = (data::integer_t)d[data{"glob"}];
		env.add_local(data{"glob"}, data{11});
		in_block_glob2 = (data::integer_t)d[data{"glob"}];
	}
	return (in_block==7)
	+ 2*d[data{"name"}].is_none()
	+ 4*(in_block_glob==3)
	+ 8*(in_block_glob2==11)
	;
}() == 15, "can create frame and no variable after frame removed" );
static_assert( [] {
	jinja_env env;
	data d = env.mk_context_data();
	data::integer_t in_block1=0, in_block2=0;
	{
		auto holder1 = env.push_area();
		env.add_local(data{"name"}, data{7});
		{
			auto holder2 = env.push_area();
			env.add_local(data{"name"}, data{11});
			in_block2 = (data::integer_t)d[data{"name"}];
		}
		in_block1 = (data::integer_t)d[data{"name"}];
	}
	return (in_block1==7) + 2*(in_block2==11);
}() == 3, "can create variables in area block" );
static_assert( [] {
	jinja_env env;
	data d = env.mk_context_data();
	auto holder_f1 = env.push_frame();
	env.add_local(data{"name"}, data{7});
	const auto v1 = (data::integer_t)d[data{"name"}];
	env.add_local(data{"name"}, data{11});
	const auto v2 = (data::integer_t)d[data{"name"}];
	auto holder_f2 = env.push_frame();
	env.add_local(data{"name"}, data{13});
	const auto v3 = (data::integer_t)d[data{"name"}];
	auto holder_a1 = env.push_area();
	env.add_local(data{"name"}, data{17});
	const auto v4 = (data::integer_t)d[data{"name"}];
	env.add_global(data{"glob"}, data{3});
	env.add_global(data{"glob"}, data{7});
	const auto v5 = (data::integer_t)d[data{"glob"}];
	return (v1==7)
	+ 2*(v2==11)
	+ 4*(v3==13)
	+ 8*(v4==17)
	+ 16*(v5==7)
	;
}() == 31, "can override variables" );

static_assert( []{
	using op_type = jinja_details::content<factory>;
	op_type r1{factory{}};
	parse(r1.mk_parser(), +parser::space, parser::make_source("12 << test_text"), r1);
	op_type::context_type ctx{factory{}};
	r1.execute(ctx);
	return ctx.out.size() +
		2*(ctx.out[0].value == "12 << test_text")
		;
}() == 3, "content appended to context");
static_assert( []{
	using op_type = jinja_details::expression_operator<factory>;
	op_type r1{factory{}};
	parse(r1.mk_parser(), +parser::space, parser::make_source("<= 3 =>"), r1);
	op_type::context_type ctx{factory{}};
	r1.execute(ctx);
	return (ctx.out.size() == 1) + 2*(ctx.out[0].value=="1: '3'");
}() == 3, "the expression operator appends to context the result of the expression" );
static_assert( [] {
	using cnt_type = jinja_details::content<factory>;
	using op_type = jinja_details::set_block<factory>;
	op_type r1{factory{}};
	r1._name = "test_name";
	r1.handler = test_expr{"test_name"};
	auto cnt = std::make_unique<cnt_type>(factory{});
	cnt->value = "test content";
	r1.holder.holder.emplace_back() = std::move(cnt);
	op_type::context_type ctx{factory{}};
	r1.execute(ctx);
	return ctx.env.size();
}() == 1, "set block sets local variable" );


int main(int,char**) {
	return 0;
}