/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of bastard.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <array>
#include <charconv>

#include "../absd.hpp"

namespace absd::details {

template<typename data_type>
constexpr void back_insert_format(auto&& pos, const data_type& src) {
	std::array<char, 512> buf;
	if(src.is_int()) {
		auto res = std::to_chars(buf.data(), buf.data()+buf.size(), (typename data_type::integer_t)src);
		for(auto cur=buf.data();cur!=res.ptr;++cur) pos = *cur;
	}
	else if(src.is_float_point()) {
		auto res = std::to_chars(buf.data(), buf.data()+buf.size(), (typename data_type::float_point_t)src);
		for(auto cur=buf.data();cur!=res.ptr;++cur) pos = *cur;
	}
}

} // namespace absd::details

