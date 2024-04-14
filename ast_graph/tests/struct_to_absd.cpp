/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include "ast_graph/node.hpp"

#include "absd.hpp"

#include <vector>
#include <memory>
#include <string>
#include <variant>
#include <string_view>
#include <source_location>

using namespace std::literals;

struct absd_factory {
	template<typename... types> using variant = std::variant<types...>;
	template<typename type> using vector = std::vector<type>;
	using float_point_t = double;
	using string_t = std::string;
	using empty_t = std::monostate;

	template<typename type> constexpr static auto mk_vec(){ return std::vector<type>{}; }
//	template<typename key, typename value>
//	constexpr static auto mk_map(){ return map_t<key,value>{}; }
	constexpr static auto mk_ptr(auto d) { return std::make_unique<decltype(d)>( std::move(d) ); }
	constexpr static void deallocate(auto* ptr) noexcept { delete ptr; }
	template<typename interface>
	[[noreturn]] constexpr static void throw_wrong_interface_error() {
		throw std::runtime_error("cannot perform operation "s + interface::describe_with_chars());
	}
	template<auto cnt>
	[[noreturn]] constexpr static void throw_wrong_parameters_count() {
		throw std::runtime_error("wrong arguments count: " + std::to_string(cnt));
	}
};

struct graph_factory {
	using string_view = std::string_view;
	using source_location = std::source_location;

	template<template<typename...>typename list, typename... types>
	constexpr auto mk_val(const list<types...>&, auto&& val) const {
		return std::variant<std::monostate, types...>{val};
	}
	template<template<typename...>typename list, typename... types>
	constexpr auto mk_val(const list<types...>&) const {
		return std::variant<std::monostate, types...>{};
	}
};

using absd_data = absd::data<absd_factory>;

constexpr auto to_str(const absd_data& d) {
	std::string ret;
	back_insert_format(std::back_inserter(ret), d);
	return ret;
}

struct simple_node {
	int f1 = 3;
};

int main(int,char**) {
	simple_node obj;
	ast_graph::node<graph_factory, simple_node> node{{}, &obj};
	std::cout << "node: ";
	visit([](const auto& v){
		if constexpr (!std::is_same_v<std::monostate,std::decay_t<decltype(v)>>) std::cout << v;
		}, node.value("f1"));
	std::cout << std::endl;
	auto data = absd_data::mk(node);
	std::cout << "we have ("sv << data.is_array() << ") "sv << to_str( data ) << std::endl;
	return 0;
}