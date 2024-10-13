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
