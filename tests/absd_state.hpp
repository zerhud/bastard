#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <string>
#include "absd.hpp"

namespace tests {

template<typename factory>
constexpr auto mk_state_str(const absd::data<factory>& d) {
	using namespace std::literals;
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

} // namespace tests
