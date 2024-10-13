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

	return 0;
}
