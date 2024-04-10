/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "absd.hpp"
#include "absd/formatter_tests.hpp"

#include <vector>
#include <memory>
#include <variant>

#include "absd/callable_tests.hpp"
using namespace std::literals;

template<typename float_point>
struct absd_factory {
	template<typename... types> using variant = std::variant<types...>;
	template<typename type> using vector = std::vector<type>;
	using float_point_t = float_point;
	using string_t = std::string;
	using empty_t = std::monostate;

	template<typename type> constexpr static auto mk_vec(){ return std::vector<type>{}; }
//	template<typename key, typename value>
//	constexpr static auto mk_map(){ return map_t<key,value>{}; }
	constexpr static auto mk_ptr(auto d) { return std::make_unique<decltype(d)>( std::move(d) ); }
	constexpr static void deallocate(auto* ptr) noexcept { delete ptr; }
	template<typename interface>
	[[noreturn]] constexpr static void throw_wrong_interface_error() {
		throw std::runtime_error("cannot perform operation "s + interface::describe_with_chars());
	}
	template<auto cnt>
	[[noreturn]] constexpr static void throw_wrong_parameters_count() {
		throw std::runtime_error("wrong arguments count: " + std::to_string(cnt));
	}
};

using absd_data1 = absd::data<absd_factory<double>>;

int main(int,char**){
	absd::tests::test_format_rt_due_gcc_bug<absd_data1>([](auto result, auto test_obj){
		if(result != test_obj) std::cout << "ERROR: " << result << "!=" << test_obj << std::endl;
	}) ;
#ifndef __clang__
	static_assert( absd_data1::test() );
	static_assert( absd::details::tests::callable2_test<absd_data1>() );
	static_assert( absd::tests::test_format<absd_data1>() );
#endif
	return !absd_data1::test() ;
}
