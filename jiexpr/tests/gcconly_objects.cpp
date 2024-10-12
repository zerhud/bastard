#include "inner_factory.hpp"

int main(int,char**) {
	test( true, eval("[]").is_array() )
	test( true, eval("[1,2,3]").is_array() )
	test( 1, (absd_data::integer_t)(eval("[1,2,3]")[0]) )
	test( true, (bool)(eval("[1,true,3]")[1]) )
	test( 6, (absd_data::integer_t)(eval("[1,2,3+3]")[2]) )
	test( true, eval("{}").is_object() )
	test( 0, (eval("{}")).size() )
	test( 1, (eval("{2:3}")).size() )
	test( 2, (eval("{2:3, 4:5}")).size() )
	test( 3, (absd_data::integer_t)(eval("{2:3}")[absd_data{2}]) )
	test( 5, (absd_data::integer_t)(eval("{2:3, 4:5}")[absd_data{4}]) )
	test( 3, (absd_data::integer_t)(eval("{2:3, 4:5}")[absd_data{2}]) )

	test( 7, []{
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
