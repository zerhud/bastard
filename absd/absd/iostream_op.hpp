#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "formatter.hpp"
#include <iterator>

namespace absd {

template<typename ostream, typename factory>
ostream& operator<<(ostream& out, const data<factory>& d) {
	std::ostream_iterator<typename ostream::char_type> pos(out);
	back_insert_format(pos, d);
	return out;
}

}  // namespace absd
