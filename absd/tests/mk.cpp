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
#include <iostream>

#ifndef __clang__
#define CTRT(code) static_assert( code );
#else
#define CTRT(code) assert( code );
#endif

struct test_exception {};
struct ex_factory : tests::factory {
	template<typename>
	[[noreturn]] static void throw_wrong_interface_error() {
		throw test_exception{};
	}
};
using absd_data = absd::data<tests::factory>;


static_assert( absd_data::mk(1).is_int() );
static_assert( absd_data::mk(0.1).is_float_point() );

namespace object_tests {

template<typename data_type> struct constant_object {
	using integer_t = data_type::integer_t;
	data_type m0{3}, m1{4}, empty;
	constexpr data_type& at(data_type key) {
		if(!key.is_int()) return empty;
		if((integer_t)key == 0) return m0;
		if((integer_t)key == 1) return m1;
		return empty;
	}
	constexpr bool contains(const data_type& key) const {
		return key.is_int() && (integer_t)key == 0 && (integer_t)key == 1;
	}
	constexpr std::size_t size() const { return 2; }
	constexpr std::vector<data_type> keys() const {
		return {data_type{0}, data_type{1}};
	}
};

struct constant_array {
	absd_data m0{5}, m1{7};
	constexpr absd_data at(absd_data::integer_t ind) {
		if(ind==0) return m0;
		if(ind==1) return m1;
		return absd_data{};
	}
	constexpr bool contains(const absd_data& key) const {
		return key.is_int() && (key==absd_data{0} || key==absd_data{1});
	}
	constexpr std::size_t size() const { return 2; }
	constexpr std::vector<absd_data> keys() const {
		return {absd_data{0}, absd_data{1}};
	}
};

struct constant_array_and_obj {
	absd_data m0{5}, m1{7};
	constexpr absd_data at(absd_data key) {
		if(key.is_string() && key==absd_data{"m0"}) return m0;
		return absd_data{};
	}
	constexpr absd_data at(absd_data::integer_t ind) {
		if(ind==0) return m0;
		if(ind==1) return m1;
		return absd_data{};
	}
	constexpr bool contains(const absd_data& key) const {
		return key.is_int() && (key==absd_data{0} || key==absd_data{1});
	}
	constexpr std::size_t size() const { return 2; }
	constexpr std::vector<absd_data> keys() const {
		return {absd_data{0}, absd_data{1}};
	}
};

template<bool run_ct>
constexpr void test() {
	using obj = constant_object<absd_data>;
	static_assert( absd_data::mk(1) == absd_data{1} );
	static_assert( absd_data::mk(obj{}).is_object() );
	static_assert( absd_data::mk(obj{}).size() == 2 );
	static_assert( absd_data::mk(obj{})[absd_data{0}] == absd_data{3} );
	static_assert( absd_data::mk(obj{}).keys().size() == 2 );
	static_assert( absd_data::mk(constant_array{}).is_array() );
	static_assert( absd_data::mk(constant_array_and_obj{}).is_array() );
	static_assert( absd_data::mk(constant_array_and_obj{}).is_object() );
	CTRT( absd_data::mk(constant_array_and_obj{})[absd_data{"m0"}].is_int() );
	static_assert( absd_data::mk(constant_array_and_obj{})[absd_data{0}].is_int() );
}

void cannot_modify() {
	int ex = 0;
	using ex_data = absd::data<ex_factory>;
	using obj = constant_object<ex_data>;
	try {
		ex_data::mk(obj{}).put(ex_data{1},ex_data{2});
	}
	catch(const test_exception&) {
		++ex;
	}
	assert( ex == 1 );
}

} // namespace object_tests

int main(int,char**) {
	object_tests::test<false>();
	object_tests::cannot_modify();
	return 0;
}