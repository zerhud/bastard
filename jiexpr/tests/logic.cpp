#include "factory.hpp"

int main(int,char**) {
	test_rt( false, (bool)jiexpr_test::test_terms<parser>("!true") );
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("!0") );
	test_rt( false, (bool)jiexpr_test::test_terms<parser>("!1") );
	test_rt( false, (bool)jiexpr_test::test_terms<parser>("!.05") );
	test_rt( false, (bool)jiexpr_test::test_terms<parser>("!'str'") );
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("!''") );

	test( false, (bool)jiexpr_test::test_terms<parser>("true and !true") )
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("true or !true") )
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("1 != 2") )
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("1 < 2") )
	test_rt( false, (bool)jiexpr_test::test_terms<parser>("1 > 2") )
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("3 > 2") )
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("2 >= 2") )
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("3 >= 2") )
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("1 <= 2") )
	test_rt( true, (bool)jiexpr_test::test_terms<parser>("'a' in 'bca'") )
	test_rt( false, (bool)jiexpr_test::test_terms<parser>("'a' in 'bcd'") )
	test( true, (bool)jiexpr_test::test_terms<parser>("1 in [3,1,2]") )

	return 0;
}
