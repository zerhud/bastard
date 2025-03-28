/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "heap_variant.hpp"

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
	int* test_field=nullptr;
	template<typename... types> using variant_t = std::variant<types...>;
};
constexpr void deallocate(const factory&, auto* ptr) { delete ptr; }
template<typename type> constexpr auto* allocate(const factory&, auto&&... args) {
	return new type{std::forward<decltype(args)>(args)...};
}

struct info { int value{0}; };
struct base {
	using factory = struct factory;
	virtual ~base() noexcept =default ;
	virtual int eval(const info&) const =0 ;
};

struct expr1_tag {};
struct expr1 : base, expr1_tag {
	factory f;
	constexpr expr1() =default ;
	constexpr expr1(factory f) : f(f) {}
	constexpr int eval(const info&) const override { return 3; }
};
struct expr2 : base {
	factory f;
	constexpr expr2() =default ;
	constexpr expr2(factory f) : f(f) {}
	constexpr int eval(const info&) const override { return 5; }
};

template<typename type>
struct wrapper : base {
	factory f;
	type item{};

	constexpr wrapper() =default ;
	constexpr wrapper(factory f) : f(f) {}

	constexpr int eval(const info&) const override { return item; }
};

struct test_variant : heap_variant<
		base, wrapper,
		expr1, expr2, int, double> {
	using base_type = heap_variant<
			base, wrapper,
			expr1, expr2, int, double>;
	constexpr test_variant() =default ;
	constexpr test_variant(factory f) : base_type(std::move(f)) {}
	constexpr int eval(const info& i) const override { return this->pointer->eval(i);}
};

static_assert( []{test_variant v; return create<expr1>(v).eval(info{});}() == 3 );
static_assert( []{test_variant v; return create<int>(v);}() == 0 );
static_assert( []{test_variant v; return create<2>(v);}() == 0 );
static_assert( test_variant{}.eval(info()) == 3 );
static_assert( []{
	test_variant v;
	auto& e1 = create<0>(v);
	return (e1.eval(info{}) == 3) + 2*(v.eval(info{}) == 3) ;
}() == 3 );
static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	return v.eval(info{});
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

static_assert( first_index_of<expr1_tag>(test_variant{}) == 0 );
static_assert( first_index_of<int>(test_variant{}) == 2 );

// move
static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	test_variant v2(std::move(v));
	return v2.eval(info{});
}() == random_number, "can use move ctor" );
static_assert( []{
	test_variant v;
	create<int>(v) = random_number;
	test_variant v2;
	v2 = std::move(v);
	return v2.eval(info{});
}() == random_number, "can use move operator=" );

int main(int,char**) {
}
