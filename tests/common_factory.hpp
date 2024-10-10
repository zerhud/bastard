#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <memory>
#include <vector>

namespace tests {

struct common_factory { };

constexpr auto mk_ptr(const common_factory&, auto d) {
	return std::make_unique<decltype(d)>( std::move(d) );
}

template<typename type> constexpr auto mk_vec(const common_factory&) {
	return std::vector<type>{};
}

} // namespace tests
