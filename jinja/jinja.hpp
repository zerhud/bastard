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
        return const_cast<rt*>(static_cast<const rt*>(v.get()));
    };
}
template<typename factory>
constexpr auto mk_content_parser(factory f) {
    using p = factory::parser;
    using bp = base_parser<factory>;

    auto content_parser = content<factory>::mk_parser();
    auto comment_parser = comment_operator<factory>::mk_parser();
    auto expression_parser = expression_operator<factory>::mk_parser();
    auto block_parser = named_block<factory>::mk_parser();
    auto macro_parser = macro_block<factory>::mk_parser();
    auto set_block_parser = set_block<factory>::mk_parser();
    constexpr auto trim_parser = trim_info<factory>::mk_parser();
	//TODO: remove skip() for block_parser - it should to be in block parser
	//      but we cannot do it now for some compile issue with glvalue
    return lexeme(
       trim_parser++ >> bp::mk_block_end()
    >> *(
          expression_parser(mk_ptr_maker<expression_operator>(f))
        | comment_parser(mk_ptr_maker<comment_operator>(f))
        | content_parser(mk_ptr_maker<content>(f))
        | skip(block_parser(mk_ptr_maker<named_block>(f)))
        | skip(macro_parser(mk_ptr_maker<macro_block>(f)))
        | skip(macro_parser(mk_ptr_maker<macro_block>(f)))
        | skip(set_block_parser(mk_ptr_maker<set_block>(f)))
    )
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
		using ptr_type = decltype(mk_empty_ptr<const base>(f));
		return mk_vec<ptr_type>(f);
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

	constexpr static auto struct_fields_count() { return 4; }
	factory f;
	string_t _name;
	block_content<factory> holder;
	decltype(make_holder(std::declval<factory>())) blocks;

	constexpr static auto mk_parser(factory f) {
		using bp = base_parser<factory>;
		constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
		return
		   bp::mk_block_begin() >> p::template lit<"template"> >> ++ident
		>> ++mk_content_parser(f)
		>> p::template lit<"endtemplate"> >> bp::mk_block_end()
		;
	}
};

} // namespace jinja_details

template<typename factory>
struct jinja {
	factory f;

	constexpr jinja() : jinja(factory{}) {}
	constexpr explicit jinja(factory f) : f(std::move(f)) {}
};
