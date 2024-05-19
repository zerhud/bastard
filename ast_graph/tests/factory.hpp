/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "ast_graph/field_names.hpp"

#ifndef __clang__
#define CTRT(code) static_assert( code );
#else
#include <cassert>
#define CTRT(code) assert( code );
#endif
#define RT(code)  assert( code );
#include "ast_graph/node.hpp"

#include <variant>
#include <vector>
#include <memory>
#include <string_view>
#include <source_location>

using namespace std::literals;

namespace ast_graph_tests {

template<typename type>
struct clang_bug_74963_wa {
	//NOTE: std::unique_ptr should to be used, but clang has a bug
	//      https://github.com/llvm/llvm-project/issues/74963

	clang_bug_74963_wa(const clang_bug_74963_wa&&) =delete ;

	constexpr clang_bug_74963_wa() : v(nullptr) {}
	constexpr clang_bug_74963_wa(type* v) : v(v) {}
	constexpr clang_bug_74963_wa(clang_bug_74963_wa&& other) : v(other.v) {
		other.v = nullptr;
	}
	constexpr clang_bug_74963_wa& operator=(clang_bug_74963_wa&& other) {
		if(v) delete v;
		v = other.v;
		other.v = nullptr;
		return *this;
	}

	constexpr ~clang_bug_74963_wa() {
		// this warning same bug about
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wdelete-incomplete"
		if(v) delete v;
		#pragma clang diagnostic pop
	}

	constexpr type& operator*() { return *v; }
	constexpr const type& operator*() const { return *v; }

	constexpr type* operator->() { return v; }
	constexpr const type* operator->() const { return v; }

	constexpr auto* get() const { return v; }

	type* v;
};

struct factory {
	using string_view = std::string_view;
	using source_location = std::source_location;

	template<typename... types>
	using variant = std::variant<types...>;

	template<typename... types>
	constexpr static auto mk_variant() {
		return variant<types...>{};
	}

	constexpr auto alloc_smart(auto&& v) const -> clang_bug_74963_wa<std::decay_t<decltype(v)>> {
		using vt = std::decay_t<decltype(v)>;
		return clang_bug_74963_wa<vt>(new vt(std::forward<decltype(v)>(v)));
		//return std::make_unique<std::decay_t<decltype(v)>>(std::forward<decltype(v)>(v));
	}

	constexpr auto mk_edge_name() const { return std::string(); }

	template<typename type>
	constexpr static auto mk_vec(auto&&... items) {
		std::vector<std::decay_t<type>> ret;
		(void)(ret.emplace_back(std::forward<decltype(items)>(items)), ...);
		return ret;
	}

	template<template<typename...>typename list, typename... types>
	constexpr auto mk_val(const list<types...>&, auto&& val) const {
		return std::variant<std::monostate, types...>{val};
	}
	template<template<typename...>typename list, typename... types>
	constexpr auto mk_val(const list<types...>&) const {
		return std::variant<std::monostate, types...>{};
	}

	template<typename type>
	constexpr static bool is_field_type() {
		return std::is_integral_v<type> || std::is_same_v<type, std::string>;
	}

	[[noreturn]] static void throw_no_value_access() {
		throw std::runtime_error("access to no_value");
	}
};

} // namespace ast_graph_tests
