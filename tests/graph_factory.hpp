#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <string>
#include <utility>
#include <source_location>

namespace tests {

struct graph_factory {
	using string_view = std::string_view;
	using source_location = std::source_location;

	template<typename type>
	constexpr static bool is_field_type() {
		if constexpr(requires(const type& v){static_cast<bool>(v); *v; typename type::value_type;}) return is_field_type<typename type::value_type>();
		else return std::is_integral_v<type> || std::is_same_v<type, std::string>;
	}
};

} // namespace tests
