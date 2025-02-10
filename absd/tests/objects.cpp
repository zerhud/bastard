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
	using absd_data = absd::data<factory>;
	using integer_t = absd_data::integer_t;
	static_assert( absd_data{}.mk_empty_object().is_object() == true );
	static_assert( absd_data{}.mk_empty_object().is_array() == false );
	static_assert( []{absd_data d{}; d.mk_empty_object(); d.put(absd_data{1}, absd_data{7}); return (integer_t)d[absd_data{1}];}() == 7 );
	static_assert( []{absd_data d{}; d.put(absd_data{1}, absd_data{7}); return (integer_t)d[absd_data{1}];}() == 7 );
	static_assert( []{ absd_data d;
		d.put(absd_data{1}, absd_data{7});
		d.put(absd_data{2}, absd_data{8});
		auto keys = d.keys();
		return (keys.size() == 2) + (2*((integer_t)keys[0] == 1)) + (4*((integer_t)keys[1] == 2)); }() == 7 );
	static_assert( ([]{
		absd::details::constexpr_kinda_map<factory,absd_data,absd_data> v;
		v.insert(std::make_pair(absd_data{1}, absd_data{2}));
		return v.contains(absd_data{1});
	}()) );
	static_assert( ((integer_t)[]{
		absd::details::constexpr_kinda_map<factory,absd_data,absd_data> v;
		v.insert(std::make_pair(absd_data{1}, absd_data{3}));
		return absd_data::mk(std::move(v))[absd_data{1}];}() == 3) );

	static_assert( []{
		absd_data d;d.mk_empty_object();
		d.put(absd_data{1}, absd_data{7});
		return d.contains(absd_data{1}) + (2*!d.contains(absd_data{7}));
	}() == 3);

	static_assert( ([]{
		absd_data l,r;
		l.mk_empty_object();
		r.mk_empty_object();
		return l == r;
	}() == true) );
	static_assert( ([]{
		absd_data l,r;
		l.put(absd_data{1}, absd_data{7});
		r.put(absd_data{1}, absd_data{7});
		return l == r;
	}() == true) );
}

template<typename fp> struct absd_factory : tests::factory {};

int main(int,char**) {
	main_test<absd_factory<float>>();
	main_test<absd_factory<double>>();
}
