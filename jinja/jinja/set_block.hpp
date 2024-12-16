/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "common.hpp"
#include "block_content.hpp"

namespace jinja_details {

template<typename factory>
struct set_block : element_with_name<factory> {
	using base =  element_with_name<factory>;
	using string_t = typename base::string_t;
	using context_type = typename base::context_type;
	using expr_type = typename factory::jinja_expression;
	using p = typename factory::parser;
	template<auto s> using th = p::template tmpl<s>;

	constexpr set_block() : set_block(factory{}) {}
	constexpr set_block(factory f)
	: f(std::move(f))
#ifdef __clang__
	//TODO:GCC15: remove ifdef when gcc bug will be fixed (cannot compile for some reason the parser.cpp test)
	, _name(mk_str(this->f))
#endif
	, holder(this->f)
	{}

	constexpr const string_t& name() const override { return _name; }
	constexpr void execute(context_type& ctx) const override { }

	constexpr auto size() const { return holder.size(); }
	constexpr auto& operator[](auto ind) const { return holder[ind]; }

	constexpr static auto struct_fields_count() { return 6; }

	factory f;
	trim_info<factory> begin_left;
	string_t _name;
	expr_type handler;
	block_content<factory> holder;
	trim_info<factory> end_right;

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		auto expr_parser = expr_type::mk_parser();
		constexpr auto trim_parser = trim_info<factory>::mk_parser();
		constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
		//TODO: we cannot wrap the result in skip() here for some reason (error with glvalue)
		return
		++lexeme(bp::mk_block_begin() >> trim_parser)
		>> p::template lit<"set">++ >> reparse(ident)++
		>> expr_type::mk_parser()
		>> ++p::template req<1>
		>> p::template lit<"endset"> >> ++trim_parser >> bp::mk_block_end()
		;
	}
};

} // namespace jinja_details