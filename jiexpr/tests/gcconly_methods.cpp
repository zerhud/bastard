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

constexpr auto test_with_env(auto src) {
	absd_data env;
	env.put(absd_data{absd_data::string_t{"a"}}, absd_data{1});
	env.put(absd_data{absd_data::string_t{"b"}}, absd_data{2});
	env.put(absd_data{absd_data::string_t{"fnc1"}}, absd_data::mk([]{return absd_data{1};}));
	env.put(absd_data{absd_data::string_t{"fnc2"}}, absd_data::mk([](int i){return absd_data{i+1};}, absd_data::mk_param("i")));
	env.put(absd_data{absd_data::string_t{"fnc3"}}, absd_data::mk(
			[](int a, int b){return absd_data{a-b};},
			absd_data::mk_param("a", absd_data{7}),
			absd_data::mk_param("b", absd_data{5})
			));
	env.put(absd_data{absd_data::string_t{"to_4"}}, absd_data::mk(
			[](absd_data cnt){return absd_data{4};},
			absd_data::mk_param("$", absd_data{})
	));
	env.put(absd_data{absd_data::string_t{"add_filter"}}, absd_data::mk(
			[](absd_data cnt, absd_data b){
				return absd_data{(absd_data::integer_t)cnt - (absd_data::integer_t)b};
			},
			absd_data::mk_param("cnt"),
			absd_data::mk_param("b", absd_data{4})
	));
	absd_data obj, arr, obj_with_a;
	obj_with_a.put(absd_data{absd_data::string_t{"a"}}, absd_data{5});
	obj.put(absd_data{absd_data::string_t{"b"}}, absd_data{3});
	arr.push_back(absd_data{4});
	arr.push_back(obj_with_a);
	//TODO: add parameters with default args and call by positioned and named params
	arr.push_back(absd_data::mk([]{return absd_data{4};}));
	obj.put(absd_data{absd_data::string_t{"arr"}}, std::move(arr));
	env.put(absd_data{absd_data::string_t{"obj"}}, std::move(obj));
	return eval(src, env);
}

int main(int,char**) {

	static_assert( 1 == (absd_data::integer_t)test_with_env("a") );
	static_assert( 2 == (absd_data::integer_t)test_with_env("b") );
	static_assert( 3 == (absd_data::integer_t)test_with_env("obj.b") );
	static_assert( 3 == (absd_data::integer_t)test_with_env("obj['b']") );
	static_assert( 4 == (absd_data::integer_t)test_with_env("obj.arr[3-3]") );
	static_assert( 5 == (absd_data::integer_t)test_with_env("obj.arr[3-2].a") );
;
	static_assert( 1 == (absd_data::integer_t)test_with_env("fnc1()") );
	static_assert( 2 == (absd_data::integer_t)test_with_env("fnc2(1)") );
	static_assert( 2 == (absd_data::integer_t)test_with_env("fnc3()") );
	static_assert( 5 == (absd_data::integer_t)test_with_env("fnc3(a=10)") );
	static_assert( 7 == (absd_data::integer_t)test_with_env("fnc3(b=3, a=10)") );
	static_assert( 4 == (absd_data::integer_t)test_with_env("obj.arr[4-(8*1-6)]()") );

	static_assert( 8 == (absd_data::integer_t)test_with_env("1+7") );
	static_assert( 4 == (absd_data::integer_t)test_with_env("1+7|to_4") );
	static_assert( 4 == (absd_data::integer_t)test_with_env("1+7|add_filter ") );
	static_assert( -4 == (absd_data::integer_t)test_with_env("1+7|add_filter(12) ") );

	static_assert( true == (bool)test_with_env("1+7 is add_filter") );
	static_assert( false == (bool)test_with_env("1+7 is add_filter(8) ") );
	return 0;
}

