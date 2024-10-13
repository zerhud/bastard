#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <tuple>

namespace tests {

//NOTE: there is a set of bugs in clang with variant
//      https://github.com/llvm/llvm-project/issues/107593
//TODO: check and remove this class if can compile without it
template<typename... types>
struct variant_clang_bug_workaround {
	decltype(sizeof...(types)) cur{0};
	std::tuple<types...> holder;

	template<typename type>
	constexpr auto& emplace() {
		holder = std::tuple<types...>{};
		return get<type>(holder);
	}
};

} // namespace tests
