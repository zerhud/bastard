#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <string>
#include <variant>
#include <stdexcept>

#include "common_factory.hpp"

namespace tests {

struct factory : common_factory {
	using empty_t = std::monostate;
	using string_t = std::string;
	template<typename... types> using variant = std::variant<types...>;

	constexpr static void deallocate(auto* ptr) noexcept { delete ptr; }

	template<typename interface>
	[[noreturn]] constexpr static void throw_wrong_interface_error() {
		using namespace std::literals;
		throw std::runtime_error("cannot perform operation "s + interface::describe_with_chars());
	}
};

} // namespace tests
