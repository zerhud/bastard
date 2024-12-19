#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

namespace jinja_details {

template<typename factory>
struct trim_info {
	using p = factory::parser;
	int shift{0};
	bool trim{false};
	constexpr static auto mk_parser() {
		return -p::int_++ >> -as<true>(p::template char_<'+'>);
	}
};

} // namespace jinja_details
