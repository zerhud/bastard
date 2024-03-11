//
// Created by zerhud on 10/03/24.
//

#pragma once

namespace jiexpr_details {

template<typename... types> struct test_variant {
	int ind=0;
	constexpr auto index() const { return ind; }
};

template<typename left, typename right>
consteval bool same_or_derived() {
	return std::is_same_v<left,right> || std::is_base_of_v<left, right> || std::is_base_of_v<right,left>;
}

template<typename required, typename... types>
consteval auto index_of() {
	decltype(sizeof...(types)) ind=0;
	((same_or_derived<required,types>() || (++ind,false)) || ... );
	return ind;
}

template<typename tag, template<typename...>class variant, typename... alternatives>
constexpr auto& get_by_tag(const variant<alternatives...>& var) {
	return get< index_of<tag,alternatives...>() >(var);
}

template<typename required, template<typename...>class variant, typename... alternatives>
constexpr bool variant_holds(const variant<alternatives...>& var) {
	static_assert( (... + same_or_derived<required,alternatives>()) == 1, "the variant has to contain exactly one alternative" );
	return var.index() == index_of<required,alternatives...>();
}

constexpr void details_tests() {
	struct tag {};
	struct var1 {}; struct var2 : tag {int f1=1;int f2=2;}; struct var3 {}; struct var4 {};
	using t1_var = test_variant<var1, var2, var3, var4>;
	static_assert( !variant_holds<tag>(t1_var{0}) );
	static_assert(  variant_holds<tag>(t1_var{1}) );
	static_assert( !variant_holds<tag>(t1_var{2}) );
}

} // namespace jiexpr_details
