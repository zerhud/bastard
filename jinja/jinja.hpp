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
struct content {
	using string_type = factory::string_t;
	using parser = factory::parser;

	string_type value;

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		return fnum<0>(parser::nop) >> +(parser::any - (bp::mk_block_begin() | bp::mk_expr_begin() | bp::mk_comment_begin()));
	}
};
template<typename factory>
struct comment_operator {
	using parser = factory::parser;
	using string_type = factory::string_t;

	trim_info<factory> begin;
	string_type value;
	trim_info<factory> end;

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		constexpr auto end = trim_info<factory>::mk_parser() >> bp::mk_comment_end();
		return
		   bp::mk_comment_begin() >> trim_info<factory>::mk_parser()++
		>> *(parser::any - end)
		>> ++end;
	}
};

template<typename factory>
struct expression_operator {
	using parser = factory::parser;
	using expr_type = factory::jinja_expression;

	trim_info<factory> begin;
	expr_type expr;
	trim_info<factory> end;

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		return
		   bp::mk_expr_begin() >> trim_info<factory>::mk_parser()++
		>> expr_type::mk_parser()
		>> ++trim_info<factory>::mk_parser() >> bp::mk_expr_end()
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
