#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

namespace jinja_details {

template<typename factory>
struct base_parser {
	using p = factory::parser;
	template<auto s> using t = p::template tmpl<s>;
	constexpr static auto mk_block_begin() { return p::template lit<"<%">; }
	constexpr static auto mk_block_end() { return p::template lit<"%>">; }
	constexpr static auto mk_comment_begin() { return p::template lit<"<#">; }
	constexpr static auto mk_comment_end() { return p::template lit<"#>">; }
	constexpr static auto mk_expr_begin() { return p::template lit<"<=">; }
	constexpr static auto mk_expr_end() { return p::template lit<"=>">; }
	constexpr static auto mk_check_parser() {
		return t<'<'>::char_ >> (t<'%'>::char_ | t<'#'>::char_ | t<'='>::char_);
	}
};
template<typename factory>
struct trim_info {
	using p = factory::parser;
	int shift{0};
	bool trim{false};
	constexpr static auto mk_parser() {
		return -p::int_++ >> -as<true>(p::template char_<'+'>);
	}
};

template<typename factory>
struct execution_context {};

template<typename factory>
struct base_jinja_element {
	virtual ~base_jinja_element() noexcept =default ;
	virtual void execute(execution_context<factory>& ctx) const =0 ;
};

} // namespace jinja_details
