/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"

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

struct factory : tests::variant_workaround_factory {
	mutable int* test_field = nullptr;
	using data_type = absd::data<factory>;
};
using data_type = factory::data_type;

struct eval_info {};

using virtbase = jiexpr_details::expression_base<factory, eval_info>;
template<typename type> using virt_item_wrapper = jiexpr_details::expression_item_wrapper<factory, eval_info, type>;

struct expr_1 : virtbase {
	constexpr expr_1() =default ;
	constexpr explicit expr_1(factory) {}
	constexpr data_type eval(const eval_info&) const override {return data_type{101};}
};
struct expr_2 : virtbase {
	constexpr expr_2() =default ;
	constexpr explicit expr_2(factory) {}
	constexpr data_type eval(const eval_info&) const override {return data_type{102};}
};

using virtvar = jiexpr_details::expression_variant<factory::virtual_variant_t<
        virtbase, virt_item_wrapper,
        expr_1, expr_2, int>>;

static_assert( []{virtvar v;return create<2>(v);}() == 0 );
static_assert( []{virtvar v;return create<int>(v);}() == 0 );
static_assert( []{
	virtvar v;
	auto& e1 = create<0>(v);
	return
	    (e1.eval(eval_info{}) == data_type{101})
	+ 2*(v.eval(eval_info{}) == data_type{101})
	;
}() == 3 );
static_assert( []{
	virtvar v;
	create<int>(v) = random_number;
	return (data_type::integer_t)v.eval(eval_info{});
}() == random_number );
static_assert( virtvar{}.eval(eval_info()) == data_type{101} );

static_assert( []{
	virtvar v;
	create<int>(v) = random_number;
	virtvar v2(std::move(v));
	create<expr_1>(v);
	return (data_type::integer_t)v2.eval(eval_info{});
}() == random_number, "can use move ctor" );

static_assert( []{
	virtvar v;
	create<int>(v) = random_number;
	virtvar v2;
	v2 = std::move(v);
	create<expr_1>(v);
	return (data_type::integer_t)v2.eval(eval_info{});
}() == random_number, "can use move operator=" );

static_assert( []{
	int val = 0;
	factory f;
	f.test_field = &val;
	virtvar v{f};
	create<int>(v) = 3;
	auto d = v.eval(eval_info{});
	val = 7;
	return *d.factory.test_field;
}() == 7, "the factory are passed to resulting data objects" );

int main(int,char**) {
}
