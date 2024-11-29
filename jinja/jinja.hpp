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
#include "jinja/operators.hpp"

namespace jinja_details {

template<typename factory>
struct content : base_jinja_element<factory> {
	using string_type = factory::string_t;
	using parser = factory::parser;
	using context_type = base_jinja_element<factory>::context_type;

	string_type value;

	constexpr void execute(context_type& ctx) const override {
		ctx.append_content(value);
	}

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		return lexeme( fnum<0>(parser::nop) >> +(parser::any - bp::mk_check_parser()) );
	}
};

template<typename factory>
struct named_block : base_jinja_element<factory> {
	using base = base_jinja_element<factory>;
	using string_t = decltype(mk_str(std::declval<factory>()));
	using context_type = base_jinja_element<factory>::context_type;
	using p = factory::parser;
	template<auto s> using th = p::template tmpl<s>;

	constexpr static auto mk_content_holder(const factory& f) {
		using ptr_t = decltype(mk_ptr<const base>(f));
		return mk_vec<ptr_t>(f);
	}

	using holder_type = decltype(mk_content_holder(std::declval<factory>()));

	constexpr void execute(context_type& ctx) const override { }

	constexpr static auto struct_fields_count() { return 7; }
	factory f;
	trim_info<factory> begin_left, end_left;
	string_t name;
	holder_type holder;
	trim_info<factory> begin_right, end_right;

	constexpr named_block() : named_block(factory{}) {}
	constexpr explicit named_block(factory f)
	: f(std::move(f))
#ifdef __clang__
	//TODO:GCC15: remove ifdef when gcc bug will be fixed (cannot compile for some reason the parser.cpp test)
	, name(mk_str(this->f))
#endif
	, holder(mk_content_holder(this->f))
	{}

	template<template<typename>class type>
	constexpr static auto mk_ptr_maker(const factory& f) {
		auto make_type = [f]{
			if constexpr(requires{new type<factory>{f};}) return new type<factory>{f};
			else return new type<factory>{};
		};
		return [make_type](auto& v){
			auto* ptr = make_type();
			v.reset(ptr);
			return ptr;
		};
	}
	constexpr static auto mk_parser() {
		return mk_parser(factory{});
	}
	constexpr static auto mk_parser(factory f) {
		using bp = base_parser<factory>;
		auto content_parser = content<factory>::mk_parser();
		auto comment_parser = comment_operator<factory>::mk_parser();
		auto expression_parser = expression_operator<factory>::mk_parser();
		constexpr auto trim_parser = trim_info<factory>::mk_parser();
		constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
		return
		   ++lexeme(bp::mk_block_begin() >> trim_parser)++
		>> p::template lit<"block">++ >> ident >> --trim_parser >> use_seq_result(lexeme(fnum<3>(bp::mk_block_end())
		>> *fnum<4>(
		        expression_parser(mk_ptr_maker<expression_operator>(f))
		      | comment_parser(mk_ptr_maker<comment_operator>(f))
		      | content_parser(mk_ptr_maker<content>(f))
			)
		>> bp::mk_block_begin() >> fnum<5>(trim_parser)))
		>> p::template lit<"endblock"> >> fnum<6>(trim_parser) >> bp::mk_block_end()
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
