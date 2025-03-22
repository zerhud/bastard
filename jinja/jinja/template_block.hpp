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

template<typename factory> constexpr auto mk_content_parser(factory f) ;

struct template_holder;

template<typename factory>
struct template_block : element_with_name<factory> {
  using base = element_with_name<factory>;
  using string_t = base::string_t;
  using name_t = base::string_t;
  using context_type = base::context_type;
  using p = factory::parser;
  template<auto s> using th = p::template tmpl<s>;

  constexpr const string_t& name() const override { return _name; }
  constexpr void execute(context_type& ctx) const override { }

  constexpr auto size() const { return holder.size(); }
  constexpr auto& operator[](auto ind) const { return holder[ind]; }

  constexpr static auto make_holder(const factory& f) {
    return mk_vec<const base*>(f);
  }

  constexpr template_block() : template_block(factory{}) {}
  constexpr explicit template_block(factory f)
  : f(std::move(f))
#ifdef __clang__
  //TODO:GCC15: remove ifdef when gcc bug will be fixed (cannot compile for some reason the parser.cpp test)
  , _name(mk_str(this->f))
#endif
  , holder(this->f)
  , blocks(make_holder(this->f))
  {}

  constexpr static auto struct_fields_count() { return 5; }
  factory f;
  string_t _name;
  decltype(mk_vec<name_t>(std::declval<factory>())) extends;
  block_content<factory> holder;
  decltype(make_holder(std::declval<factory>())) blocks;

  friend constexpr void add_block(template_block& tmpl, auto* ptr) { tmpl.blocks.emplace_back(ptr); }
  constexpr static auto mk_parser(factory f) {
    using bp = base_parser<factory>;
    constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
    return create_in_ctx<template_holder>([](template_block&t){return &t;},
         bp::mk_block_begin()
      >> def(lit<"template">(p{}))
      >> -by_ind<1>(ident)
      >> -by_ind<2>(lit<"extends">(p{}) >> ident % ',')
      >> by_ind<3>(mk_content_parser(f))
      >> lit<"endtemplate">(p{}) >> bp::mk_block_end()
    );
  }
};

}