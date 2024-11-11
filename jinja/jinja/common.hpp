#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

namespace jinja_details {

template<typename factory>
struct execution_context {
	using content_holder = decltype(mk_vec<decltype(mk_str(std::declval<factory>()))>(std::declval<factory>()));

	content_holder holder;
	constexpr void append_output(const auto& value) {
		holder.emplace_back(std::forward<decltype(value)>(value));
	}
};

template<typename t> struct type_c{ using type = t; };
template<typename factory>
constexpr auto mk_context_type() {
	if constexpr(requires{ typename factory::jinja_context;}) return type_c<typename factory::jinja_context>{};
	else return type_c<execution_context<factory>>{};
}

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
struct base_jinja_element {
	using context_type = decltype(mk_context_type<factory>())::type;

	virtual ~base_jinja_element() noexcept =default ;
	virtual void execute(context_type& ctx) const =0 ;
};

} // namespace jinja_details