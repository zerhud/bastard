#include "factory.hpp"

int main(int,char**) {
	test_rt( 1, (absd_data::integer_t)jiexpr_test::test_terms<parser>("1") )
	test_rt( 2, (absd_data::integer_t)jiexpr_test::test_terms<parser>("2") )
	test_rt( 2, ((absd_data::string_t)jiexpr_test::test_terms<parser>("'ok'")).size() )
	test_rt( 'k', ((absd_data::string_t)jiexpr_test::test_terms<parser>("'ok'"))[1] )

	test_rt( true,  (bool)jiexpr_test::test_terms<parser>("true") )
	test_rt( false,  (bool)jiexpr_test::test_terms<parser>("false") )
	test_rt( 2,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("4 // 2") )
	test_rt( 2,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("5 // 2") )
	test_rt( 3,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("6 // 2") )
	test_rt( 10,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("5 * 2") )
	test_rt( 2.5,  (absd_data::float_point_t)jiexpr_test::test_terms<parser>("5 * 0.5") )
	test_rt( 2.5,  (absd_data::float_point_t)jiexpr_test::test_terms<parser>("0.5 * 5") )
	test_rt( 2.0,  (absd_data::float_point_t)jiexpr_test::test_terms<parser>("5 / 2") )
	test_rt( 2.5,  (absd_data::float_point_t)jiexpr_test::test_terms<parser>("5 / 2.0") )
	test_rt( 3,  (absd_data::float_point_t)jiexpr_test::test_terms<parser>("5 - 2.0") )
	test_rt( 7,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("5 + 2") )
	test_rt( 7,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("1 + 2 + 4") )
	test_rt( -1,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("5 - 2 * 3") )
	test_rt( 11,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("5 + 2 * 3") )
	test_rt( 100,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("10 ** 2") )
	test_rt( 30, (absd_data::integer_t)jiexpr_test::test_terms<parser>("5+5 ** 2") );


	test( 28,  (absd_data::integer_t)jiexpr_test::test_terms<parser>("(3 + 2) * 2 + 3 + 1 + 2 + 3 + 4 + 5") )
	test( false, (bool)jiexpr_test::test_terms<parser>("!true") );

	test( 2, ((absd_data::string_t)jiexpr_test::test_terms<parser>("1 ~ 1")).size() );
	test_rt( '1', ((absd_data::string_t)jiexpr_test::test_terms<parser>("1 ~ 1"))[0] );
	test_rt( '1', ((absd_data::string_t)jiexpr_test::test_terms<parser>("1 ~ 1"))[1] );

	return 0;
}
