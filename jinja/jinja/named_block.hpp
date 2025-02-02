#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "common.hpp"
#include "block_content.hpp"

namespace jinja_details {

template<typename factory, typename crtp>
struct block_with_params : element_with_name<factory> {
	using base =  element_with_name<factory>;
	using string_t = typename base::string_t;
	using context_type = typename base::context_type;
	using p = typename factory::parser;
	template<auto s> using th = p::template tmpl<s>;
	using expr_type = decltype(mk_jinja_expression(std::declval<factory>()));

	struct parameter {
		string_t name;
		expr_type value;
	};

	using parameters_holder = decltype(mk_vec<parameter>(std::declval<factory>()));

	constexpr const string_t& name() const override { return _name; }
	constexpr void execute(context_type& ctx) const override { }

	constexpr static auto struct_fields_count() { return 7; }
	factory f;
	trim_info<factory> begin_left;
	shift_info shift_inside;
	string_t _name;
	parameters_holder parameters;
	block_content<factory> holder;
	trim_info<factory> end_right;

	constexpr block_with_params() : block_with_params(factory{}) {}
	constexpr explicit block_with_params(factory f)
	: f(std::move(f))
#ifdef __clang__
	//TODO:GCC15: remove ifdef when gcc bug will be fixed (cannot compile for some reason the parser.cpp test)
	, _name(mk_str(this->f))
#endif
	, parameters(mk_vec<parameter>(this->f))
	, holder(this->f)
	{}

	constexpr auto size() const { return holder.size(); }
	constexpr auto& operator[](auto ind) const { return holder[ind]; }

	constexpr static auto mk_parser(const auto& f) {
		using bp = base_parser<factory>;
		auto expr_parser = mk_jinja_expression_parser(f);
		constexpr auto trim_parser = trim_info<factory>::mk_parser();
		constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
		//TODO: we cannot wrap the result in skip() here for some reason (error with glvalue)
		return
		++lexeme(bp::mk_block_begin() >> trim_parser)++
		>> -shift_info::mk_parser<p>()
		>> crtp::keyword_open()++
		>> ident++
		>> -(th<'(' >::_char >> -((ident++ >> -(th<'='>::_char >> expr_parser)) % ',') >> th<')'>::_char)
		>> ++th<0>::req
		>> crtp::keyword_close() >> ++trim_parser >> bp::mk_block_end()
		;
	}
};

template<typename factory>
struct named_block : block_with_params<factory, named_block<factory>> {
	using base = block_with_params<factory, named_block>;
	using p = typename base::p;
	constexpr explicit named_block(factory f) : base(std::move(f)) {}
	consteval static auto keyword_open() { return base::p::template lit<"block">; }
	consteval static auto keyword_close() { return base::p::template lit<"endblock">; }
};

template<typename factory>
struct macro_block : block_with_params<factory, macro_block<factory>> {
	using base = block_with_params<factory, macro_block>;
	constexpr explicit macro_block(factory f) : base(std::move(f)) {}
	consteval static auto keyword_open() { return base::p::template lit<"macro">; }
	consteval static auto keyword_close() { return base::p::template lit<"endmacro">; }
};

} // namespace jinja_details
