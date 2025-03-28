/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"
#include "absd.hpp"

#include <cassert>

template<typename factory>
constexpr void main_test() {
	using data_type = absd::data<factory>;
	using int_t = data_type::integer_t;

	static_assert( data_type::mk_ca([]{return 1;})() == 1 );
	static_assert( data_type::mk_ca([](int i){return i+1;})(3) == 4 );
	static_assert( data_type::mk_ca([](int a, int b){return a+b;})(3,2) == 5 );
	static_assert( data_type::mk_ca(
			[](int a, int b){return a+b;},
			data_type::mk_param("a"),
			data_type::mk_param("b", data_type{3})
	)(1) == 4);
	static_assert( []{
		int ret=2;
		data_type::mk_ca([&ret](int){++ret;})(3);
		return ret;
	}() == 3, "void lambda should to be called with () operator");
	static_assert( []{
		int ret=2;
		auto result = data_type::mk_ca([&ret](int){++ret;}, data_type::mk_param("a")).call(data_type::mk_map(0, 3));
		return (ret==3) + result.is_none();
	}() == 2 );

	static_assert( !data_type::mk([]{}).is_array() );
	static_assert( !data_type::mk([]{}).is_object() );
	static_assert( data_type::mk([]{}).is_callable() );
	static_assert( data_type::mk([]{}).call(data_type::mk_map()).is_none() );
	static_assert( (int_t)data_type::mk([]{return 1;}).call(data_type::mk_map()) == 1);
	static_assert( (int_t)data_type::mk([](int a){return a-1;}, data_type::mk_param("a")).call(data_type::mk_map(data_type{0}, data_type{3})) == 2 );

	static_assert( (int_t)data_type::mk([](int a, int b){return a-b;}, data_type::mk_param("a"), data_type::mk_param("b", data_type{1})).call(data_type::mk_map(0, 3)) == 2);

	constexpr auto amb = []{return data_type::mk([](int a, int b){return a-b;}, data_type::mk_param("a", data_type{3}), data_type::mk_param("b")); };
	static_assert( (int_t)amb().call(data_type::mk_map(0, 3, 1, 1)) == 2 );
	static_assert( (int_t)amb().call(data_type::mk_map("a", 3, "b", 1)) == 2 );
	static_assert( (int_t)amb().call(data_type::mk_map("b", 1, "a", 3)) == 2 );
	static_assert( (int_t)amb().call(data_type::mk_map("b", 3)) == 0 );
	static_assert( (int_t)amb().call(data_type::mk_map(0, 7, "b", 3)) == 4 );
	static_assert( (int_t)amb().call(data_type::mk_map("b", 3, 0, 7)) == 4 );

	struct callable_obj_arr : absd::details::constexpr_kinda_map<typename data_type::factory_t, data_type, data_type> {
		mutable data_type fake_data;
		mutable data_type data_7{7}, data_3{3}, data_5{5};
		constexpr data_type operator()() const {return data_type{88};}
		constexpr data_type operator()(int_t a, int_t b) const {return data_type{a-b};}
		constexpr data_type& at(int_t ind) const { if(ind==0) return data_3; if(ind==1) return data_5; return fake_data; }
		constexpr data_type& emplace_back(data_type) { return fake_data; }
		//constexpr int_t size() const { return 2; }
		//constexpr bool contains(const data_type&) const { return false; }
	};

	static_assert( data_type::mk(callable_obj_arr{}).is_array() );
	static_assert( data_type::mk(callable_obj_arr{}).is_object() );
	static_assert( data_type::mk(callable_obj_arr{}, data_type::mk_param("a"), data_type::mk_param("b")).is_callable() );

	struct object_with_call_method { constexpr data_type call(data_type) const { return data_type{17}; } };
	static_assert( data_type::mk(object_with_call_method{}).is_callable() );
	static_assert( data_type::mk(object_with_call_method{}).call(data_type{}) == 17 );
}

template<typename fp> struct absd_factory : tests::factory {
	using float_point_t = fp;
};

template<typename data_type>
constexpr auto mk_callable(auto&& fnc, auto&&... args) {
	return absd::details::callable2<data_type, std::decay_t<decltype(fnc)>>{
		std::forward<decltype(fnc)>(fnc),
		std::forward<decltype(args)>(args)...
	};
}
struct bad_param {int cnt;};
struct bad_param_factory : tests::factory { };
template<auto cnt> void throw_wrong_parameters_count(const bad_param_factory&) { throw bad_param{cnt}; }

void wrong_parameter_count_test() {
	using data_type = absd::data<bad_param_factory>;
	constexpr auto amb = [](const auto& mk) {return mk(
		[](int a, int b){return a-b;},
		data_type::mk_param("a", data_type{3}),
		data_type::mk_param("b"));};
	constexpr auto amb_c = [=]{
		return amb([](auto&&...a){ return mk_callable<data_type>(std::forward<decltype(a)>(a)...);});
	};
	constexpr auto amb_d = [=]{
		return amb([](auto&&...a){return data_type::mk(std::forward<decltype(a)>(a)...);});
	};
	auto ex_count = 0;

	try { (void)amb_c()(3); }
	catch(const bad_param& ex) { ++ex_count; assert( ex.cnt == 1 ); }
	assert( ex_count == 1 );

	try { (void)amb_d().call(data_type::mk_map("a", 3)); }
	catch(const bad_param& ex) { ++ex_count; assert( ex.cnt == 1 ); }
	assert( ex_count == 2 );
}

int main(int,char**) {
	main_test<absd_factory<float>>();
	main_test<absd_factory<double>>();
	wrong_parameter_count_test();
}
