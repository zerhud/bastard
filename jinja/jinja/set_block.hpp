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

template<typename factory>
struct set_block : element_with_name<factory> {
  using base =  element_with_name<factory>;
  using name_t = typename base::name_t;
  using context_type = typename base::context_type;
  using expr_type = decltype(mk_jinja_expression(std::declval<factory>()));
  using p = typename factory::parser;
  template<auto s> using th = typename p::template tmpl<s>;

  constexpr set_block() : set_block(factory{}) {}
  constexpr set_block(factory f)
  : f(std::move(f))
#ifdef __clang__
  //TODO:GCC15: remove ifdef when gcc bug will be fixed (cannot compile for some reason the parser.cpp test)
  , _name(mk_str(this->f))
#endif
  , holder(this->f)
  {}

  constexpr const name_t& name() const override { return _name; }
  constexpr void execute(context_type& ctx) const override {
    auto data = ctx.mk_data();
    {
      auto area_holder = ctx.env.push_area();
      auto output_holder = ctx.catch_output();
      holder.execute(ctx);
      data = ctx.extract_output_to_data();
    }
    ctx.env.add_local(ctx.mk_data(name()), std::move(data));
    ctx.env.add_local(ctx.mk_data(name()), jinja_expression_eval(ctx.f, ctx.env.mk_context_data(), handler));
  }

  constexpr auto size() const { return holder.size(); }
  constexpr auto& operator[](auto ind) const { return holder[ind]; }

  constexpr static auto struct_fields_count() { return 6; }

  factory f;
  trim_info<factory> begin_left;
  name_t _name;
  expr_type handler;
  block_content<factory> holder;
  trim_info<factory> end_right;

  constexpr static auto mk_parser(const auto& f) {
    using bp = base_parser<factory>;
    constexpr auto trim_parser = trim_info<factory>::mk_parser();
    constexpr auto ident = bp::mk_ident_parser();
    //TODO: we cannot wrap the result in skip() here for some reason (error with glvalue)
    return skip(
    ++lexeme(bp::mk_block_begin() >> trim_parser)
    >> def(p::template lit<"set">)++
    >> (def(th<'('>::_char) >> ident >> th<')'>::_char | reparse(ident))++
    >> mk_jinja_expression_parser(f)
    >> ++th<1>::rec
    >> p::template lit<"endset"> >> ++trim_parser >> bp::mk_block_end()
    );
  }
};

} // namespace jinja_details
