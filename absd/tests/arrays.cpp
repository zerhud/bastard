/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"
#include "absd.hpp"

template<typename factory>
constexpr void main_test() {
	using data = absd::data<factory>;
	using integer_t = data::integer_t;

	static_assert( data{}.mk_empty_array().is_array() == true );
	static_assert( data{}.mk_empty_array().is_object() == false );
	static_assert( data{(integer_t)10}.mk_empty_array().is_array() == true );

	constexpr auto push_back_10 = []{
		data d;
		d.push_back(data{(integer_t)10});
		return d;
	};
	static_assert( push_back_10() == push_back_10(), "can compare" );
	static_assert( (integer_t)(push_back_10()[0]) == 10 );
	static_assert( (integer_t)(push_back_10()[data{0}]) == 10 );
	static_assert( [=]{ data d = push_back_10(); return (integer_t)d[0]; }() == 10 );
	static_assert( [=]{ data d = push_back_10(); auto dd = d; return (integer_t)dd[0]; }() == 10 );
	static_assert( [=]{ data d = push_back_10(); auto dd = std::move(d); return (integer_t)dd[0]; }() == 10 );
	static_assert( [=]{ data d = push_back_10(); auto dd = std::move(d); return dd.size(); }() == 1 );

	static_assert( []{
		auto vec = mk_vec<data>(factory{});
		vec.emplace_back(data{(integer_t)1});
		vec.emplace_back(data{(integer_t)3});
		auto d = data::mk(std::move(vec));
		return (integer_t)d[1]; }() == 3);

	static_assert( []{
		data d;
		d.push_back(data{2});
		d.push_back(data{3});
		return d.contains(data{3}) + !d.contains(data{1});
	}() == 2 );

	static_assert( ([]{
		data l,r;
		l.push_back(data{2});
		r.push_back(data{2});
		return l == r;
	}() == true) );

	static_assert( ([]{
		data l,r;
		l.push_back(data{2});
		r.mk_empty_object();
		return l == r;
	}() == false) );
}

template<typename fp> struct absd_factory : tests::factory {
	using float_point_t = fp;
};

int main(int,char**) {
	main_test<absd_factory<float>>();
	main_test<absd_factory<double>>();
	return 0;
}