//
// Created by zerhud on 27.02.24.
//

#pragma once

#include "callable.hpp"
#include "../absd.hpp"

namespace absd::details::tests {

template<typename data_type> constexpr void mk_params_impl(data_type& params, auto&& key, auto&& val, auto&&... args) {
	params.put(data_type{std::forward<decltype(key)>(key)}, data_type{std::forward<decltype(val)>(val)});
	if constexpr (sizeof...(args)!=0) mk_params_impl(params, std::forward<decltype(args)>(args)...);
}
template<typename data_type> constexpr auto mk_params(auto&&... args) requires (sizeof...(args)%2 == 0) {
	data_type params;
	mk_params_impl(params, std::forward<decltype(args)>(args)...);
	return params;
}

template<typename data_type>
constexpr auto test_callable_ab(auto&&... args) {
	using integer_t = typename data_type::integer_t;
	auto wd = data_type::mk_ca(
			[](int a, int b){return a-b;},
			data_type::mk_param("a"),
			data_type::mk_param("b", data_type{3}) );
	return (integer_t)wd.call(mk_params<data_type>(std::forward<decltype(args)>(args)...));
}

template<typename data_type>
constexpr bool callable2_test() {

	static_assert( data_type::mk_ca([]{return 1;})() == 1 );
	static_assert( data_type::mk_ca([](int i){return i+1;})(3) == 4 );
	static_assert( data_type::mk_ca([](int a, int b){return a+b;})(3,2) == 5 );

	static_assert( data_type::mk_ca([](int a, int b){return a+b;}, data_type::mk_param("a"), data_type::mk_param("b"))(1,2) == 3);
	static_assert( data_type::mk_ca(
			[](int a, int b){return a+b;},
			data_type::mk_param("a"),
			data_type::mk_param("b", data_type{3})
			)(1) == 4);
	static_assert( test_callable_ab<data_type>(0, 3, 1, 1) == 2 );
	static_assert( test_callable_ab<data_type>("a", 3, "b", 1) == 2 );
	static_assert( test_callable_ab<data_type>(0, 3, "b", 1) == 2 );
	static_assert( test_callable_ab<data_type>("a", 3, 1, 1) == 2 );

	static_assert( []{
		int ret=2;
		data_type::mk_ca([&ret](int){++ret;})(3);
		return ret;
	}() == 3, "void lambda should to be called with () operator");

	static_assert( []{
		int ret=2;
		auto result = data_type::mk_ca([&ret](int){++ret;}, data_type::mk_param("a")).call(mk_params<data_type>(0, 3));
		return (ret==3) + result.is_none();
	}() == 2, "void lambda should to be called with call() method and returns empty data object" );

	return true;
}

} // namespace absd::details::tests
