/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/


#include "tests/factory.hpp"
#include "absd.hpp"

namespace pointers_test {

template<typename data_type>
struct empty_object {
	int inner_val = 47;

	constexpr data_type at(auto&&) const { return data_type{3}; }
	constexpr auto size() const { return 1ul; }
	constexpr bool contains(const auto& key) const { return (typename data_type::integer_t)key == 1; }
	constexpr std::vector<data_type> keys() const {
		return {data_type{1}};
	}
};

template<typename... types> struct type_list{};

struct factory : tests::factory {
	using extra_types = type_list<empty_object<absd::data<factory>>>;
};

} // namespace pointers_test

template<typename data>
void main_test() {
	using namespace std::literals;
	static_assert( []{
		typename data::string_t str;
		data d{&str};
		return d.is_string();
	}() == true );
	static_assert( []{
		typename data::string_t str = "foo";
		data d{&str};
		return (typename data::string_t)d;
	}() == "foo"sv );
	static_assert( []{
		typename data::string_t str = "foo";
		data d{&str};
		return d.size();
	}() == 3 );
	static_assert( absd::details::as_object<pointers_test::empty_object<data>, data> );
	static_assert( []{
		pointers_test::empty_object<data> obj;
		data d{&obj};
		return d.is_object() + d.is_array();
	}()  == 2 );
	static_assert( []{
		pointers_test::empty_object<data> obj;
		data d{&obj};
		return (typename data::integer_t)d[data{1}] + d.size();
	}() == 4 );
	static_assert( []{
		pointers_test::empty_object<data> obj;
		data d{&obj};
		return d.contains(data{1}) + (2*(!d.contains(data{2})));
	}() == 3 );
}

template<typename data>
void exec_op_test() {
	static_assert( []{
		pointers_test::empty_object<data> obj;
		data d{&obj};
		obj.inner_val = 53;
		return (typename data::integer_t)exec_operation(d, [](const pointers_test::empty_object<data>& v){
			return v.inner_val;
		});
	}() == 53 );
}

int main(int,char**) {
	main_test<absd::data<pointers_test::factory>>();
	exec_op_test<absd::data<pointers_test::factory>>();
	return 0;
}