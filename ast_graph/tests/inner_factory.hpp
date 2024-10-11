#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#ifndef __clang__
#define CTRT(code) static_assert( code );
#else
#include <cassert>
#define CTRT(code) assert( code );
#endif
#include <cassert>
#define RT(code)  assert( code );
#include "ast_graph/node.hpp"

#include <list>
#include <memory>
#include <vector>
#include <variant>
#include <optional>
#include <string_view>
#include <source_location>
#include "absd.hpp"

using namespace std::literals;

namespace ast_graph_tests {

namespace detail {
template<uint8_t... digits> struct positive_to_chars { static const char value[]; };
template<uint8_t... digits> const char positive_to_chars<digits...>::value[] = {('0' + digits)..., 0};

template<uint8_t... digits> struct negative_to_chars { static const char value[]; };
template<uint8_t... digits> const char negative_to_chars<digits...>::value[] = {'-', ('0' + digits)..., 0};

template<bool neg, uint8_t... digits> struct to_chars : positive_to_chars<digits...> {};
template<uint8_t... digits> struct to_chars<true, digits...> : negative_to_chars<digits...> {};
template<bool neg, uintmax_t rem, uint8_t... digits> struct explode : explode<neg, rem / 10, rem % 10, digits...> {};
template<bool neg, uint8_t... digits> struct explode<neg, 0, digits...> : to_chars<neg, digits...> {};
template<typename T> constexpr uintmax_t cabs(T num) { return (num < 0) ? -num : num; }
}

template<typename T, T num>
struct string_from : detail::explode<num < 0, detail::cabs(num)> {};

template<typename type>
struct clang_bug_74963_wa {
	//NOTE: std::unique_ptr should to be used, but clang has a bug
	//      https://github.com/llvm/llvm-project/issues/74963

	using element_type = type;

	clang_bug_74963_wa(const clang_bug_74963_wa&&) =delete ;

	constexpr clang_bug_74963_wa() : v(nullptr) {}
	constexpr clang_bug_74963_wa(std::unique_ptr<type> v) : v(v.get()) { v.release(); }
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
	constexpr void reset() { if(v) delete v; v=nullptr; }
	constexpr type* reset(type* nv) {
		reset();
		v = nv;
		return v;
	}

	constexpr operator bool() const { return !!v; }
	constexpr friend bool operator==(const clang_bug_74963_wa<type>& l, const type* r) { return l.v == r; }

	type* v;
};

struct absd_factory {
	template<typename... types> using variant = std::variant<types...>;
	template<typename type> using vector = std::vector<type>;
	using float_point_t = double;
	using string_t = std::string;
	using empty_t = std::monostate;

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
[[noreturn]] void throw_key_not_found(const absd_factory&, const auto&) {
	using namespace std::literals;
	throw std::runtime_error("attempt to get value by nonexistent key: "s);
}

constexpr auto mk_ptr(const absd_factory&, auto d) { return std::make_unique<decltype(d)>( std::move(d) ); }
template<typename type> constexpr auto mk_vec(const absd_factory&){ return std::vector<type>{}; }

struct inner_factory {
	using string_view = std::string_view;
	using source_location = std::source_location;
	using data_type = absd::data<absd_factory>;

	template<typename... types>
	using variant = std::variant<types...>;

	template<typename... types>
	constexpr static auto mk_variant() {
		return variant<types...>{};
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
		if constexpr(requires(const type& v){static_cast<bool>(v); *v; typename type::value_type;}) return is_field_type<typename type::value_type>();
		else return std::is_integral_v<type> || std::is_same_v<type, std::string>;
	}
};

constexpr auto mk_data(const inner_factory&, std::string_view src) {
	return inner_factory::data_type{inner_factory::data_type::string_t{src} };
}
constexpr auto mk_data(const inner_factory&, auto&& src) {
	return inner_factory::data_type{std::forward<decltype(src)>(src) };
}

template<typename type>
constexpr auto mk_list(const inner_factory&) {
	//return std::list<type>{};
	std::vector<type> ret;
	ret.reserve(1024);
	return ret;
}

template<typename type>
constexpr auto mk_vec(const inner_factory&, auto&&... items) {
	std::vector<type> ret{};
	if constexpr(sizeof...(items) > 0)
		(void)(ret.emplace_back(std::forward<decltype(items)>(items)), ...);
	return ret;
}

constexpr auto to_field_name(const inner_factory&, auto val) {
	using namespace std::literals;
	return ""sv;
}

struct query_factory : inner_factory {
	using integer_t = int;
	using float_point_t = double;

	//NOTE: c++23 standard bug and clang implement it. the dtor will be instantiated
	//      inside class, where the sizeof is cannot to be called, and we cannot use
	//      std::unique_ptr for forward_ast (only in clang) with a constexpr dtor.
	//template<typename type> using forward_ast = std::shared_ptr<type>;
	template<typename type> using forward_ast = clang_bug_74963_wa<type>;
	template<typename type> using optional = std::optional<type>;
	using string_t = std::string;

	constexpr auto mk_fwd(auto& v) const {
		using v_type = std::decay_t<decltype(v)>;
		static_assert( !std::is_pointer_v<v_type>, "the result have to be a smart ptr like type" );
		static_assert( !std::is_reference_v<v_type>, "the result have to be a smart ptr like type" );
		v = std::make_unique<typename v_type::element_type>();
		return v.get();
	}
	constexpr auto mk_result(auto&& v) const {
		using expr_t = std::decay_t<decltype(v)>;
		return std::make_unique<expr_t>(std::move(v));
	}
};

} // namespace ast_graph_tests
