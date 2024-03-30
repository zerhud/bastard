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
		/*
		auto val = (typename data_type::float_point_t)src;
		if(val<0) {
			val*=-1;
			pos = '-';
		}
		auto int_part = static_cast<typename data_type::integer_t>(val);
		back_insert_format(pos, data_type{int_part});
		val -= int_part;
		do {
			val *= 10;
			int_part = static_cast<typename data_type::integer_t>(val * 10);
		} while(val - int_part > 0) ;
		pos = '.';
		back_insert_format(pos, data_type{int_part});
		*/
	}
	else if(src.is_string()) for(auto i:(typename data_type::string_t)src) pos = i;
	else if(src.is_array()) {
		pos = '[';
		for(typename data_type::integer_t i=0;i<src.size();++i) {
			auto cur = src[i];
			if(!cur.is_string()) back_insert_format(pos, cur);
			else {
				pos = '\'';
				for(auto i:(typename data_type::string_t)cur) {
					if((i=='\'') + (i=='\\')) pos = '\\';
					pos = i;
				}
				pos = '\'';
			}

			if(i+1 < src.size()) pos = ',';
		}
		pos = ']';
	}
	else if(src.is_object()) {
		;
	}
}

} // namespace absd::details

