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

static_assert( absd::details::as_object<jinja_ctx::out_info, data> );
static_assert( absd::details::as_array<jinja_ctx::output_frame, data> );
static_assert( absd::details::as_object<jinja_env, data>, "for check the reason if we cannot" );
static_assert( jinja_env{}.mk_context_data().is_object(), "context is data object" );

static_assert( [] {
	jinja_env env;
	data d = env.mk_context_data();
	env.add_global(data{"foo"}, data{3});
	auto keys = d.keys();
	return (d[data{"foo"}]==data{3})
	+ 2*d[data{"bar"}].is_none()
	+ 4*d.contains(data{"foo"})
	+ 8*d.contains("foo")
	+ 16*(d.size()==1)
	+ 32*(keys.size()==1)
	;
}() == 63, "can store global variables" );
static_assert( [] {
	jinja_env env;
	data d = env.mk_context_data();
	env.add_global(data{"glob"}, data{3});
	data::integer_t in_block=0, in_block_glob=0, in_block_glob2=0;
	{
		auto holder = env.push_frame();
		env.add_local(data{"name"}, data{7});
		in_block = (data::integer_t)d[data{"name"}];
		in_block_glob = (data::integer_t)d[data{"glob"}];
		env.add_local(data{"glob"}, data{11});
		in_block_glob2 = (data::integer_t)d[data{"glob"}];
	}
	return (in_block==7)
	+ 2*d[data{"name"}].is_none()
	+ 4*(in_block_glob==3)
	+ 8*(in_block_glob2==11)
	;
}() == 15, "can create frame and no variable after frame removed" );
static_assert( [] {
	jinja_env env;
	data d = env.mk_context_data();
	data::integer_t in_block1=0, in_block2=0, in_block2_keys_cnt=0;
	env.add_global(data{"glob"}, data{3});
	{
		auto holder1 = env.push_area();
		env.add_local(data{"name"}, data{7});
		{
			auto holder2 = env.push_area();
			env.add_local(data{"name"}, data{11});
			in_block2 = (data::integer_t)d[data{"name"}];
			in_block2_keys_cnt = env.keys(factory{}).size();
		}
		in_block1 = (data::integer_t)d[data{"name"}];
	}
	return (in_block1==7) + 2*(in_block2==11) + 4*(in_block2_keys_cnt==2);
}() == 7, "can create variables in area block" );
static_assert( [] {
	jinja_env env;
	data d = env.mk_context_data();
	auto holder_f1 = env.push_frame();
	env.add_local(data{"name"}, data{7});
	const auto v1 = (data::integer_t)d[data{"name"}];
	env.add_local(data{"name"}, data{11});
	const auto v2 = (data::integer_t)d[data{"name"}];
	auto holder_f2 = env.push_frame();
	env.add_local(data{"name"}, data{13});
	const auto v3 = (data::integer_t)d[data{"name"}];
	auto holder_a1 = env.push_area();
	env.add_local(data{"name"}, data{17});
	const auto v4 = (data::integer_t)d[data{"name"}];
	env.add_global(data{"glob"}, data{3});
	env.add_global(data{"glob"}, data{7});
	const auto v5 = (data::integer_t)d[data{"glob"}];
	return (v1==7)
	+ 2*(v2==11)
	+ 4*(v3==13)
	+ 8*(v4==17)
	+ 16*(v5==7)
	;
}() == 31, "can override variables" );
static_assert( [] {
	jinja_ctx ctx(factory{});
	data obj;
	obj.put(data{"key1"}, data{1});
	obj.put(data{"key2"}, data{3});
	ctx(data{"a"})(data{"b"})(obj);
	return ctx.cur_output().stringify();
}() == "ab{'key1':1,'key2':3}", "stringify_cur_output() turns current output to string");
static_assert( [] {
	jinja_ctx ctx(factory{});
	const jinja_details::trim_info<factory> ti{true};
	ctx(ti, data{" sp\nace \t"}, ti)(ti, data{" space \t"}, ti);
	return ctx.cur_output().stringify() ;
}() == " sp\nace space", "cur_output().stringify() trim spaces if required" );
static_assert( [] {
	jinja_ctx ctx(factory{});
	const jinja_details::shift_info si{2, false};
	ctx(si)(data{"tes\nt"});
	return ctx.cur_output().stringify() ;
}() == "tes\n\t\tt", "cur_output().stringify() shifts new lines with tabs if required" );
static_assert( [] {
	jinja_ctx ctx(factory{});
	const jinja_details::shift_info si{2, false};
	ctx(si)(data{"tes\nt"});
	(void)ctx.cur_output().stringify() ;
	return ctx.cur_output().stringify() ;
}() == "tes\n\t\tt", "cur_output().stringify() resets shift information on call" );

static_assert( [] {
	jinja_ctx c{ factory {} };
	c(data{"content"});
	auto out = c.extract_output();
	auto obj = data::mk(out);
	return (obj.size()==1) + 2*obj.is_array() + 4*obj[0].is_object();
}() == 7 );
static_assert( [] {
	jinja_ctx c{ factory {} };
	c(data{"content  "});
	c(jinja_details::trim_info<factory>{true}, data{"trim"}, jinja_details::trim_info<factory>{false});
	c(jinja_details::shift_info{3, true});
	auto out = c.extract_output();
	auto obj = data::mk(out);
	return (obj.size()==3) + 2*(obj[0][data{"value"}]=="content  ") + 4*(obj[0][data{"trim_before"}]==false) +
		8*(obj[1][data{"value"}]=="trim") + 16*(obj[1][data{"trim_before"}]==true) +
		32*obj[2][data{"value"}].is_none() + 64*(obj[2][data{"shift"}]==3) +
		64*((data::string_t)obj.call(data{})=="contenttrim"sv)
	;
}() == 127 );

int main(int,char**) {
}
