#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "common.hpp"

namespace jinja_details {

template<typename factory>
struct content : base_jinja_element<factory> {
  using parser = typename factory::parser;
  using string_type = decltype(mk_str(std::declval<factory>()));
  using data_type = typename base_jinja_element<factory>::data_type;
  using context_type = typename base_jinja_element<factory>::context_type;

  factory f;
  string_type value;
  constexpr static auto struct_fields_count() { return 2; }

  constexpr explicit content(factory f)
  : f(std::move(f))
#ifdef __clang__ //TODO: GCC15: gcc bug: cannot initialize a field with a method from ADL
  , value(mk_str(this->f))
#endif
  {}

  constexpr void execute(context_type& ctx) const override {
    ctx(ctx.mk_data(value));
  }

  constexpr static auto mk_parser() {
    using bp = base_parser<factory>;
    return lexeme( fnum<1>(parser::nop) >> +(parser::any - bp::mk_check_parser()) );
  }
};

} // namespace jinja_details
