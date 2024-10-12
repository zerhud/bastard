#include "inner_factory.hpp"

int main(int,char**) {
	test( 2, (absd_data::integer_t)eval("2") )
	test( 2, ((absd_data::string_t)eval("'ok'")).size() )
	test( 'k', ((absd_data::string_t)eval("'ok'"))[1] )

	test( true,  (bool)eval("true") )
	test( false,  (bool)eval("false") )
	test( 2,  (absd_data::integer_t)eval("4 // 2") )
	test( 2,  (absd_data::integer_t)eval("5 // 2") )
	test( 3,  (absd_data::integer_t)eval("6 // 2") )
	test( 10,  (absd_data::integer_t)eval("5 * 2") )
	test( 2.5,  (absd_data::float_point_t)eval("5 * 0.5") )
	test( 2.5,  (absd_data::float_point_t)eval("0.5 * 5") )
	test( 2.0,  (absd_data::float_point_t)eval("5 / 2") )
	test( 2.5,  (absd_data::float_point_t)eval("5 / 2.0") )
	test( 3,  (absd_data::float_point_t)eval("5 - 2.0") )
	test( 7,  (absd_data::integer_t)eval("5 + 2") )
	test( 7,  (absd_data::integer_t)eval("1 + 2 + 4") )
	test( -1,  (absd_data::integer_t)eval("5 - 2 * 3") )
	test( 11,  (absd_data::integer_t)eval("5 + 2 * 3") )
	test( 100,  (absd_data::integer_t)eval("10 ** 2") )
	test( 30, (absd_data::integer_t)eval("5+5 ** 2") );

	test( 1, (absd_data::integer_t)eval("1") )
	test( 28,  (absd_data::integer_t)eval("(3 + 2) * 2 + 3 + 1 + 2 + 3 + 4 + 5") )
	test( false, (bool)eval("!true") )

	test( 2, ((absd_data::string_t)eval("1 ~ 1")).size() )
	test( '1', ((absd_data::string_t)eval("1 ~ 1"))[0] )
	test( '1', ((absd_data::string_t)eval("1 ~ 1"))[1] )

	return 0;
}
