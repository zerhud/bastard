/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <tests/factory.hpp>
#include <tests/factory.hpp>

#include "common.hpp"
#include "block_content.hpp"

namespace jinja_details {

template<typename factory> struct if_else_block : base_jinja_element<factory> {
	using base = base_jinja_element<factory>;
	using context_type = base::context_type;
	using expr_type = decltype(mk_jinja_expression(std::declval<factory>()));
	using p = typename factory::parser;
	template<auto s> using th = typename p::template tmpl<s>;

	constexpr if_else_block() : if_else_block(factory{}) {}
	constexpr explicit if_else_block(factory f) : f(std::move(f))/*, condition(mk_jinja_expression(this->f), true)*/, holder(this->f) {}

	factory f;
	trim_info<factory> left;
	//expr_type condition;
	block_content<factory> holder;
	constexpr static auto struct_fields_count() { return 3; }

	constexpr void execute(context_type& ctx) const override {
		//auto result = jinja_expression_eval(f, condition);
		//if (!!result) holder.execute(ctx);
	}

	constexpr static auto mk_parser(const auto& f) {
		using bp = base_parser<factory>;
		constexpr auto trim_parser = trim_info<factory>::mk_parser();
		return skip(
		   ++lexeme(bp::mk_block_begin() >> trim_parser)
		>> def(p::template lit<"else">)
		>> ++lexeme(trim_parser([](auto& v){return &v.left;}) >> bp::mk_block_end())
		>> th<0>::rec([](auto& v){return &v.holder;})
		);
	}
};

template<typename factory> struct if_block : base_jinja_element<factory> {
	using base = base_jinja_element<factory>;
	using context_type = base::context_type;
	using expr_type = decltype(mk_jinja_expression(std::declval<factory>()));
	using p = typename factory::parser;
	template<auto s> using th = typename p::template tmpl<s>;

	constexpr if_block() : if_block(factory{}) {}
	constexpr explicit if_block(factory f) : f(std::move(f))
#ifdef __clang__ //TODO: GCC15: remove the condition then gcc15
	, condition(mk_jinja_expression(this->f))
#endif
	, holder(this->f)
	, else_blocks(mk_vec<if_else_block<factory>>(this->f))
	{}

	factory f;
	trim_info<factory> begin_left;
	expr_type condition;
	block_content<factory> holder;
	decltype(mk_vec<if_else_block<factory>>(std::declval<factory>())) else_blocks;
	trim_info<factory> end_right;
	constexpr static auto struct_fields_count() { return 6; }

	constexpr void execute(context_type& ctx) const override {
		auto result = jinja_expression_eval(f, condition);
		if (!!result) holder.execute(ctx);
	}

	constexpr static auto mk_parser(const auto& f) {
		using bp = base_parser<factory>;
		auto expr_parser = mk_jinja_expression_parser(f);
		constexpr auto trim_parser = trim_info<factory>::mk_parser();
		return skip(
		   ++lexeme(bp::mk_block_begin() >> trim_parser)
		>> def(p::template lit<"if">)
		>> ++mk_jinja_expression_parser(f)
		>> ++lexeme(trim_parser([](auto& v){return &v.left;}) >> bp::mk_block_end())
		>> th<0>::rec([](auto& v){return &v.holder;})
		>> fnum<4>(*if_else_block<factory>::mk_parser(f))
		>> fnum<3>(lexeme(trim_parser([](auto&v){return &v.right;}) >> bp::mk_block_begin()))
		>> p::template lit<"endif">++
		>> ++trim_parser >> bp::mk_block_end()
		);
	}
};

}
