/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "inner_factory.hpp"

int main(int,char**) {
	static_assert( !!eval2("true") );
	static_assert( (absd_data::integer_t)eval2("155") == 155 );
	static_assert( !eval2("!true") );
	static_assert( eval2("!false") );
	static_assert( true == (bool)eval2("!0") );
	static_assert( false == (bool)eval2("!1") );
	static_assert( false == (bool)eval2("!.05") );
	static_assert( false == (bool)eval2("!'str'") );
	static_assert( true == (bool)eval2("!''") );

	static_assert( false == (bool)eval2("true and !true") );
	static_assert( true == (bool)eval2("true or !true") );
	static_assert( false == (bool)eval2("1+2-3 or 3- 3 ") );
	static_assert( true == (bool)eval2("1 != 2") );
	static_assert( true == (bool)eval2("1 < 2") );
	static_assert( false == (bool)eval2("1 > 2") );
	static_assert( true == (bool)eval2("3 > 2") );
	static_assert( true == (bool)eval2("2 >= 2") );
	static_assert( true == (bool)eval2("2 == 2") );
	static_assert( false == (bool)eval2("nonexisting == 2") );
	static_assert( false == (bool)eval2("2 == nonexisting") );
	static_assert( true == (bool)eval2("3 >= 2") );
	static_assert( true == (bool)eval2("1 <= 2") );
	static_assert( true == (bool)eval2("'a' in 'bca'") );
	static_assert( false == (bool)eval2("'a' in 'bcd'") );
	static_assert( true == (bool)eval2("1 in [3,1,2]") );

	static_assert( true == eval2("3 if false").is_none() );
	static_assert( 3 == (absd_data::integer_t)eval2("3 if true") );
	static_assert( 2 == (absd_data::integer_t)eval2("3 if false else 2") );

	return 0;
}
