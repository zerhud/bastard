/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "inner_factory.hpp"

int main(int,char**) {
	static_assert( 2 == (absd_data::integer_t)eval("2") );
	static_assert( 2 == ((absd_data::string_t)eval("'ok'")).size() );
	static_assert( 'k' == ((absd_data::string_t)eval("'ok'"))[1] );

	static_assert( true ==  (bool)eval("true") );
	static_assert( false ==  (bool)eval("false") );
	static_assert( 2 ==  (absd_data::integer_t)eval("4 // 2") );
	static_assert( 2 ==  (absd_data::integer_t)eval("5 // 2") );
	static_assert( 3 ==  (absd_data::integer_t)eval("6 // 2") );
	static_assert( 10 ==  (absd_data::integer_t)eval("5 * 2") );
	static_assert( 2.5 ==  (absd_data::float_point_t)eval("5 * 0.5") );
	static_assert( 2.5 ==  (absd_data::float_point_t)eval("0.5 * 5") );
	static_assert( 2.0 ==  (absd_data::float_point_t)eval("5 / 2") );
	static_assert( 2.5 ==  (absd_data::float_point_t)eval("5 / 2.0") );
	static_assert( 3 ==  (absd_data::float_point_t)eval("5 - 2.0") );
	static_assert( 7 ==  (absd_data::integer_t)eval("5 + 2") );
	static_assert( 7 ==  (absd_data::integer_t)eval("1 + 2 + 4") );
	static_assert( -1 ==  (absd_data::integer_t)eval("5 - 2 * 3") );
	static_assert( 11 ==  (absd_data::integer_t)eval("5 + 2 * 3") );
	static_assert( 100 ==  (absd_data::integer_t)eval("10 ** 2") );
	static_assert( 30 == (absd_data::integer_t)eval("5+5 ** 2") );

	static_assert( 1 == (absd_data::integer_t)eval("1") );
	static_assert( 28 ==  (absd_data::integer_t)eval("(3 + 2) * 2 + 3 + 1 + 2 + 3 + 4 + 5") );
	static_assert( false == (bool)eval("!true") );

	static_assert( 2 == ((absd_data::string_t)eval("1 ~ 1")).size() );
	static_assert( '1' == ((absd_data::string_t)eval("1 ~ 1"))[0] );
	static_assert( '1' == ((absd_data::string_t)eval("1 ~ 1"))[1] );

	return 0;
}
