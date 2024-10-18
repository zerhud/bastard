/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "virtual_variant.hpp"
#include "tests/variant_clang_bug_workaround.hpp"

#include <variant>

constexpr long int random_number =
		  1'000'00*(__TIME__[7])
		+ 1'000'0*(__TIME__[6])
		+ 1'000*(__TIME__[3])
		+ 1'00*(__TIME__[4])
		+ 1'0*(__TIME__[1])
		+ 1*(__TIME__[0])
;

struct factory {
	mutable int* test_field = nullptr;
#ifndef __clang__
	template<typename... types> using variant_t = std::variant<types...>;
#else
	template<typename... types> using variant_t = tests::variant_clang_bug_workaround<types...>;
#endif
};

struct info { int value{0}; };
struct base {
	using factory = struct factory;
	virtual ~base() noexcept =default ;
	virtual int solve(const info&) const =0 ;
};

struct expr1 : base {
	factory f;
	constexpr expr1() =default ;
	constexpr expr1(factory f) : f(f) {}
	constexpr int solve(const info&) const override { return 3; }
};
struct expr2 : base {
	factory f;
	constexpr expr2() =default ;
	constexpr expr2(factory f) : f(f) {}
	constexpr int solve(const info&) const override { return 5; }
};

template<typename type>
struct wrapper : base {
	factory f;
	type item{};

	constexpr wrapper() =default ;
	constexpr wrapper(factory f) : f(f) {}

	constexpr int solve(const info&) const override { return item; }
};

struct test_variant : virtual_variant<
		factory::variant_t, base, wrapper,
		expr1, expr2, int, double> {
	using base_type = virtual_variant<
			factory::variant_t, base, wrapper,
			expr1, expr2, int, double>;
	constexpr test_variant() =default ;
	constexpr test_variant(factory f) : base_type(f) {}
	constexpr int solve(const info& i) const override { return this->pointer->solve(i);}
};

static_assert( []{test_variant v; return create<expr1>(v).solve(info{});}() == 3 );
static_assert( []{test_variant v; return create<int>(v);}() == 0 );
static_assert( []{test_variant v; return create<2>(v);}() == 0 );
static_assert( test_variant{}.solve(info()) == 3 );
static_assert( []{
	test_variant v;
	auto& e1 = create<0>(v);
	return (e1.solve(info{}) == 3) + 2*(v.solve(info{}) == 3) ;
}() == 3 );
static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	return v.solve(info{});
}() == random_number );

static_assert( []{
	int val = 0;
	factory f;
	f.test_field = &val;
	test_variant v{f};
	auto& created = create<expr2>(v);
	val = 7;
	return *created.f.test_field;
}() == 7, "the factory are passed to resulting data objects" );

// move
static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	test_variant v2(std::move(v));
	create<expr1>(v);
	return v2.solve(info{});
}() == random_number, "can use move ctor" );
static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	test_variant v2;
	v2 = std::move(v);
	create<expr1>(v);
	return v2.solve(info{});
}() == random_number, "can use move operator=" );

// copy
static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	test_variant v2(v);
	create<expr1>(v);
	return v2.solve(info{});
}() == random_number, "can use copy ctor" );
static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	test_variant v2;
	v2 = v;
	create<expr1>(v);
	return v2.solve(info{});
}() == random_number, "can use copy operator=" );

static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	return get<int>(v).item + get<2>(v).item;
}() == random_number + random_number );

static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	return holds_alternative<int>(v) + holds_alternative<2>(v);
}() == 2 );

int main(int,char**) {
	return 0;
}