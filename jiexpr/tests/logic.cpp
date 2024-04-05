#include "factory.hpp"

int main(int,char**) {
	test_rt( false, (bool)eval("!true") );
	test_rt( true, (bool)eval("!0") );
	test_rt( false, (bool)eval("!1") );
	test_rt( false, (bool)eval("!.05") );
	test_rt( false, (bool)eval("!'str'") );
	test_rt( true, (bool)eval("!''") );

	test( false, (bool)eval("true and !true") )
	test_rt( true, (bool)eval("true or !true") )
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

	return 0;
}
