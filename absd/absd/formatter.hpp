#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <array>
#include <charconv>

#include "../absd.hpp"

namespace absd::details {

template<auto qsym='\'', typename data_type>
constexpr void back_insert_format_req(auto&& pos, const data_type& src);

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
	else if(src.is_string()) for(auto i:(typename data_type::string_t)src) pos = i;
	else if(src.is_array()) {
		pos = '[';
		for(typename data_type::integer_t i=0;i<src.size();++i) {
			back_insert_format_req(pos, src[i]);
			if(i+1 < src.size()) pos = ',';
		}
		pos = ']';
	}
	else if(src.is_object()) {
		pos = '{';
		auto keys = src.keys();
		for(typename data_type::integer_t i=0;i<keys.size();++i) {
			back_insert_format_req(pos, keys[i]);
			pos=':';
			back_insert_format_req(pos, src[keys[i]]);
			if(i+1 < keys.size()) pos = ',';
		}
		pos = '}';
	}
	else if(src.is_none()) pos = 'n', pos = 'u', pos = 'l', pos = 'l';
}

template<auto qsym, typename data_type>
constexpr void back_insert_format_req(auto&& pos, const data_type& src) {
	if(!src.is_string()) details::back_insert_format(pos, src);
	else {
		pos = qsym;
		for(auto i:(typename data_type::string_t)src) {
			if((i==qsym) + (i=='\\')) pos = '\\';
			pos = i;
		}
		pos = qsym;
	}
}

} // namespace absd::details

namespace absd {
constexpr void back_insert_format(auto&& pos, const auto& src) {
	return details::back_insert_format(pos, src);
}
} // namespace absd
