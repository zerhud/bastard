/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"
#include "absd.hpp"

#include <iostream>

using namespace std::literals;

static_assert( absd::details::type_c<int> == absd::details::type_c<int> );
static_assert( absd::details::type_c<int> != absd::details::type_c<char> );
static_assert( !(absd::details::type_c<int> == absd::details::type_c<char>) );

template<typename factory>
constexpr void main_test() {
	using data = absd::data<factory>;
	using integer_t = data::integer_t;
	using float_point_t = data::float_point_t;
	using string_t = data::string_t;

	static_assert( data{}.is_none() );
	static_assert( data{factory{}}.is_none() );
	static_assert( data{ (integer_t)10 }.is_none() == false );
	static_assert( data{ (integer_t)10 }.assign().is_none() );
	static_assert( data{ (integer_t)10 }.assign().is_none() );
	static_assert( data{ (float_point_t).5 }.is_int() == false );
	static_assert( data{ (float_point_t).5 }.assign( (integer_t)3 ).is_int() );
	static_assert( data{ string_t{} }.is_string() == true );
	static_assert( data{ string_t{} }.is_array() == false );
	static_assert( []{ data d; d=10; return (integer_t)d; }() == 10 );
	static_assert( []{ data d; d="hel"; auto ret = ((string_t)d)[2]; return ret; }() == 'l' );
	static_assert( data{ integer_t{} }.size() == sizeof(integer_t) );
	static_assert( data{ string_t{"hello"} }.size() == 5 );
	static_assert( data{3} == data{3} );
	static_assert( (bool)exec_operation(data{2}, data{3}, [](auto& l, auto& r)requires requires{l<r;}{ return l<r;}) == true );
	static_assert( (bool)exec_operation(data{3}, data{3}, [](auto& l, auto& r)requires requires{l<r;}{ return l<r;}) == false );
	static_assert(  data{string_t{"abcd"}}.contains(string_t{"d"}) );
	static_assert( !data{string_t{"abcd"}}.contains(string_t{"e"}) );
	static_assert( data{string_t{"abcd"}}.contains(data{string_t{"d"}}) );
	static_assert( []{
		string_t str = "test";
		data d{&str};
		return (d.size() == 4) + 2*(d == data{"test"}) + 4*(d == data{&str}) + 8*(d=="test");
	}() == 15, "works with string pointer" );
	static_assert( data{ "foo" } == "foo", "can compare with value" );
	static_assert( data{ 100 } == 100, "can compare with value" );
	static_assert( data{ 100 } != 101, "can compare with value" );
	static_assert( data{ true } == true, "can compare with value" );
	static_assert( data{} == data{}, "none is equal to none only" );
	static_assert( data{} != 1, "none is equal to none only" );
}

constexpr std::string test_format(auto&& d) {
	std::string ret;
	absd::back_insert_format(std::back_inserter(ret), std::forward<decltype(d)>(d));
	return ret;
}

template<typename data_type>
constexpr void test_format() {
	static_assert( "1"sv == test_format(data_type{1}) );
	static_assert( "test"sv == test_format(data_type{"test"}) );
	static_assert( "[]"sv == test_format(data_type{}.mk_empty_array()) );
	static_assert( "[1]"sv == []{
		data_type src;
		src.push_back(data_type{1});
		return test_format(src);
	}() );
	static_assert( "[1,2]"sv == []{
		data_type src;
		src.push_back(data_type{1});
		src.push_back(data_type{2});
		return test_format(src);
	}() );
}

template<typename data_type>
constexpr void test_format_rt_due_gcc_bug(auto&& test_fnc) {
	{
		data_type src;
		src.push_back(data_type{1});
		src.push_back(data_type{"str"});
		src.push_back(data_type{typename data_type::string_t{"'t\\r"}});
		test_fnc(R"-([1,'str','\'t\\r'])-", test_format(src));
	}

	{
		test_fnc("1.2", test_format(data_type{1.2}));
		test_fnc("-1.2", test_format(data_type{-1.2}));
		test_fnc("-1", test_format(data_type{-1.0}));
	}

	{
		data_type src;
		src.mk_empty_object();
		test_fnc("{}", test_format(src));
		src.put(data_type{1}, data_type{2});
		test_fnc("{1:2}", test_format(src));
		src.put(data_type{3}, data_type{"string"});
		test_fnc("{1:2,3:'string'}", test_format(src));
		src.put(data_type{"str'key"}, data_type{5});
		test_fnc("{1:2,3:'string','str\\'key':5}", test_format(src));
	}
}

template<typename fp> struct absd_factory : tests::factory {
	using float_point_t = fp;
};

int main(int,char**) {
	using absd_data = absd::data<absd_factory<double>>;
	main_test<absd_factory<float>>();
	main_test<absd_factory<double>>();

	test_format_rt_due_gcc_bug<absd_data>([](auto result, auto test_obj){
		if(result != test_obj) std::cout << "ERROR: " << result << "!=" << test_obj << std::endl;
	}) ;

	test_format<absd::data<absd_factory<float>>>();
	test_format<absd::data<absd_factory<double>>>();

	return 0;
}
