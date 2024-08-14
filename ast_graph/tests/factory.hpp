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

namespace detail
{
template<uint8_t... digits> struct positive_to_chars { static const char value[]; };
template<uint8_t... digits> const char positive_to_chars<digits...>::value[] = {('0' + digits)..., 0};

template<uint8_t... digits> struct negative_to_chars { static const char value[]; };
template<uint8_t... digits> const char negative_to_chars<digits...>::value[] = {'-', ('0' + digits)..., 0};

template<bool neg, uint8_t... digits>
struct to_chars : positive_to_chars<digits...> {};

template<uint8_t... digits>
struct to_chars<true, digits...> : negative_to_chars<digits...> {};

template<bool neg, uintmax_t rem, uint8_t... digits>
struct explode : explode<neg, rem / 10, rem % 10, digits...> {};

template<bool neg, uint8_t... digits>
struct explode<neg, 0, digits...> : to_chars<neg, digits...> {};

template<typename T>
constexpr uintmax_t cabs(T num) { return (num < 0) ? -num : num; }
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

struct factory {
	using string_view = std::string_view;
	using source_location = std::source_location;
	using data_type = absd::data<absd_factory>;

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

constexpr auto mk_data(const factory&, std::string_view src) {
	return factory::data_type{ factory::data_type::string_t{src} };
}
constexpr auto mk_data(const factory&, auto&& src) {
	return factory::data_type{ std::forward<decltype(src)>(src) };
}

template<typename type>
constexpr auto mk_list(const factory&) {
	//return std::list<type>{};
	std::vector<type> ret;
	ret.reserve(1024);
	return ret;
}

template<typename type>
constexpr auto mk_vec(const factory&) {
	return std::vector<type>{};
}

constexpr auto to_field_name(const factory&, auto val) {
	using namespace std::literals;
	return ""sv;
}

struct query_factory : factory {
	using field_name_type = std::string_view;
	using integer_type = int;
	using float_point_type = double;
	using data_type = int;

	//template<typename type> using forward_ast = clang_bug_74963_wa<type>;
	template<typename type> using forward_ast = std::unique_ptr<type>;
	template<typename type> using optional = std::optional<type>;
	using string_t = std::string;
	using string_type = std::string;

	constexpr auto mk_fwd(auto& v) const {
		using v_type = std::decay_t<decltype(v)>;
		static_assert( !std::is_pointer_v<v_type>, "the result have to be a unique_ptr like type" );
		static_assert( !std::is_reference_v<v_type>, "the result have to be a unique_ptr like type" );
		v = std::make_unique<typename v_type::element_type>();
		//v = clang_bug_74963_wa<v_type>(new typename v_type::element_type(std::forward<decltype(v)>(v)));
		return v.get();
	}
	constexpr auto mk_result(auto&& v) const {
		using expr_t = std::decay_t<decltype(v)>;
		return std::make_unique<expr_t>(std::move(v));
		//return clang_bug_74963_wa<expr_t>(new expr_t(std::forward<decltype(v)>(v)));
	}
};

} // namespace ast_graph_tests
