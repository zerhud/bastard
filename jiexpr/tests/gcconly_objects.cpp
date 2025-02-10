/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "inner_factory.hpp"

int main(int,char**) {
	static_assert( true == eval("[]").is_array() );
	static_assert( true == eval("[1,2,3]").is_array() );
	static_assert( 1 == (absd_data::integer_t)(eval("[1,2,3]")[0]) );
	static_assert( true == (bool)(eval("[1,true,3]")[1]) );
	static_assert( 6 == (absd_data::integer_t)(eval("[1,2,3+3]")[2]) );
	static_assert( true == eval("{}").is_object() );
	static_assert( 0 == (eval("{}")).size() );
	static_assert( 1 == (eval("{2:3}")).size() );
	static_assert( 2 == (eval("{2:3, 4:5}")).size() );
	static_assert( 3 == (absd_data::integer_t)(eval("{2:3}")[absd_data{2}]) );
	static_assert( 5 == (absd_data::integer_t)(eval("{2:3, 4:5}")[absd_data{4}]) );
	static_assert( 3 == (absd_data::integer_t)(eval("{2:3, 4:5}")[absd_data{2}]) );

	static_assert( 7 == []{
		absd_data env;
		// empty env will just copy an empty value,
		// an object as env will copy reference to object
		env.mk_empty_object();
		eval("test = 1==1", env);
		eval("a = 2", env);
		eval("b = 1+3", env);
		return
			  (bool)env[absd_data{"test"}]
			+ (absd_data::integer_t)env[absd_data{"a"}]
			+ (absd_data::integer_t)env[absd_data{"b"}]
			;
	}() );
}
