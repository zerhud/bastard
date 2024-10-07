/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "factory.hpp"

#ifndef __clang__
#define CTRT(code) static_assert( code );
#else
#include <cassert>
#define CTRT(code) assert( code );
#endif

template<typename factory>
constexpr void main_test() {
	using data = absd::data<factory>;
	using integer_t = data::integer_t;
	CTRT(data{}.mk_empty_array().is_array() == true )
	CTRT(data{}.mk_empty_array().is_object() == false )
	CTRT(data{(integer_t)10}.mk_empty_array().is_array() == true )
	CTRT([]{ data d; d.mk_empty_array(); d.push_back(data{(integer_t)10}); return (integer_t)d[0]; }() == 10 )
	CTRT([]{ data d; d.mk_empty_array(); d.push_back(data{(integer_t)10}); auto dd = d; return (integer_t)dd[0]; }() == 10 )
	CTRT([]{ data d; d.mk_empty_array(); d.push_back(data{(integer_t)10}); auto dd = std::move(d); return (integer_t)dd[0]; }() == 10 )
	CTRT([]{ data d; d.mk_empty_array(); d.push_back(data{(integer_t)10}); auto dd = std::move(d); return dd.size(); }() == 1 )

	CTRT( []{
		auto vec = mk_vec<data>(factory{});
		vec.emplace_back(data{(integer_t)1});
		vec.emplace_back(data{(integer_t)3});
		auto d = data::mk(std::move(vec));
		return (integer_t)d[1]; }() == 3)

	CTRT( []{
		data d;
		d.push_back(data{3});
		return (integer_t)d[data{0}];
	}() == 3 )

	CTRT( []{
		data d;
		d.push_back(data{2});
		d.push_back(data{3});
		return d.contains(data{3}) + !d.contains(data{1});
	}() == 2 );

	CTRT( ([]{
		data l,r;
		l.push_back(data{2});
		r.push_back(data{2});
		return l == r;
	}() == true) );

	CTRT( ([]{
		data l,r;
		l.push_back(data{2});
		r.mk_empty_object();
		return l == r;
	}() == false) );
}

int main(int,char**) {
	main_test<absd_factory<float>>();
	main_test<absd_factory<double>>();
	return 0;
}