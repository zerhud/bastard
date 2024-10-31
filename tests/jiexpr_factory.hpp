#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <string>
#include <iterator>

namespace tests {

struct jiexpr_factory {
};

constexpr auto back_inserter(const jiexpr_factory&, auto& v) {
	return std::back_inserter(v);
}
constexpr auto mk_str(const jiexpr_factory&) {
	return std::string{};
}
constexpr auto mk_str(const jiexpr_factory&, auto&& arg) {
	return std::string{std::forward<decltype(arg)>(arg)};
}

} // namespace tests
