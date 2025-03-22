/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "common.hpp"
#include "block_content.hpp"

namespace jinja_details {

template<typename factory> struct for_block : base_jinja_element<factory> {
  using base = base_jinja_element<factory>;
  using context_type = base::context_type;
  using expr_type = decltype(mk_jinja_expression(std::declval<factory>()));
  using p = typename factory::parser;
  template<auto s> using th = typename p::template tmpl<s>;

  constexpr for_block() : for_block(factory{}) {}
  constexpr explicit for_block(factory f)
    : f(std::move(f))
    , for_exprs(this->f)
    , holder(this->f)
    , else_holder(this->f)
  {}

  constexpr static auto mk_variables(const factory& f) {
    using string_type = decltype(mk_str(f));
    return mk_vec<string_type>(f);
  }

  struct for_expr {
    constexpr explicit for_expr(const factory& f)
#ifdef __clang__
    : variables(mk_variables(f))
    , expr(mk_jinja_expression(f))
#endif
    {}
    constexpr static auto struct_fields_count() { return 2; }
    decltype(mk_variables(std::declval<factory>())) variables;
    expr_type expr;
  };

  constexpr static auto mk_for_exprs(const factory& f) { return mk_vec<for_expr>(f); }

  struct for_expr_holder {
    decltype(mk_for_exprs(std::declval<factory>())) exprs;
    factory f;
    constexpr static auto struct_fields_count() { return 2; }
    explicit constexpr for_expr_holder(factory f)
    :
#ifdef __clang__
    exprs(mk_for_exprs(f)),
#endif
    f(std::move(f))
    {}
    friend constexpr auto& emplace_back(for_expr_holder& obj) { return obj.exprs.emplace_back(obj.f); }
    constexpr auto size() const { return exprs.size(); }
    constexpr const auto& operator[](auto&& ind) const { return exprs[std::forward<decltype(ind)>(ind)]; }
  };

  factory f;
  trim_info<factory> left;
  for_expr_holder for_exprs;
  block_content<factory> holder;
  block_content<factory> else_holder;
  trim_info<factory> right;
  constexpr static auto struct_fields_count() { return 6; }

  constexpr void execute(context_type& ctx) const override {
    execute_expr(ctx, 0);
  }
  constexpr void execute_expr(context_type& ctx, unsigned expr_ind) const {
    if (expr_ind == for_exprs.size()) holder.execute(ctx);
    else iterate_expr(ctx, for_exprs.exprs[expr_ind], expr_ind+1);
  }
  constexpr void iterate_expr(context_type& ctx, const for_expr& expr, unsigned expr_ind) const {
    auto result = jinja_expression_eval(ctx.f, ctx.env.mk_context_data(), expr.expr);
    using string_t = typename decltype(result)::string_t;
    if (result.is_string())
      exec(result, [&](const auto&, const string_t& val) { for (auto& s:val) prepare_ctx(ctx, val, expr_ind); });
    else if (result.is_array())
      for (auto i=0;i<result.size();++i) prepare_ctx(ctx, result[i], expr_ind);
    else if (result.is_object()) {
      auto keys = result.keys();
      for (auto i=0;i<keys.size();++i) prepare_ctx(ctx, result[keys[i]], expr_ind);
    }
  }
  constexpr void prepare_ctx(context_type& ctx, auto result, unsigned expr_ind) const {
    execute_expr(ctx, expr_ind);
  }

  constexpr static auto mk_parser(const auto& f) {
    using bp = base_parser<factory>;
    auto trim_parser = trim_info<factory>::mk_parser();
    constexpr auto ident = bp::mk_ident_parser();
    return skip(
       ++lexeme(bp::mk_block_begin() >> trim_parser)
    >> def(p::template lit<"for">)++
    >> (ident % ',' >> p::template lit<"in">++ >> mk_jinja_expression_parser(f)) % ';'
    >> ++th<1>::rec++
    >> -(p::template lit<"else"> >> th<1>::rec)
    >> p::template lit<"endfor"> >> ++trim_parser >> bp::mk_block_end()
    );
  }
};

}
