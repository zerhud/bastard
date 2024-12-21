#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <string>
#include <format>
#include <utility>
#include <string_view>
#include <source_location>

namespace tests {

struct graph_factory {
	using string_view = std::string_view;
	using source_location = std::source_location;

	template<typename type>
	constexpr static bool is_field_type() {
		constexpr bool is_opt = requires(const type& v){static_cast<bool>(v); *v; typename type::value_type;};
		if constexpr(is_opt) return is_field_type<typename type::value_type>();
		else return std::is_integral_v<type> || std::is_same_v<type, std::string>;
	}
};

template<template<typename...>typename list, typename... types, typename factory>
constexpr auto mk_graph_node_field_value(const factory&, const list<types...>&, auto&& val) {
	return typename factory::template variant<typename factory::empty_t, types...>{val};
}
template<template<typename...>typename list, typename... types, typename factory>
constexpr auto mk_graph_node_field_value(const factory&, const list<types...>&) {
	return typename factory::template variant<typename factory::empty_t, types...>{};
}

template<typename factory>
constexpr auto mk_data(const factory& f) {
	return typename factory::data_type{f};
}
template<typename factory>
constexpr auto mk_data(const factory&, std::string_view src) {
	using dt = typename factory::data_type;
	return dt{ typename dt::string_t{src} };
}
template<typename factory>
constexpr auto mk_data(const factory&, auto&& src) {
	return typename factory::data_type{ std::forward<decltype(src)>(src) };
}
template<typename factory>
[[noreturn]] void throw_vertex_not_present_in_graph(const factory&, const auto* vertex, const auto& graph) {
	using namespace std::literals;
	throw std::runtime_error(std::format("the vertex {}, is not present in the graph {}", (void*)vertex, (void*)&graph));
}
template<typename factory>
[[noreturn]] void throw_base_not_matched(const factory&) {
	using namespace std::literals;
	throw std::runtime_error("base doesn't match in in view operations");
}

} // namespace tests
