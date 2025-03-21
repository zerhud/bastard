#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "ai_helpers.hpp"
#include "tests/factory.hpp"
#include <ascip.hpp>
#include <absd.hpp>
#include <absd/formatter.hpp>
/*
#ifdef __clang__
#include "ascip_all_clang.hpp"
#else
#include "ascip_all_gcc.hpp"
#endif
*/

#include "jinja.hpp"

#include <charconv>
#include <iterator>

using namespace std::literals;

namespace helpers =  ai_helpers;

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
  using p = ascip;
  template<auto s> using th = p::template tmpl<s>;
  constexpr static auto mk_parser(const auto&, int, int) {
    constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
    return ident | p::int_ | (as<true>(p::template lit<"true">)|as<false>(p::template lit<"false">));
  }
};

struct factory : tests::factory {
  using extra_types = tests::test_type_list<jinja_details::environment<factory>>;
  using parser = ascip;
  using data_type = absd::data<factory>;
  template<typename... types> using data_type_tmpl = absd::data<types...>;

  void* test_data = nullptr;
  data_type(*eval_data_fnc)(const factory& f, const jinja_details::environment<factory>& env, const test_expr& data) = nullptr;

  helpers::constexpr_function<void(data_type, const test_expr&)> on_eval;
};

using parser = factory::parser;
using jinja_env = jinja_details::environment<factory>;
using jinja_ctx = jinja_details::context<factory>;
using data = jinja_env::data_type;

constexpr auto mk_jinja_expression(const factory& f) { return test_expr{}; }
constexpr auto mk_jinja_expression(const factory& f, bool cond) { return test_expr{cond}; }
constexpr auto mk_jinja_expression_parser(const factory& f) {
  return test_expr::mk_parser(f, 1, 1);
}
constexpr factory::data_type jinja_expression_eval(const factory& f, data env, const test_expr& e) {
  if (holds_alternative<bool>(e)) return factory::data_type{get<bool>(e)};
  if (f.on_eval) f.on_eval(std::move(env), e);
  return factory::data_type{test_to_string(e.index()) + ": '"
  + visit([](auto e) {
    if constexpr (std::is_same_v<decltype(e), std::string>) return e;
    else return test_to_string((int)e);
  }, e) + "'"}
  ;
}

constexpr std::string jinja_to_string(const factory& f, const data& obj) {
  auto ret = mk_str(f);
  back_insert_format(back_inserter(ret), obj);;
  return ret;
}

constexpr data jinja_to_data(const factory& f, const auto& env, const auto& data) {
  if (f.eval_data_fnc) return f.eval_data_fnc(f, env, data);
  if (data.index()==0) return env.at(factory::data_type{get<0>(data)});
  return visit([](auto& v){return factory::data_type{v};}, data);
}

template<typename out_info>
constexpr data jinja_to_data(const factory& f, const std::vector<out_info>& d) {
  return data{};
}
