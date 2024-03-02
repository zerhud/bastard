//
// Created by zerhud on 01/03/24.
//

#pragma once

#include <utility>
#include "type_erasure.hpp"

namespace absd::details {

template<typename factory, typename data>
struct array : inner_counter {
	using vec_content = data;

	[[no_unique_address]] factory f;
	std::decay_t<decltype(std::declval<factory>().template mk_vec<vec_content>())> holder;


	constexpr array(factory _f) requires (requires{ _f.template mk_vec<vec_content>(); }) : f(std::move(_f)), holder(f.template mk_vec<vec_content>()) {}
	constexpr array(factory _f) requires (!requires{ _f.template mk_vec<vec_content>(); }) : f(std::move(_f)) {}

	constexpr data& emplace_back(data d) { return holder.emplace_back(std::move(d)); }
	constexpr data& at(std::integral auto ind) { return holder.at(ind); }
	constexpr auto size() const { return holder.size(); }
};
template<typename data, typename factory> constexpr auto mk_array_type(const factory& f) {
	if constexpr(!requires{ typename factory::array_t; }) return array<factory, data>(f);
	else return typename factory::array_t{};
}

template<typename factory, typename data>
struct type_erasure_array : counter_interface {
	constexpr virtual ~type_erasure_array() noexcept =default ;

	constexpr virtual data& emplace_back(data d) =0 ;
	constexpr virtual data& at(typename data::integer_t ind) =0 ;
	constexpr virtual decltype(sizeof(data)) size() const =0 ;
};

template<typename data_type>
constexpr auto mk_te_array(const auto& f, auto&& src) {
	using v_type = std::decay_t<decltype(src)>;
	using te_array = type_erasure_array<typename data_type::factory_t, data_type>;
	struct te : te_array {
		v_type val;
		constexpr te(v_type v) : val(std::move(v)) {}

		constexpr data_type& emplace_back(data_type d) override { return val.emplace_back(std::move(d)); }
		constexpr data_type& at(typename data_type::integer_t ind) override { return val.at(ind); }
		constexpr decltype(sizeof(data_type)) size() const override { return val.size(); }
	};
	return mk_coutner_and_assign<data_type, te_array, te>(f, std::forward<decltype(src)>(src));
}

} // namespace absd::details
