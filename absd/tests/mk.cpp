/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "factory.hpp"

#include <cassert>
#include <iostream>

#ifndef __clang__
#define CTRT(code) static_assert( code );
#else
#define CTRT(code) assert( code );
#endif


static_assert( absd_data::mk(1).is_int() );
static_assert( absd_data::mk(0.1).is_float_point() );

namespace object_tests {

struct constant_object {
	absd_data m0{3}, m1{4}, empty;
	constexpr absd_data& at(absd_data key) {
		if(!key.is_int()) return empty;
		if((absd_data::integer_t)key == 0) return m0;
		if((absd_data::integer_t)key == 1) return m1;
		return empty;
	}
	constexpr bool contains(const absd_data& key) const {
		return key.is_int() && (absd_data::integer_t)key == 0 && (absd_data::integer_t)key == 1;
	}
	constexpr std::size_t size() const { return 2; }
	constexpr std::vector<absd_data> keys() const {
		return {absd_data{0}, absd_data{1}};
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
	CTRT( absd_data::mk(1) == absd_data{1} );
	CTRT( absd_data::mk(constant_object{}).is_object() );
	CTRT( absd_data::mk(constant_object{}).size() == 2 );
	CTRT( absd_data::mk(constant_object{})[absd_data{0}] == absd_data{3} );
	CTRT( absd_data::mk(constant_object{}).keys().size() == 2 );
	CTRT( absd_data::mk(constant_array{}).is_array() );
	CTRT( absd_data::mk(constant_array_and_obj{}).is_array() );
	CTRT( absd_data::mk(constant_array_and_obj{}).is_object() );
	CTRT( absd_data::mk(constant_array_and_obj{})[absd_data{"m0"}].is_int() );
	CTRT( absd_data::mk(constant_array_and_obj{})[absd_data{0}].is_int() );
}

void cannt_modify() {
	int ex = 0;
	try {
		absd_data::mk(constant_object{}).put(absd_data{1},absd_data{2});
	}
	catch(const std::exception&) {
		++ex;
	}
	assert( ex == 1 );
}

} // namespace object_tests

int main(int,char**) {
	object_tests::test<false>();
	object_tests::cannt_modify();
	return 0;
}