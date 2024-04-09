#include "factory.hpp"

int main(int,char**) {
	test_rt( false, (bool)eval("!true") );
	test_rt( true, (bool)eval("!false") );
	test_rt( true, (bool)eval("!0") );
	test_rt( false, (bool)eval("!1") );
	test_rt( false, (bool)eval("!.05") );
	test_rt( false, (bool)eval("!'str'") );
	test_rt( true, (bool)eval("!''") );

	test( false, (bool)eval("true and !true") )
	test_rt( true, (bool)eval("true or !true") )
	test( false, (bool)eval("1+2-3 or 3- 3 ") )
	test_rt( true, (bool)eval("1 != 2") )
	test_rt( true, (bool)eval("1 < 2") )
	test_rt( false, (bool)eval("1 > 2") )
	test_rt( true, (bool)eval("3 > 2") )
	test_rt( true, (bool)eval("2 >= 2") )
	test_rt( true, (bool)eval("3 >= 2") )
	test_rt( true, (bool)eval("1 <= 2") )
	test_rt( true, (bool)eval("'a' in 'bca'") )
	test_rt( false, (bool)eval("'a' in 'bcd'") )
	test( true, (bool)eval("1 in [3,1,2]") )

	test_rt( true, eval("3 if false").is_none() );
	test_rt( 3, (absd_data::integer_t)eval("3 if true") );
	test_rt( 2, (absd_data::integer_t)eval("3 if false else 2") );

	return 0;
}