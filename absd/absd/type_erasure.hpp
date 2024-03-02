//
// Created by zerhud on 02/03/24.
//

#pragma once

namespace absd::details {

struct inner_counter {
	unsigned long int ref_counter = 0;
	constexpr auto increase_counter() { return ++ref_counter; }
	constexpr auto decrease_counter() { return --ref_counter; }
};

struct counter_interface {
	virtual ~counter_interface() noexcept =default ;
	constexpr virtual decltype(inner_counter::ref_counter) increase_counter() =0 ;
	constexpr virtual decltype(inner_counter::ref_counter) decrease_counter() =0 ;
};

template<typename type>
struct counter_maker : inner_counter, type {
	constexpr counter_maker(auto&&... args) : type(std::forward<decltype(args)>(args)...) {}
	constexpr decltype(inner_counter::ref_counter) increase_counter() override { return inner_counter::increase_counter(); }
	constexpr decltype(inner_counter::ref_counter) decrease_counter() override { return inner_counter::decrease_counter(); }
};
template<typename data_type, typename from, typename type>
constexpr static data_type mk_coutner_and_assign(const auto& f, auto&& v) {
	data_type ret;
	auto tmp = f.mk_ptr(counter_maker<type>(std::forward<decltype(v)>(v)));
	ret.assign(static_cast<from*>(tmp.get()));
	tmp.release();
	return ret;
}

} // namespace absd::details
