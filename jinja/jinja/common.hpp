#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>
#include "trim_info.hpp"
#include "context.hpp"

namespace jinja_details {

template<typename t> struct type_c{ using type = t; t operator+()const; };
template<typename factory>
constexpr auto mk_context_type() {
	if constexpr(requires{ typename factory::jinja_context;}) return type_c<typename factory::jinja_context>{};
	else return type_c<context<factory>>{};
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
struct base_jinja_element {
	using context_type = decltype(+mk_context_type<factory>());
	using data_type =  typename context_type::data_type;

	virtual ~base_jinja_element() noexcept =default ;
	virtual void execute(context_type& ctx) const =0 ;
};

template<typename factory>
struct element_with_name : base_jinja_element<factory> {
	using string_t = decltype(mk_str(std::declval<factory>()));
	virtual const string_t& name() const =0 ;
};

} // namespace jinja_details
