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
struct comment_operator : base_jinja_element<factory> {
	using parser = factory::parser;
	using string_type = factory::string_t;

	trim_info<factory> begin;
	string_type value;
	trim_info<factory> end;
	factory f;
	constexpr static auto struct_fields_count() { return 4; }

	constexpr explicit comment_operator(factory f) : f(std::move(f)) {}

	constexpr void execute(base_jinja_element<factory>::context_type&) const override { }

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		constexpr auto end = lexeme(trim_info<factory>::mk_parser() >> bp::mk_comment_end());
		return
			   lexeme(bp::mk_comment_begin() >> trim_info<factory>::mk_parser())++
			>> *(parser::any - end) >> ++end;
	}
};

template<typename factory>
struct expression_operator : base_jinja_element<factory> {
	using parser = factory::parser;
	using expr_type = factory::jinja_expression;
	using context_type = base_jinja_element<factory>::context_type;

	trim_info<factory> begin;
	expr_type expr;
	trim_info<factory> end;
	factory f;
	constexpr static auto struct_fields_count() { return 4; }

	constexpr explicit expression_operator(factory f) : f(std::move(f)) {}

	constexpr void execute(context_type& ctx) const override {
		ctx(begin, ctx.mk_data(jinja_to_string(ctx.f, expr)), end);
	}

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		return
			   lexeme(bp::mk_expr_begin() >> trim_info<factory>::mk_parser())++
			>> use_seq_result(skip(++expr_type::mk_parser()
			>> ++lexeme(trim_info<factory>::mk_parser() >> bp::mk_expr_end())))
		;
	}
};

} // namespace jinja_details
