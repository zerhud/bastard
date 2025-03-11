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

template<typename factory> struct call_block : base_jinja_element<factory> {
  using base = base_jinja_element<factory>;
  using context_type = base::context_type;
  using expr_type = decltype(mk_jinja_expression(std::declval<factory>()));
  using string_t = decltype(mk_str(std::declval<factory>()));
  using p = typename factory::parser;
  template<auto s> using th = typename p::template tmpl<s>;

  struct parameter {
    string_t name;
    expr_type value;
  };

  using parameters_holder = decltype(mk_vec<parameter>(std::declval<factory>()));

  constexpr call_block() : call_block(factory{}) {}
  constexpr explicit call_block(factory f)
    : f(std::move(f))
    , holder(this->f)
  {}

  factory f;
  trim_info<factory> left;
  parameters_holder params;
  expr_type expr;
  block_content<factory> holder;
  trim_info<factory> right;
  constexpr static auto struct_fields_count() { return 6; }

  constexpr void execute(context_type& ctx) const override {
  }

  constexpr static auto mk_parser(const auto& f) {
    using bp = base_parser<factory>;
    auto trim_parser = trim_info<factory>::mk_parser();
    constexpr auto ident = bp::mk_ident_parser();
		auto expr_parser = mk_jinja_expression_parser(f);
    return skip(
       ++lexeme(bp::mk_block_begin() >> trim_parser)
    >> def(p::template lit<"call">)++
		>> -(th<'(' >::_char >> -((ident++ >> -(th<'='>::_char >> expr_parser)) % ',') >> th<')'>::_char)
		>> ++expr_parser
    >> ++th<1>::rec
    >> p::template lit<"endcall"> >> ++trim_parser >> bp::mk_block_end()
    );
  }
};

}