/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"
#include "tests/variant_clang_bug_workaround.hpp"

#include "absd.hpp"
#include "jiexpr/virtual_variant.hpp"

constexpr long int random_number =
		  1'000'00*(__TIME__[7])
		+ 1'000'0*(__TIME__[6])
		+ 1'000*(__TIME__[3])
		+ 1'00*(__TIME__[4])
		+ 1'0*(__TIME__[1])
		+ 1*(__TIME__[0])
		;

struct factory : tests::factory {
	using data_type = absd::data<factory>;
#ifndef __clang__
	template<typename... types> using variant_t = std::variant<types...>;
#else
	template<typename... types> using variant_t = tests::variant_clang_bug_workaround<types...>;
#endif
};
using data_type = factory::data_type;

struct solve_info {};

using virtbase = jiexpr_details::expression_base<factory, solve_info>;

struct expr_1 : virtbase {
	constexpr data_type solve(const solve_info&) const override {return data_type{101};}
};
struct expr_2 : virtbase {
	constexpr data_type solve(const solve_info&) const override {return data_type{102};}
};

using virtvar = jiexpr_details::virtual_variant< factory::variant_t, virtbase, expr_1, expr_2, int>;

static_assert( []{virtvar v;return create<2>(v);}() == 0 );
static_assert( []{virtvar v;return create<int>(v);}() == 0 );
static_assert( []{
	virtvar v;
	auto& e1 = create<0>(v);
	return (v.pointer==&e1)
	+ 2*(v.pointer->solve(solve_info{}) == data_type{101})
	+ 4*(v.solve(solve_info{}) == data_type{101})
	;
}() == 7 );
static_assert( []{
	virtvar v;
	auto& e1 = create<int>(v);
	e1 = random_number;
	return (data_type::integer_t)v.pointer->solve(solve_info{});
}() == random_number );
static_assert( virtvar{}.solve(solve_info()) == data_type{101} );

int main(int,char**) {
	return 0;
}