#include "tests/factory.hpp"

#include "absd.hpp"
#include "jiexpr.hpp"
#include "jiexpr/default_operators.hpp"
#include "ascip.hpp"

struct factory : tests::factory {
	using data_type = absd::data<factory>;
};

using parser = ascip<std::tuple>;
using absd_data = absd::data<factory>;
using jiexpr_test = jiexpr<absd_data, jiexpr_details::expr_operators_simple, factory>;

constexpr absd_data eval(std::string_view src, absd_data& env) {
	jiexpr_test::operators_executer ops;
	jiexpr_test ev{&env, ops};
	auto parsed = ev.parse_str<parser>(src);
	return ev(parsed);
}

constexpr absd_data eval(std::string_view src) {
	absd_data env;
	env.mk_empty_object();
	return eval(src, env);
}

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
