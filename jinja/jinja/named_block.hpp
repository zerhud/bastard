#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "common.hpp"
#include "block_content.hpp"

namespace jinja_details {

template<typename factory, typename crtp>
struct block_with_params : element_with_name<factory> {
  using base =  element_with_name<factory>;
  using name_t = typename base::name_t;
  using p = typename factory::parser;
  template<auto s> using th = p::template tmpl<s>;
  using expr_type = decltype(mk_jinja_expression(std::declval<factory>()));
  using context_type = typename base::context_type;

  struct parameter {
    name_t name;
    expr_type value;
  };

  using parameters_holder = decltype(mk_vec<parameter>(std::declval<factory>()));

  constexpr const name_t& name() const override { return _name; }

  constexpr static auto struct_fields_count() { return 7; }
  factory f;
  trim_info<factory> begin_left;
  shift_info shift_inside;
  name_t _name;
  parameters_holder parameters;
  block_content<factory> holder;
  trim_info<factory> end_right;

  constexpr block_with_params() : block_with_params(factory{}) {}
  constexpr explicit block_with_params(factory f)
  : f(std::move(f))
#ifdef __clang__
  //TODO:GCC15: remove ifdef when gcc bug will be fixed (cannot compile for some reason the parser.cpp test)
  , _name(mk_str(this->f))
#endif
  , parameters(mk_vec<parameter>(this->f))
  , holder(this->f)
  {}

  constexpr auto size() const { return holder.size(); }
  constexpr auto& operator[](auto ind) const { return holder[ind]; }

  constexpr void execute(context_type& ctx) const override {
    for (auto& p:parameters)
      ctx.env.add_local(ctx.mk_data(p.name), jinja_expression_eval(ctx.f, ctx.env.mk_context_data(), p.value));
    this->holder.execute(ctx);
  }

  constexpr static auto mk_parser(const auto& f) {
    using bp = base_parser<factory>;
    auto expr_parser = mk_jinja_expression_parser(f);
    constexpr auto trim_parser = trim_info<factory>::mk_parser();
    constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
    return skip(
    ++lexeme(bp::mk_block_begin() >> trim_parser)++
    >> -shift_info::mk_parser<p>()
    >> def(crtp::keyword_open())++
    >> ident++
    >> -(th<'(' >::_char >> -((ident++ >> -(th<'='>::_char >> expr_parser)) % ',') >> th<')'>::_char)
    >> ++th<1>::rec
    >> crtp::keyword_close() >> ++trim_parser >> bp::mk_block_end()
    );
  }
};

template<typename factory>
struct named_block : block_with_params<factory, named_block<factory>> {
  using base = block_with_params<factory, named_block>;
  using context_type = typename base::context_type;
  using p = typename base::p;
  constexpr explicit named_block(factory f) : base(std::move(f)) {}
  consteval static auto keyword_open() { return lit<"block">(p{}); }
  consteval static auto keyword_close() { return lit<"endblock">(p{}); }
  constexpr void execute(context_type& ctx) const override {
    auto area_holder = ctx.env.push_frame();
    base::execute(ctx);
  }
};

template<typename factory>
struct macro_block : block_with_params<factory, macro_block<factory>> {
  using base = block_with_params<factory, macro_block>;
  using context_type = typename base::context_type;
  using p = typename base::p;
  constexpr explicit macro_block(factory f) : base(std::move(f)) {}
  consteval static auto keyword_open() { return lit<"macro">(p{}) ; }
  consteval static auto keyword_close() { return lit<"endmacro">(p{}); }
};

} // namespace jinja_details
