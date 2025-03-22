#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>
#include "trim_info.hpp"
#include "context.hpp"

namespace jinja_details {

template<typename factory, typename type> struct vector_with_factory {
  using vec_type = decltype(mk_vec<type>(std::declval<factory>()));

  factory f;
  vec_type vec;

  constexpr explicit vector_with_factory(factory f) : f(std::move(f)), vec(mk_vec<type>(this->f)) {}

  constexpr auto size() const { return vec.size(); }
  constexpr auto& operator[](auto ind) { return vec[ind]; }
  constexpr const auto& operator[](auto ind) const { return vec[ind]; }
  friend constexpr auto& emplace_back(vector_with_factory& obj) {
    return obj.vec.emplace_back(obj.f);
  }
  friend constexpr void pop_back(vector_with_factory& obj) {
    obj.vec.pop_back();
  }
};

template<typename t> struct type_c{ using type = t; t operator+()const; };
template<typename factory>
constexpr auto mk_context_type() {
  if constexpr(requires{ typename factory::jinja_context;}) return type_c<typename factory::jinja_context>{};
  else return type_c<context<factory>>{};
}

template<typename factory>
struct base_parser {
  using p = typename factory::parser;
  template<auto s> using t = typename p::template tmpl<s>;
  constexpr static auto mk_block_begin() { return p::template lit<"<%">; }
  constexpr static auto mk_block_end() { return p::template lit<"%>">; }
  constexpr static auto mk_comment_begin() { return p::template lit<"<#">; }
  constexpr static auto mk_comment_end() { return p::template lit<"#>">; }
  constexpr static auto mk_expr_begin() { return p::template lit<"<=">; }
  constexpr static auto mk_expr_end() { return p::template lit<"=>">; }
  constexpr static auto mk_check_parser() {
    return t<'<'>::char_ >> (t<'%'>::char_ | t<'#'>::char_ | t<'='>::char_);
  }
  constexpr static auto mk_ident_parser() {
    return lexeme(p::alpha >> *(p::alpha | p::d10 | p::template char_<'_'>));
  }
};

template<typename factory>
struct base_jinja_element {
  using context_type = decltype(+mk_context_type<factory>());
  using data_type =  typename context_type::data_type;

  virtual ~base_jinja_element() noexcept =default ;
  virtual void execute(context_type& ctx) const =0 ;
};

template<typename factory>
struct element_with_name : base_jinja_element<factory> {
  using name_t = decltype(mk_str(std::declval<factory>()));
  using string_t = decltype(mk_str(std::declval<factory>()));
  virtual const name_t& name() const =0 ;
};

} // namespace jinja_details
