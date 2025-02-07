/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "factory.hpp"

#include "absd/iostream_op.hpp"
#include <iostream>

using namespace std::literals;


static_assert( []{
	using op_type = jinja_details::content<factory>;
	op_type r1{factory{}};
	parse(r1.mk_parser(), +parser::space, parser::make_source("12 << test_text"), r1);
	op_type::context_type ctx{factory{}};
	r1.execute(ctx);
	auto [_,_,val] = ctx.cur_output().records[0].value();
	return (ctx.cur_output().size()==1) +
		2*(val == "12 << test_text")
		;
}() == 3, "content appended to context");
static_assert( []{
	using op_type = jinja_details::expression_operator<factory>;
	op_type r1{factory{}};
	parse(r1.mk_parser(factory{}), +parser::space, parser::make_source("<= 3 =>"), r1);
	op_type::context_type ctx{factory{}};
	r1.execute(ctx);
	auto [_,_,val] = ctx.cur_output().records[0].value();
	return (ctx.cur_output().size() == 1) + 2*(val=="1: '3'");
}() == 3, "the expression operator appends to context the result of the expression" );
static_assert( [] {
	using cnt_type = jinja_details::content<factory>;
	using op_type = jinja_details::set_block<factory>;
	op_type r1{factory{}};
	r1._name = "test_name";
	r1.handler = test_expr{"test_name"};
	auto cnt = std::make_unique<cnt_type>(factory{});
	cnt->value = "test content";
	r1.holder.holder.emplace_back() = std::move(cnt);
	op_type::context_type ctx{factory{}};
	r1.execute(ctx);
	auto result = ctx.env.at(data{"test_name"});
	return (ctx.env.size()==1) + 2*result.is_array() + 4*(result.size()==1) +
	8*(result[0][data{"value"}] == data{"test content"})
	;
}() == 15 );

int main(int,char**) {
}
