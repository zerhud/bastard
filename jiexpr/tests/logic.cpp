#include "factory.hpp"

int main(int,char**) {
	test( false, (bool)eval("!true") );
	test( true, (bool)eval("!false") );
	test( true, (bool)eval("!0") );
	test( false, (bool)eval("!1") );
	test( false, (bool)eval("!.05") );
	test( false, (bool)eval("!'str'") );
	test( true, (bool)eval("!''") );

	test( false, (bool)eval("true and !true") )
	test( true, (bool)eval("true or !true") )
	test( false, (bool)eval("1+2-3 or 3- 3 ") )
	test( true, (bool)eval("1 != 2") )
	test( true, (bool)eval("1 < 2") )
	test( false, (bool)eval("1 > 2") )
	test( true, (bool)eval("3 > 2") )
	test( true, (bool)eval("2 >= 2") )
	test( true, (bool)eval("3 >= 2") )
	test( true, (bool)eval("1 <= 2") )
	test( true, (bool)eval("'a' in 'bca'") )
	test( false, (bool)eval("'a' in 'bcd'") )
	test( true, (bool)eval("1 in [3,1,2]") )

	test( true, eval("3 if false").is_none() );
	test( 3, (absd_data::integer_t)eval("3 if true") );
	test( 2, (absd_data::integer_t)eval("3 if false else 2") );

	return 0;
}
