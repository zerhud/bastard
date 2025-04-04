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
#include "jinja/named_env_object.hpp"
#include "jinja/set_block.hpp"
#include "jinja/if_block.hpp"
#include "jinja/for_block.hpp"
#include "jinja/call_block.hpp"
#include "jinja/template_block.hpp"

namespace jinja_details {

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
constexpr auto mk_content_inner_parser(factory f) {
  using p = factory::parser;

  auto content_parser = content<factory>::mk_parser();
  auto comment_parser = comment_operator<factory>::mk_parser();
  auto expression_parser = expression_operator<factory>::mk_parser(f);
  auto block_parser = named_block<factory>::mk_parser(f);
  auto macro_parser = macro_block<factory>::mk_parser(f);
  auto set_block_parser = set_block<factory>::mk_parser(f);
  auto if_block_parser = if_block<factory>::mk_parser(f);
  auto for_block_parser = for_block<factory>::mk_parser(f);
  auto call_block_parser = call_block<factory>::mk_parser(f);

  return p::seq_enable_recursion >> *(
        expression_parser(mk_ptr_maker<expression_operator>(f))
      | comment_parser(mk_ptr_maker<comment_operator>(f))
      | content_parser(mk_ptr_maker<content>(f))
      | from_ctx<template_holder>([](auto& block, auto* tmpl){add_block(*tmpl, &block);}, block_parser)(mk_ptr_maker<named_block>(f))
      | from_ctx<template_holder>([](auto& block, auto* tmpl){add_block(*tmpl, &block);}, macro_parser)(mk_ptr_maker<macro_block>(f))
      | set_block_parser(mk_ptr_maker<set_block>(f))
      | if_block_parser(mk_ptr_maker<if_block>(f))
      | for_block_parser(mk_ptr_maker<for_block>(f))
      | call_block_parser(mk_ptr_maker<call_block>(f))
  );
}
template<typename factory>
constexpr auto mk_content_parser(factory f) {
  using p = factory::parser;
  using bp = base_parser<factory>;

  constexpr auto trim_parser = trim_info<factory>::mk_parser();
  //TODO:
  //     parser facade (to parse file)
  return lexeme(
     trim_parser++ >> bp::mk_block_end() >> p::seq_enable_recursion
  >> mk_content_inner_parser(f)
  >> ++trim_parser >> bp::mk_block_begin()
  );
}

template<typename factory>
struct file {
  using p = factory::parser;

  factory f;
  vector_with_factory<factory, import_operator<factory>> imports;
  vector_with_factory<factory, template_block<factory>> templates;
  decltype(mk_str(std::declval<factory>())) name;
  constexpr static auto struct_fields_count() { return 4; }

  constexpr file() : file(factory{}) {}
  constexpr explicit file(factory f) : f(std::move(f)), imports(this->f), templates(this->f)
#ifdef __clang__
  , name(mk_str(this->f)) //TODO: GCC15: cannot init the name field in gcc14 - compilation fail
#endif
  {}

  constexpr static auto mk_parser(const auto& f) {
    return by_ind<1>(+import_operator<factory>::mk_parser(f)) >> by_ind<2>(+template_block<factory>::mk_parser(f));
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
    jinja_details::file<factory> result{f};
    result.name = std::move(name);
    parse(result.mk_parser(f), +factory::parser::space, src, result);
    return result;
  }
};
