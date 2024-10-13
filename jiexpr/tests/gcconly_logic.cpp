/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "inner_factory.hpp"

int main(int,char**) {
	static_assert( !eval("!true") );
	static_assert( eval("!false") );
	static_assert( true == (bool)eval("!0") );
	static_assert( false == (bool)eval("!1") );
	static_assert( false == (bool)eval("!.05") );
	static_assert( false == (bool)eval("!'str'") );
	static_assert( true == (bool)eval("!''") );

	static_assert( false == (bool)eval("true and !true") );
	static_assert( true == (bool)eval("true or !true") );
	static_assert( false == (bool)eval("1+2-3 or 3- 3 ") );
	static_assert( true == (bool)eval("1 != 2") );
	static_assert( true == (bool)eval("1 < 2") );
	static_assert( false == (bool)eval("1 > 2") );
	static_assert( true == (bool)eval("3 > 2") );
	static_assert( true == (bool)eval("2 >= 2") );
	static_assert( true == (bool)eval("2 == 2") );
	static_assert( false == (bool)eval("nonexisting == 2") );
	static_assert( false == (bool)eval("2 == nonexisting") );
	static_assert( true == (bool)eval("3 >= 2") );
	static_assert( true == (bool)eval("1 <= 2") );
	static_assert( true == (bool)eval("'a' in 'bca'") );
	static_assert( false == (bool)eval("'a' in 'bcd'") );
	static_assert( true == (bool)eval("1 in [3,1,2]") );

	static_assert( true == eval("3 if false").is_none() );
	static_assert( 3 == (absd_data::integer_t)eval("3 if true") );
	static_assert( 2 == (absd_data::integer_t)eval("3 if false else 2") );

	return 0;
}
