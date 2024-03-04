//
// Created by zerhud on 03/03/24.
//

#pragma once

#include "type_erasure.hpp"
#include "callable.hpp"

namespace absd::details {

template<typename factory, typename data>
struct type_erasure_callable : counter_interface {
	using factory_t = factory;
	using string_t = typename data::string_t;
	struct parameter_descriptor {
		string_t name;
		data default_value;
	};
	using params_t = decltype(std::declval<factory_t>().template mk_vec<parameter_descriptor>());

	constexpr virtual ~type_erasure_callable() noexcept =default ;
	constexpr virtual data call(data params) =0 ;
	//TODO: params never used
	constexpr virtual params_t params(const factory_t& f) const =0 ;
};

template<typename data_type, typename factory>
constexpr auto mk_te_callable(const factory& f, auto&& v, auto&&... args) {
	using v_type = decltype(data_type::mk_ca(std::forward<decltype(v)>(v), std::forward<decltype(args)>(args)...));
	using te_callable = type_erasure_callable<factory, data_type>;
	struct te : te_callable {
		v_type val;
		constexpr te(v_type v) : val(std::move(v)) {}
		constexpr data_type call(data_type params) override { return data_type{val.call(std::move(params))}; }
		constexpr te_callable::params_t params(const factory& f) const override {
			return {};
		}
	};
	return mk_coutner_and_assign<data_type, te_callable, te>(f, data_type::mk_ca(std::forward<decltype(v)>(v), std::forward<decltype(args)>(args)...));
}

template<typename data_type>
constexpr bool test_callable() {
	using int_t = typename data_type::integer_t;
	static_assert( data_type::mk([]{}).is_callable() );
	static_assert( data_type::mk([]{}).call(data_type::mk_map()).is_none() );
	static_assert( (int_t)data_type::mk([]{return 1;}).call(data_type::mk_map()) == 1);
	static_assert( (int_t)data_type::mk([](int a){return a-1;}, data_type::mk_param("a")).call(data_type::mk_map(data_type{0}, data_type{3})) == 2 );

	static_assert( (int_t)data_type::mk([](int a, int b){return a-b;}, data_type::mk_param("a"), data_type::mk_param("b", data_type{1})).call(data_type::mk_map(0, 3)) == 2);

	constexpr auto amb = []{return data_type::mk([](int a, int b){return a-b;}, data_type::mk_param("a", data_type{3}), data_type::mk_param("b")); };
	static_assert( (int_t)amb().call(data_type::mk_map("b", 3)) == 0);
	static_assert( (int_t)amb().call(data_type::mk_map(0, 7, "b", 3)) == 4);
	static_assert( (int_t)amb().call(data_type::mk_map("b", 3, 0, 7)) == 4);

	return true;
}

} // namespace absd::details
