#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

#include "jinja/common.hpp"
#include "jinja/content.hpp"
#include "jinja/operators.hpp"
#include "jinja/named_block.hpp"
#include "jinja/block_content.hpp"
#include "jinja/set_block.hpp"
#include "jinja/if_block.hpp"
#include "jinja/for_block.hpp"
#include "jinja/call_block.hpp"

namespace jinja_details {

struct template_holder;

//NOTE: GCC15: GCC_BUG: if named_block will be forward declarated
//      the gcc14 will report: use f after end of life.
//      also we cannot use &f for same reason (clang can)
//      so we need to create the mk_content_parser and mk_ptr_maker
//      outside the block_content class
template<template<typename>class type, typename factory>
constexpr auto mk_ptr_maker(const factory& f) {
  using rt = type<factory>;
  return [f](auto& v){
    v = mk_ptr<rt>(f, f);
    return const_cast<rt*>((const rt*)v.get());
  };
}
template<typename factory>
constexpr auto mk_content_parser(factory f) {
  using p = factory::parser;
  using bp = base_parser<factory>;

  auto content_parser = content<factory>::mk_parser();
  auto comment_parser = comment_operator<factory>::mk_parser();
  auto expression_parser = expression_operator<factory>::mk_parser(f);
  auto block_parser = named_block<factory>::mk_parser(f);
  auto macro_parser = macro_block<factory>::mk_parser(f);
  auto set_block_parser = set_block<factory>::mk_parser(f);
  constexpr auto trim_parser = trim_info<factory>::mk_parser();
  auto if_block_parser = if_block<factory>::mk_parser(f);
  auto for_block_parser = for_block<factory>::mk_parser(f);
  auto call_block_parser = call_block<factory>::mk_parser(f);
  //TODO:
  //     templates: inheritance, import (import as)
  //     parser facade (to parse file)
  return lexeme(
     trim_parser++ >> bp::mk_block_end() >> p::seq_enable_recursion
  >> (p::seq_enable_recursion >> *(
        expression_parser(mk_ptr_maker<expression_operator>(f))
      | comment_parser(mk_ptr_maker<comment_operator>(f))
      | content_parser(mk_ptr_maker<content>(f))
      | from_ctx<template_holder>([](auto& block, auto* tmpl){add_block(*tmpl, &block);}, block_parser)(mk_ptr_maker<named_block>(f))
      | from_ctx<template_holder>([](auto& block, auto* tmpl){add_block(*tmpl, &block);}, macro_parser)(mk_ptr_maker<macro_block>(f))
      | set_block_parser(mk_ptr_maker<set_block>(f))
      | if_block_parser(mk_ptr_maker<if_block>(f))
      | for_block_parser(mk_ptr_maker<for_block>(f))
      | call_block_parser(mk_ptr_maker<call_block>(f))
  ))
  >> ++trim_parser >> bp::mk_block_begin()
  );
}

template<typename factory>
struct template_block : element_with_name<factory> {
  using base = element_with_name<factory>;
  using string_t = base::string_t;
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
  block_content<factory> holder;
  decltype(make_holder(std::declval<factory>())) blocks;
  string_t file_name;

  friend constexpr void add_block(template_block& tmpl, auto* ptr) { tmpl.blocks.emplace_back(ptr); }
  constexpr static auto mk_parser(factory f) {
    using bp = base_parser<factory>;
    constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
    return create_in_ctx<template_holder>([](template_block&t){return &t;},
         bp::mk_block_begin()
      >> def(lit<"template">(p{}))
      >> by_ind<1>(ident)
      >> by_ind<2>(mk_content_parser(f))
      >> lit<"endtemplate">(p{}) >> bp::mk_block_end()
    );
  }
};

} // namespace jinja_details

template<typename factory>
struct jinja {
  using context = jinja_details::context<factory>;
  using environment = jinja_details::environment<factory>;

  factory f;
  constexpr jinja() : jinja(factory{}) {}
  constexpr explicit jinja(factory f) : f(std::move(f)) {}

  constexpr auto parse_file(auto src, auto name) const {
    auto parser = jinja_details::template_block<factory>::mk_parser(f);
    jinja_details::template_block<factory> result{f};
    result.file_name = std::move(name);
    parse(parser, +factory::parser::space, src, result);
    return result;
  }
};
