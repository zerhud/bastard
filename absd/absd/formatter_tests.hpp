/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of bastard.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "formatter.hpp"

#include <string>
#include <string_view>

namespace absd::tests {

constexpr std::string test_format(auto&& d) {
	std::string ret;
	details::back_insert_format(std::back_inserter(ret), std::forward<decltype(d)>(d));
	return ret;
}

template<typename data_type>
constexpr bool test_format() {
	using namespace std::literals;
	static_assert( "1"sv == test_format(data_type{1}) );
	//static_assert( "1.1"sv == test_format(data_type{1.1}) );
	static_assert( "test"sv == test_format(data_type{"test"}) );
	static_assert( "[]"sv == test_format(data_type{}.mk_empty_array()) );
	static_assert( "[1]"sv == []{
		data_type src;
		src.push_back(data_type{1});
		return test_format(src);
	}() );
	static_assert( "[1,2]"sv == []{
		data_type src;
		src.push_back(data_type{1});
		src.push_back(data_type{2});
		return test_format(src);
	}() );
	return true;
}

template<typename data_type>
constexpr void test_format_rt_due_gcc_bug(auto&& test_fnc) {
	{
		data_type src;
		src.push_back(data_type{1});
		src.push_back(data_type{"str"});
		src.push_back(data_type{typename data_type::string_t{"'t\\r"}});
		test_fnc(R"-([1,'str','\'t\\r'])-", test_format(src));
	}

	{
		test_fnc("1.2", test_format(data_type{1.2}));
		test_fnc("-1.2", test_format(data_type{-1.2}));
		test_fnc("-1", test_format(data_type{-1.0}));
	}
}

} // namespace absd::tests
