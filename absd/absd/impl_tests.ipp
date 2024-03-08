//
// Created by zerhud on 07/03/24.
//

#pragma once


namespace absd {

template<typename factory>
constexpr bool data<factory>::test() {
	return test_simple_cases() && test_array_cases() && details::test_callable<self_type>() && test_object_cases();
}

template<typename factory>
constexpr bool data<factory>::test_simple_cases() {
	static_assert( self_type{}.is_none(), "data is empty by defualt" );
	static_assert( self_type{ (integer_t)10 }.is_none() == false );
	static_assert( self_type{ (integer_t)10 }.assign().is_none() );
	static_assert( self_type{ (float_point_t).5 }.is_int() == false );
	static_assert( self_type{ (float_point_t).5 }.assign( (integer_t)3 ).is_int() );
	static_assert( self_type{ string_t{} }.is_string() == true );
	static_assert( self_type{ string_t{} }.is_array() == false );
	static_assert( []{ self_type d; d=10; return (integer_t)d; }() == 10 );
	//NOTE: string cannot to be tested in compile time due gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111284
	//	static_assert( []{ self_type d; d="hel"; auto ret = ((string_t)d)[2]; return ret; }() == 'l' );
	static_assert( self_type{ integer_t{} }.size() == sizeof(integer_t) );
	static_assert( self_type{ string_t{"hello"} }.size() == 5 );
	static_assert( self_type{3} == self_type{3});
	return true;
}

template<typename factory>
constexpr bool data<factory>::test_array_cases() {
	static_assert( self_type{}.mk_empty_array().is_array() == true );
	static_assert( self_type{}.mk_empty_array().is_object() == false );
	static_assert( self_type{(integer_t)10}.mk_empty_array().is_array() == true );
	static_assert( []{ self_type d; d.mk_empty_array(); d.push_back(self_type{(integer_t)10}); return (integer_t)d[0]; }() == 10 );
	static_assert( []{ self_type d; d.mk_empty_array(); d.push_back(self_type{(integer_t)10}); auto dd = d; return (integer_t)dd[0]; }() == 10 );
	static_assert( []{ self_type d; d.mk_empty_array(); d.push_back(self_type{(integer_t)10}); auto dd = std::move(d); return (integer_t)dd[0]; }() == 10 );
	static_assert( []{ self_type d; d.mk_empty_array(); d.push_back(self_type{(integer_t)10}); auto dd = std::move(d); return dd.size(); }() == 1 );

	static_assert( []{
		auto vec = factory{}.template mk_vec<self_type>();
		vec.emplace_back(self_type{(integer_t)1});
		vec.emplace_back(self_type{(integer_t)3});
		auto d = self_type::mk(std::move(vec));
		return (integer_t)d[1]; }() == 3);

	return true;
}

template<typename factory>
constexpr bool data<factory>:: test_object_cases() {
	static_assert( self_type{}.mk_empty_object().is_object() == true );
	static_assert( self_type{}.mk_empty_object().is_array() == false );
	static_assert( []{self_type d{}; d.mk_empty_object(); d.put(self_type{1}, self_type{7}); return (integer_t)d[self_type{1}];}() == 7 );
	static_assert( []{ self_type d; d.put(self_type{1}, self_type{7}); d.put(self_type{2}, self_type{8}); return d.size(); }() == 2 );
	static_assert( []{ self_type d;
		d.put(self_type{1}, self_type{7});
		d.put(self_type{2}, self_type{8});
		auto keys = d.keys();
		return (keys.size() == 2) + (2*((integer_t)keys[0] == 1)) + (4*((integer_t)keys[1] == 2)); }() == 7 );

	static_assert( []{
		details::constexpr_kinda_map<factory,self_type,self_type> v;
		v.insert(std::make_pair(self_type{1}, self_type{2}));
		return v.contains(self_type{1});
	}() );
	static_assert( (integer_t)[]{
		details::constexpr_kinda_map<factory,self_type,self_type> v;
		v.insert(std::make_pair(self_type{1}, self_type{3}));
		return self_type::mk(std::move(v))[self_type{1}];}() == 3 );

	static_assert( []{
		self_type d;d.mk_empty_object();
		d.put(self_type{1}, self_type{7});
		return d.contains(self_type{1}) + (2*!d.contains(self_type{7}));
	}() == 3);
	return true;
}

} // namespace absd
