#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "common.hpp"
#include "content.hpp"

namespace jinja_details {

template<typename factory>
struct block_content : base_jinja_element<factory> {
	using p = factory::parser;
	template<auto s> using th = p::template tmpl<s>;
	using base = base_jinja_element<factory>;
	using context_type = base_jinja_element<factory>::context_type;

	constexpr static auto mk_content_holder(const factory& f) {
		using ptr_t = decltype(mk_empty_ptr<const base>(f));
		return mk_vec<ptr_t>(f);
	}

	using holder_type = decltype(mk_content_holder(std::declval<factory>()));

	constexpr void execute(context_type& ctx) const override { }

	constexpr static auto struct_fields_count() { return 4; }

	trim_info<factory> left;
	holder_type holder;
	trim_info<factory> right;
	factory f;

	constexpr auto size() const { return holder.size(); }
	constexpr auto& operator[](auto ind) const { return holder[ind]; }

	constexpr explicit block_content(factory f)
			: holder(mk_content_holder(f))
			, f(std::move(f))
	{}

	template<template<typename>class type>
	constexpr static auto mk_ptr_maker(const factory& f) {
		using rt = type<factory>;
		return [&f](auto& v){
			v = [&f]{
				if constexpr(requires{rt{f};}) return mk_ptr<rt>(f, f);
				else return mk_ptr<rt>(f);
			}();
			return const_cast<rt*>(static_cast<const rt*>(v.get()));
		};
	}
	constexpr static auto mk_parser(factory f) {
		using bp = base_parser<factory>;
		auto content_parser = content<factory>::mk_parser();
		auto comment_parser = comment_operator<factory>::mk_parser();
		auto expression_parser = expression_operator<factory>::mk_parser();
		constexpr auto trim_parser = trim_info<factory>::mk_parser();
		return lexeme(
		   trim_parser++ >> bp::mk_block_end()
		>> *(
		       expression_parser(mk_ptr_maker<expression_operator>(f))
		    | comment_parser(mk_ptr_maker<comment_operator>(f))
		    | content_parser(mk_ptr_maker<content>(f))
		)
		>> ++trim_parser >> bp::mk_block_begin()
		);
	}
};

} // namespace jinja_details
