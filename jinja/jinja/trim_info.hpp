#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

namespace jinja_details {

struct shift_info {
  short shift{0};
  bool absolute{true};

  template <typename p, template<auto>class th=p::template tmpl>
  constexpr static auto mk_parser() {
    constexpr auto asb_field_resetter = as<true>(p::nop);
    constexpr auto abs_field_parser = as<false>(th<'+'>::char_ | th<'-'>::char_);
    return lexeme(++reparse(abs_field_parser|asb_field_resetter) >> --p::int_);
  }
};

template <typename factory>
struct trim_info {
  using p = factory::parser;
  bool trim{false};

  constexpr static auto mk_parser() {
    return p::nop++ >> ---as<true>(p::template char_<'+'>);
  }
};

} // namespace jinja_details
