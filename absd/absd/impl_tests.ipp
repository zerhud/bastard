#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#ifdef __clang__
#define ctrt_begin return 1
#define ctrt(code) && ( code )
#define ctrt_gcc_workaround(code) && ( code )
#define ctrt_end ;
#else
#define ctrt_begin
#define ctrt(code) static_assert( code );
#define ctrt_gcc_workaround(code) ;
#define ctrt_end return true;
#endif

namespace absd {

template<typename factory>
constexpr bool data<factory>::test() {
#ifndef __clang__
	return test_simple_cases() && test_array_cases() && details::test_callable<self_type>();
#else
	return test_simple_cases() && test_array_cases();
#endif
}

template<typename factory>
constexpr bool data<factory>::test_simple_cases() {
	ctrt_begin
	ctrt( self_type{}.is_none() )
	ctrt( self_type{ (integer_t)10 }.is_none() == false )
	ctrt( self_type{ (integer_t)10 }.assign().is_none() )
	ctrt( self_type{ (integer_t)10 }.assign().is_none() )
	ctrt( self_type{ (float_point_t).5 }.is_int() == false )
	ctrt( self_type{ (float_point_t).5 }.assign( (integer_t)3 ).is_int() )
	ctrt( self_type{ string_t{} }.is_string() == true )
	ctrt( self_type{ string_t{} }.is_array() == false )
	ctrt( []{ self_type d; d=10; return (integer_t)d; }() == 10 )
	ctrt_gcc_workaround( []{ self_type d; d="hel"; auto ret = ((string_t)d)[2]; return ret; }() == 'l' )
	ctrt( self_type{ integer_t{} }.size() == sizeof(integer_t) )
	ctrt( self_type{ string_t{"hello"} }.size() == 5 )
	ctrt( self_type{3} == self_type{3} )
	ctrt( (bool)exec_operation(self_type{2}, self_type{3}, [](auto& l, auto& r)requires requires{l<r;}{ return l<r;}) == true )
	ctrt( (bool)exec_operation(self_type{3}, self_type{3}, [](auto& l, auto& r)requires requires{l<r;}{ return l<r;}) == false )
	ctrt( self_type{string_t{"abcd"}}.contains(self_type{string_t{"d"}}) )
	ctrt( !self_type{string_t{"abcd"}}.contains(self_type{string_t{"e"}}) )
	ctrt_end
}

template<typename factory>
constexpr bool data<factory>::test_array_cases() {
	ctrt_begin
	ctrt( self_type{}.mk_empty_array().is_array() == true )
	ctrt( self_type{}.mk_empty_array().is_object() == false )
	ctrt( self_type{(integer_t)10}.mk_empty_array().is_array() == true )
	ctrt( []{ self_type d; d.mk_empty_array(); d.push_back(self_type{(integer_t)10}); return (integer_t)d[0]; }() == 10 )
	ctrt( []{ self_type d; d.mk_empty_array(); d.push_back(self_type{(integer_t)10}); auto dd = d; return (integer_t)dd[0]; }() == 10 )
	ctrt( []{ self_type d; d.mk_empty_array(); d.push_back(self_type{(integer_t)10}); auto dd = std::move(d); return (integer_t)dd[0]; }() == 10 )
	ctrt( []{ self_type d; d.mk_empty_array(); d.push_back(self_type{(integer_t)10}); auto dd = std::move(d); return dd.size(); }() == 1 )

	ctrt( []{
		auto vec = factory{}.template mk_vec<self_type>();
		vec.emplace_back(self_type{(integer_t)1});
		vec.emplace_back(self_type{(integer_t)3});
		auto d = self_type::mk(std::move(vec));
		return (integer_t)d[1]; }() == 3)

	ctrt( []{
		self_type d;
		d.push_back(self_type{3});
		return (integer_t)d[self_type{0}];
	}() == 3 )

	ctrt( []{
		self_type d;
		d.push_back(self_type{2});
		d.push_back(self_type{3});
		return d.contains(self_type{3});
	}() == true );

	ctrt_end
}

} // namespace absd
