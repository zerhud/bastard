#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "absd.hpp"

#include <vector>
#include <memory>
#include <variant>
#include <string>
#include <string_view>

using namespace std::literals;

template<typename float_point>
struct absd_factory {
	template<typename... types> using variant = std::variant<types...>;
	template<typename type> using vector = std::vector<type>;
	using float_point_t = float_point;
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

using absd_data = absd::data<absd_factory<double>>;

constexpr auto mk_str(const absd_data& d) {
	std::string ret;
	absd::back_insert_format(std::back_inserter(ret), d);
	return ret;
}
constexpr auto mk_state_str(const absd_data& d) {
	return
		  "none:"s + std::to_string(d.is_none())
		+ "\nint:"s + std::to_string(d.is_int())
		+ "\nbool:"s + std::to_string(d.is_bool())
		+ "\nstring:"s + std::to_string(d.is_string())
		+ "\nfloat:"s + std::to_string(d.is_float_point())
		+ "\narray:"s + std::to_string(d.is_array())
		+ "\nobject:"s + std::to_string(d.is_object())
		+ "\ncallable:"s + std::to_string(d.is_callable())
			;
}