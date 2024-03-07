//
// Created by zerhud on 07/03/24.
//

#pragma once


namespace absd {

template<typename factory, typename crtp>
constexpr bool data<factory, crtp>::test() {
	return test_simple_cases() && test_array_cases() && details::test_callable<self_type>() && test_object_cases();
}

template<typename factory, typename crtp>
constexpr bool data<factory, crtp>:: test_simple_cases() {
	struct data_type : data<factory,data_type> {using data<factory,data_type>::operator=;};

	static_assert( data_type{}.is_none(), "data is empty by defualt" );
	static_assert( data_type{ (integer_t)10 }.is_none() == false );
	static_assert( data_type{ (integer_t)10 }.assign().is_none() );
	static_assert( data_type{ (float_point_t).5 }.is_int() == false );
	static_assert( data_type{ (float_point_t).5 }.assign( (integer_t)3 ).is_int() );
	static_assert( data_type{ string_t{} }.is_string() == true );
	static_assert( data_type{ string_t{} }.is_array() == false );
	static_assert( []{ data_type d; d=10; return (integer_t)d; }() == 10 );
	//NOTE: string cannot to be tested in compile time due gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111284
	//	static_assert( []{ data_type d; d="hel"; auto ret = ((string_t)d)[2]; return ret; }() == 'l' );
	static_assert( data_type{ integer_t{} }.size() == sizeof(integer_t) );
	static_assert( data_type{ string_t{"hello"} }.size() == 5 );
	static_assert( data_type{3} == data_type{3});
	return true;
}

template<typename factory, typename crtp>
constexpr bool data<factory, crtp>::test_array_cases() {
	struct data_type : data<factory,data_type> {using data<factory,data_type>::operator=;};

	static_assert( data_type{}.mk_empty_array().is_array() == true );
	static_assert( data_type{}.mk_empty_array().is_object() == false );
	static_assert( data_type{(integer_t)10}.mk_empty_array().is_array() == true );
	static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); return (integer_t)d[0]; }() == 10 );
	static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); auto dd = d; return (integer_t)dd[0]; }() == 10 );
	static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); auto dd = std::move(d); return (integer_t)dd[0]; }() == 10 );
	static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); auto dd = std::move(d); return dd.size(); }() == 1 );

	static_assert( []{
		auto vec = factory{}.template mk_vec<data_type>();
		vec.emplace_back(data_type{(integer_t)1});
		vec.emplace_back(data_type{(integer_t)3});
		auto d = data_type::mk(std::move(vec));
		return (integer_t)d[1]; }() == 3);

	return true;
}

template<typename factory, typename crtp>
constexpr bool data<factory, crtp>:: test_object_cases() {
	struct data_type : data<factory,data_type> {using data<factory,data_type>::operator=;};

	static_assert( data_type{}.mk_empty_object().is_object() == true );
	static_assert( data_type{}.mk_empty_object().is_array() == false );
	static_assert( []{data_type d{}; d.mk_empty_object(); d.put(data_type{1}, data_type{7}); return (integer_t)d[data_type{1}];}() == 7 );
	static_assert( []{ data_type d; d.put(data_type{1}, data_type{7}); d.put(data_type{2}, data_type{8}); return d.size(); }() == 2 );
	static_assert( []{ data_type d;
		d.put(data_type{1}, data_type{7});
		d.put(data_type{2}, data_type{8});
		auto keys = d.keys();
		return (keys.size() == 2) + (2*((integer_t)keys[0] == 1)) + (4*((integer_t)keys[1] == 2)); }() == 7 );

	static_assert( []{
		details::constexpr_kinda_map<factory,data_type,data_type> v;
		v.insert(std::make_pair(data_type{1}, data_type{2}));
		return v.contains(data_type{1});
	}() );
	static_assert( (integer_t)[]{
		details::constexpr_kinda_map<factory,data_type,data_type> v;
		v.insert(std::make_pair(data_type{1}, data_type{3}));
		return data_type::mk(std::move(v))[data_type{1}];}() == 3 );

	static_assert( []{
		data_type d;d.mk_empty_object();
		d.put(data_type{1}, data_type{7});
		return d.contains(data_type{1}) + (2*!d.contains(data_type{7}));
	}() == 3);
	return true;
}

} // namespace absd
