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

namespace jinja_details {

template<typename factory>
struct execution_context {};

template<typename factory>
struct base_jinja_element {
	virtual ~base_jinja_element() noexcept =default ;
	virtual void execute(execution_context<factory>& ctx) const =0 ;
};

template<typename factory>
struct content : base_jinja_element<factory> {
	using string_type = factory::string_t;
	using parser = factory::parser;

	string_type value;

	constexpr void execute(execution_context<factory>& ctx) const override { }

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		return lexeme( fnum<0>(parser::nop) >> +(parser::any - bp::mk_check_parser()) );
	}
};
template<typename factory>
struct comment_operator : base_jinja_element<factory> {
	using parser = factory::parser;
	using string_type = factory::string_t;

	constexpr static auto struct_fields_count() { return 3; }
	trim_info<factory> begin;
	string_type value;
	trim_info<factory> end;

	constexpr void execute(execution_context<factory>& ctx) const override { }

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		constexpr auto end = trim_info<factory>::mk_parser() >> bp::mk_comment_end();
		return
		   lexeme(bp::mk_comment_begin() >> trim_info<factory>::mk_parser())++
		>> *(parser::any - end)
		>> ++lexeme(end);
	}
};

template<typename factory>
struct expression_operator : base_jinja_element<factory> {
	using parser = factory::parser;
	using expr_type = factory::jinja_expression;

	constexpr static auto struct_fields_count() { return 3; }
	trim_info<factory> begin;
	expr_type expr;
	trim_info<factory> end;

	constexpr void execute(execution_context<factory>& ctx) const override { }

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		return
		   lexeme(bp::mk_expr_begin() >> trim_info<factory>::mk_parser())++
		>> expr_type::mk_parser()
		>> ++lexeme(trim_info<factory>::mk_parser() >> bp::mk_expr_end())
		;
	}
};

template<typename factory>
struct named_block : base_jinja_element<factory> {
	using base = base_jinja_element<factory>;
	using p = factory::parser;

	constexpr static auto mk_content_holder(const factory& f) {
		return mk_vec<const base*>(f);
	}
	//using content_type = decltype();

	constexpr void execute(execution_context<factory>& ctx) const override { }

	constexpr static auto struct_fields_count() { return 4; }
	trim_info<factory> begin_left, begin_right;
	trim_info<factory> end_left, end_right;
};

} // namespace jinja_details

template<typename factory>
struct jinja {
	factory f;

	constexpr jinja() : jinja(factory{}) {}
	constexpr explicit jinja(factory f) : f(std::move(f)) {}
};
