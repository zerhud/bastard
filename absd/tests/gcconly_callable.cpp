/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"
#include "absd.hpp"

#ifndef __clang__
#define CTRT(code) static_assert( code );
#else
#include <cassert>
#define CTRT(code) assert( code );
#endif

template<typename factory>
constexpr void main_test() {
	using data_type = absd::data<factory>;
	using int_t = data_type::integer_t;

	CTRT( !data_type::mk([]{}).is_array() );
	CTRT( !data_type::mk([]{}).is_object() );
	CTRT( data_type::mk([]{}).is_callable() );
	CTRT( data_type::mk([]{}).call(data_type::mk_map()).is_none() );
	CTRT( (int_t)data_type::mk([]{return 1;}).call(data_type::mk_map()) == 1);
	CTRT( (int_t)data_type::mk([](int a){return a-1;}, data_type::mk_param("a")).call(data_type::mk_map(data_type{0}, data_type{3})) == 2 );

	CTRT( (int_t)data_type::mk([](int a, int b){return a-b;}, data_type::mk_param("a"), data_type::mk_param("b", data_type{1})).call(data_type::mk_map(0, 3)) == 2);

	constexpr auto amb = []{return data_type::mk([](int a, int b){return a-b;}, data_type::mk_param("a", data_type{3}), data_type::mk_param("b")); };
	CTRT( (int_t)amb().call(data_type::mk_map("b", 3)) == 0);
	CTRT( (int_t)amb().call(data_type::mk_map(0, 7, "b", 3)) == 4);
	CTRT( (int_t)amb().call(data_type::mk_map("b", 3, 0, 7)) == 4);

	struct callable_obj_arr : absd::details::constexpr_kinda_map<typename data_type::factory_t, data_type, data_type> {
		mutable data_type fake_data;
		mutable data_type data_7{7}, data_3{3}, data_5{5};
		constexpr data_type operator()() const {return data_type{88};}
		constexpr data_type operator()(int_t a, int_t b) const {return data_type{a-b};}
		constexpr data_type& at(int_t ind) const { if(ind==0) return data_3; if(ind==1) return data_5; return fake_data; }
		constexpr data_type& emplace_back(data_type d) { return fake_data; }
		//constexpr int_t size() const { return 2; }
		//constexpr bool contains(const data_type&) const { return false; }
	};

	CTRT( data_type::mk(callable_obj_arr{}).is_array() );
	CTRT( data_type::mk(callable_obj_arr{}).is_object() );
	CTRT( data_type::mk(callable_obj_arr{}, data_type::mk_param("a"), data_type::mk_param("b")).is_callable() );
}

template<typename fp> struct absd_factory : tests::factory {
	using float_point_t = fp;
};

int main(int,char**) {
	main_test<absd_factory<float>>();
	main_test<absd_factory<double>>();
	return 0;
}
