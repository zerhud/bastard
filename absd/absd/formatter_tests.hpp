/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of bastard.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "formatter.hpp"

#include <string>
#include <string_view>

namespace absd::tests {

constexpr std::string test_format(auto&& d) {
	std::string ret;
	details::back_insert_format(std::back_inserter(ret), std::forward<decltype(d)>(d));
	return ret;
}

template<typename data_type>
constexpr bool test_format() {
	using namespace std::literals;
	static_assert( "1"sv == test_format(data_type{1}) );
	static_assert( "1.1"sv == test_format(data_type{1.1}) );
	return true;
}

} // namespace absd::tests
