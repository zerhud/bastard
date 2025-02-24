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


using cnt_type = jinja_details::content<factory>;
constexpr auto make_content(auto&& val) {
	auto cnt = std::make_unique<cnt_type>(factory{});
	cnt->value = std::forward<decltype(val)>(val);
	return cnt;
}
static_assert( []{
	using op_type = jinja_details::content<factory>;
	op_type r1{factory{}};
	parse(r1.mk_parser(), +parser::space, parser::make_source("12 << test_text"), r1);
	op_type::context_type ctx{factory{}};
	r1.execute(ctx);
	auto [_,_,val] = ctx.cur_output().records[0].value();
	return (ctx.cur_output().size()==1) + 2*(val == "12 << test_text") ;
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
	using op_type = jinja_details::set_block<factory>;
	op_type r1{factory{}};
	r1._name = "test_name";
	r1.handler = test_expr{"test_expr"};
	auto cnt = std::make_unique<cnt_type>(factory{});
	cnt->value = "test content";
	r1.holder.holder.emplace_back() = std::move(cnt);
	op_type::context_type ctx{factory{}};
	r1.execute(ctx);
	auto result = ctx.env.at(data{"test_name"});
	return (ctx.env.size()==1) + 2*result.is_string() + 4*((data::string_t)result == "0: 'test_expr'") ;
}() == 7 );
static_assert( [] {
	using op_type =  jinja_details::if_block<factory>;
	op_type r1{factory{}}, r2{factory{}};
	r1.condition = test_expr{true};
	r2.condition = test_expr{false};
	auto cnt = std::make_unique<cnt_type>(factory{});
	cnt->value = "test content";
	r1.holder.holder.emplace_back() = std::make_unique<cnt_type>(*cnt);
	r2.holder.holder.emplace_back() = std::move(cnt);
	op_type::context_type ctx1{factory{}}, ctx2{factory{}};
	r1.execute(ctx1);
	r2.execute(ctx2);
	auto [_,_,val] = ctx1.cur_output().records[0].value();
	return val.is_string() + 2*(val=="test content") + 4*(ctx2.cur_output().size()==0);
}() == 7, "else block main part" );
static_assert( [] {
	using op_type =  jinja_details::if_block<factory>;
	op_type r1{factory{}}, r2{factory{}};
	r1.condition = test_expr{false};
	r2.condition = test_expr{false};
	r1.else_blocks.emplace_back(factory{});
	r2.else_blocks.emplace_back(factory{}).condition = test_expr{false};
	r2.else_blocks.emplace_back(factory{});
	r1.else_blocks[0].holder.holder.emplace_back(make_content("r1 test"));
	r2.else_blocks[0].holder.holder.emplace_back(make_content("r2 fail"));
	r2.else_blocks[1].holder.holder.emplace_back(make_content("r2 test"));
	op_type::context_type ctx1{factory{}}, ctx2{factory{}};
	r1.execute(ctx1);
	r2.execute(ctx2);
	auto [_,_,val1] = ctx1.cur_output().records[0].value();
	auto [_,_,val2] = ctx2.cur_output().records[0].value();
	return val1.is_string() + 2*(val1=="r1 test") + 4*val2.is_string() + 8*(val2=="r2 test");
}() == 15 );

int main(int,char**) {
}
