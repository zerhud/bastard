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
constexpr auto make_expr(auto&& val) {
  auto ret = std::make_unique<jinja_details::expression_operator<factory>>(factory{});
  ret->expr = test_expr{std::forward<decltype(val)>(val)};
  return ret;
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
  ctx.f.on_eval = [](data env, const test_expr& e) {
    if ((data::string_t)(env[data{"test_name"}][0][data{"value"}]) != "test content") throw 0;
    if (get<0>(e)!="test_expr") throw 1;
  };
  r1.execute(ctx);
  auto result = ctx.env.at(data{"test_name"});
  delete ctx.f.on_eval.impl;
  return (ctx.env.size()==1) + 2*result.is_string() + 4*((data::string_t)result == "0: 'test_expr'") ;
}() == 7, "the value of set_block is a result of block execution - array of output objects" );

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
}() == 7, "if block: main part" );
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
}() == 15, "if block: else part" );

static_assert( [] {
  using op_type =  jinja_details::for_block<factory>;
  op_type r1{factory{}};
  r1.for_exprs.exprs.emplace_back(op_type::for_expr{factory{}});
  r1.for_exprs.exprs[0].expr = {"abc"};
  r1.for_exprs.exprs[0].variables.emplace_back("a");
  r1.holder.holder.emplace_back(make_content("t"));
  op_type::context_type ctx1{factory{}};
  r1.execute(ctx1);
  auto [_,_,val1] = ctx1.cur_output().records[7].value();
  return (ctx1.cur_output().records.size()==8) + 2*(val1 == "t");
}() == 3, "for_block simple execution" );

static_assert( [] {
  using op_type = jinja_details::named_block<factory>;
  op_type r1{factory{}};
  r1._name = "foo";
  r1.parameters.emplace_back().name = "p1";
  r1.parameters.back().value = test_expr{1};
  r1.holder.holder.emplace_back(make_expr(7));
  op_type::context_type ctx1{factory{}};
  std::size_t env_count = 0;
  ctx1.f.on_eval = [&](data env, const test_expr& e) {
    env_count += env.size();
    if (env.size()!=0&&env[data{"p1"}].is_none()) throw 0;
  };
  r1.execute(ctx1);
  auto [_,_,val1] = ctx1.cur_output().records[0].value();
  delete ctx1.f.on_eval.impl;
  return (ctx1.cur_output().records.size()==1) + 2*(env_count==1) + 4*(val1 == "1: '7'") + 8*(ctx1.env.size()==0);
}() == 15, "pass parameters in named block" );

static_assert( [] {
  using op_type = jinja_details::named_block<factory>;
  op_type r1{factory{}};
  r1._name = "foo";
  r1.parameters.emplace_back().name = "p1";
  r1.parameters.back().value = test_expr{1};
  r1.holder.holder.emplace_back(make_expr(7));
  op_type::context_type ctx1{factory{}};
  auto obj = data::mk(jinja_details::named_env_object<factory>{&r1, &ctx1});
  std::size_t env_count = 0;
  ctx1.f.on_eval = [&](data env, const test_expr& e) {
    env_count += env.size();
    if (env.size()!=0&&env[data{"p1"}].is_none()) throw 0;
  };
  data params; params.put(data{"p1"}, data{100});
  auto result = obj.call(params);
  delete ctx1.f.on_eval.impl;
  return obj.is_callable() + 2*(ctx1.cur_output().records.size()==0) + 4*(env_count==1) + 8*(result[0][data{"value"}] == "1: '7'") + 16*(ctx1.env.size()==0) + 32*(obj[data{"name"}]=="foo");
}() == 63 );

int main(int,char**) {
}
