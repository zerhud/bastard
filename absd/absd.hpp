/*************************************************************************
 * Copyright © 2023,2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <utility>
#include <cassert> //TODO: remove after gcc bug will be fixed
#include "absd/callable.hpp"
#include "absd/type_erasure.hpp"
#include "absd/default_array.hpp"
#include "absd/default_object.hpp"
#include "absd/default_callable.hpp"
#include "absd/exceptions.hpp"
#include "absd/formatter.hpp"

namespace absd {

namespace details {

template<typename...> struct type_list {};
template<typename t> struct _type_c{ using type=t; };
template<typename t> constexpr _type_c<t> type_c;

template<typename factory> constexpr auto mk_integer_type() {
	if constexpr(!requires{ typename factory::integer_t; }) return int{};
	else return typename factory::integer_t{};
}
template<typename factory> constexpr auto mk_float_point_type() {
	if constexpr(!requires{ typename factory::float_pint_t; }) return double{};
	else return typename factory::float_pint_t{};
}
template<typename factory, typename... types> constexpr auto mk_variant_type() {
	return type_c<typename factory::template variant<types...>>;
}
template<typename factory, typename data_type> constexpr auto mk_holder_type() {
	constexpr auto maker = []<template<typename...>class container,typename... list>(container<list...>) {
		return mk_variant_type<factory,
				typename factory::empty_t, bool,
				typename data_type::integer_t, typename data_type::float_point_t,
				typename data_type::string_t, const typename data_type::string_t*,
				multiobject_tag*,
				list*...
		>();
	};
	if constexpr (requires{maker(typename factory::extra_types{});})
		return maker(typename factory::extra_types{});
	else return maker(type_list<>{});
}

} // namespace details

template<typename factory>
struct data {
	static_assert( noexcept( std::declval<factory>().deallocate((int*)nullptr) ), "for safety delete allocated objects the deallocate method must to be noexcept" );

	using factory_t = factory;
	using self_type = data<factory>;
	template<typename functor> using callable2 = details::callable2<self_type, functor>;
	using string_t = typename factory::string_t;
	using integer_t = decltype(details::mk_integer_type<factory>());
	using float_point_t = decltype(details::mk_float_point_type<factory>());

	using holder_t = typename decltype(details::mk_holder_type<factory, self_type>())::type;

	constexpr static auto mk_param(auto&& name) {
		return name;
	}
	constexpr static auto mk_param(auto&& _name, self_type _def_val) {
		struct desc {
			std::decay_t<decltype(_name)> name;
			std::decay_t<decltype(_def_val)> def_val;
		};
		return desc{std::forward<decltype(_name)>(_name), std::move(_def_val)};
	}
	constexpr static auto mk_ca(auto&& fnc, auto&&... params) {
		return callable2(std::forward<decltype(fnc)>(fnc), std::forward<decltype(params)>(params)...);
	}
	constexpr static self_type mk_map(auto&&... args) requires (sizeof...(args) % 2 == 0) {
		self_type ret;
		if constexpr (sizeof...(args)==0) ret.mk_empty_object();
		else mk_map_impl(factory_t{}, ret, std::forward<decltype(args)>(args)...);
		return ret;
	}
	constexpr static self_type mk(auto&& v, auto&&... args) requires (!std::is_same_v<std::decay_t<decltype(v)>, factory_t>) {
		return mk(factory_t{}, std::forward<decltype(v)>(v), std::forward<decltype(args)>(args)...);
	}
	constexpr static self_type mk(const factory& f, auto&& _v, auto&&... args) {
		self_type ret;
		auto inner = inner_mk(f, std::forward<decltype(_v)>(_v), std::forward<decltype(args)>(args)...);

		constexpr bool is_counter_maker = details::is_specialization_of<decltype(inner), counter_maker>;
		static_assert( !is_counter_maker || requires{ self_type{std::move(inner.orig_val())}; }, "cannot create absd data from the object" );
		if constexpr (is_counter_maker) return self_type{std::move(inner.orig_val())};
		else mk_ptr_and_assign(ret, f, std::move(inner));

		return ret;
	}

private:
	template<typename type> constexpr static const bool is_inner_counter_exists = requires(std::remove_pointer_t<std::decay_t<type>>& v){ v.increase_counter(); };

	holder_t holder;
	details::type_erasure_callable<factory_t, self_type>* multi_callable=nullptr;
	details::type_erasure_array<self_type>* multi_array=nullptr;
	details::type_erasure_object<factory_t, self_type>* multi_object=nullptr;

	constexpr auto suppress_clang_warning() const {}
	constexpr static auto inner_mk(const factory& f, auto&& _v, auto&&... args);
	constexpr static void mk_ptr_and_assign(self_type& ret, const auto& f, auto&& v);
	constexpr static void mk_map_impl(const auto& f, self_type& result, auto&& key, auto&& val, auto&&... tail);

	constexpr static auto mk_ca_val(auto&& v, auto&&... args);

	template<auto args_sz>
	constexpr static bool mk_check_callable(const auto& v){ return requires{ v(); } || args_sz > 0; }

	constexpr void allocate() noexcept {
		visit([](auto& v){
			if constexpr(is_inner_counter_exists<decltype(v)>) {
				v->increase_counter();
			}
		}, holder);
	}
	constexpr void deallocate() {
		visit([this](auto& v){
			if constexpr(is_inner_counter_exists<decltype(v)>) deallocate(v);
			else suppress_clang_warning();
		}, holder);
	}

	constexpr void deallocate(auto* v) {
		if(v->decrease_counter()==0) {
			factory::deallocate(v);
			holder = typename factory::empty_t{};
		}
	}

	template<typename type>
	struct counter_maker : details::inner_counter, type {
		constexpr explicit counter_maker(auto&&... args) : type(std::forward<decltype(args)>(args)...) {}
		constexpr decltype(inner_counter::ref_counter) increase_counter() override { return inner_counter::increase_counter(); }
		constexpr decltype(inner_counter::ref_counter) decrease_counter() override { return inner_counter::decrease_counter(); }
	};

	constexpr void copy_multi_pointers(const auto& other) noexcept {
		multi_callable = other.multi_callable;
		multi_array = other.multi_array;
		multi_object = other.multi_object;
	}
	constexpr void null_multi_pointers() noexcept {
		multi_callable = nullptr;
		multi_array = nullptr;
		multi_object = nullptr;
	}

	constexpr static bool is_multiptr_obj(auto&& v) {
		if constexpr (std::is_same_v<std::decay_t<decltype(v)>, details::multiobject_tag*>) return v->is_obj();
		else return false;
	}
	constexpr static bool is_multiptr_arr(auto&& v) {
		if constexpr (std::is_same_v<std::decay_t<decltype(v)>, details::multiobject_tag*>) return v->is_arr();
		else return false;
	}
	constexpr static bool is_multiptr_cll(auto&& v) {
		if constexpr (std::is_same_v<std::decay_t<decltype(v)>, details::multiobject_tag*>) return v->is_cll();
		else return false;
	}

	template<typename interface, typename ret_val_t=self_type>
	constexpr static ret_val_t throw_wrong_interface_error(ret_val_t ret_val = self_type{});

	template<typename type, template<typename...>class list, typename... types>
	constexpr static bool is_listed_in_factory(list<types...>) {
		if constexpr (sizeof...(types)==0) return false;
		else return (std::is_same_v<type, types> || ... );
	}
	template<typename type>
	constexpr static bool is_listed_in_factory() {
		if constexpr( requires{ is_listed_in_factory<std::decay_t<type>>(typename factory_t::extra_types{}); })
			return is_listed_in_factory<std::decay_t<type>>(typename factory_t::extra_types{});
		else return false;
	}
public:

	constexpr data(auto* v) requires (is_listed_in_factory<std::decay_t<decltype(*v)>>()) : holder(v) {}
	constexpr data() =default ;
	constexpr explicit data(auto v) requires (
		   std::is_same_v<decltype(v), bool>
		|| std::is_same_v<decltype(v), string_t>
		|| std::is_same_v<decltype(v), integer_t>
		|| std::is_same_v<decltype(v), float_point_t>
	) : holder(v) {} //NOTE: cannot move here due bug in gcc https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111284 (bug are closed, bug clang bug is still is)

	constexpr explicit data(const string_t* str) : holder(str) {}
	constexpr explicit data(const typename string_t::value_type* v) : holder(string_t(v)) {}

	constexpr data(const data& v) : holder(v.holder) { allocate(); copy_multi_pointers(v); }
	constexpr data(data&& v) noexcept : holder(std::move(v.holder)) {
		//NOTE: clang bug with emplace: https://github.com/llvm/llvm-project/issues/57669
		v.holder.template emplace<typename factory::empty_t>();
		copy_multi_pointers(v);
	}
	constexpr ~data() { deallocate(); }

	constexpr self_type& assign() {
		deallocate();
		holder = typename factory::empty_t{};
		return static_cast<self_type&>(*this);
	}
	constexpr self_type& assign(auto&& v) {
		deallocate();
		if constexpr(requires{ holder = std::forward<decltype(v)>(v); })
			holder = std::forward<decltype(v)>(v);
		else {
			holder = v.get();
			v.release();
		}
		allocate();
		return static_cast<self_type&>(*this);
	}

	constexpr auto& operator=(const data& v) { copy_multi_pointers(v); return assign(v.holder); }
	constexpr auto& operator=(data&& v) noexcept { copy_multi_pointers(v); return assign(std::move(v.holder)); }

	constexpr auto& operator=(string_t v){ null_multi_pointers(); return assign(std::move(v)); }
	constexpr auto& operator=(integer_t v){ null_multi_pointers(); return assign(v); }
	constexpr auto& operator=(float_point_t v){ null_multi_pointers(); return assign(v); }

	constexpr explicit operator bool() const { return get<bool>(holder); }
	constexpr operator string_t() const { return holder.index() == 5 ? *get<const string_t*>(holder) : get<string_t>(holder); }
	constexpr operator integer_t() const { return get<integer_t>(holder); }
	constexpr operator float_point_t() const { return get<float_point_t>(holder); }

	[[nodiscard]] constexpr bool is_int() const { return holder.index() == 2; }
	[[nodiscard]] constexpr bool is_none() const { return holder.index() == 0; }
	[[nodiscard]] constexpr bool is_bool() const { return holder.index() == 1; }
	[[nodiscard]] constexpr bool is_string() const { return (holder.index() == 4) + (holder.index() == 5); }
	[[nodiscard]] constexpr bool is_float_point() const { return holder.index() == 3; }
	[[nodiscard]] constexpr bool is_array() const {
		return visit( [](const auto& v){ return is_multiptr_arr(v) || details::as_array<std::remove_pointer_t<std::decay_t<decltype(v)>>, self_type>; }, holder);
	}
	[[nodiscard]] constexpr bool is_object() const {
		return visit( [](const auto& v){ return is_multiptr_obj(v) || details::as_object<std::remove_pointer_t<std::decay_t<decltype(v)>>, self_type>; }, holder);
	}
	[[nodiscard]] constexpr bool is_callable() const {
		return visit( [](const auto& v) { return is_multiptr_cll(v) || requires{ v->call({}); }; }, holder);
	}

	[[nodiscard]] constexpr bool contains(const auto& val) const;
	[[nodiscard]] constexpr auto size() const;
	[[nodiscard]] constexpr auto keys() const;

	constexpr self_type& mk_empty_array() { mk_ptr_and_assign(*this, factory{}, inner_mk(factory{}, factory{}.template mk_vec<self_type>())); return *this; }
	constexpr self_type& mk_empty_object() { mk_ptr_and_assign(*this, factory{}, inner_mk(factory{}, details::mk_map_type<self_type, self_type>(factory{}))); return *this; }
	constexpr self_type& push_back(self_type d);
	constexpr self_type& put(self_type key, self_type value);
	[[nodiscard]] constexpr const self_type operator[](integer_t ind) const {return const_cast<self_type&>(*this)[ind];}
	[[nodiscard]] constexpr self_type operator[](integer_t ind);
	[[nodiscard]] constexpr const self_type operator[](const self_type& key) const { return const_cast<self_type&>(*this)[std::move(key)]; }
	[[nodiscard]] constexpr self_type operator[](const self_type& key);

	[[nodiscard]] constexpr self_type call(auto&& params);

	[[nodiscard]] friend constexpr auto exec_operation(const self_type& left, const self_type& right, auto&& op) {
		return visit([&op](const auto& l, const auto& r) -> self_type {
			if constexpr (requires{op(l,r);}) return self_type{op(l,r)};
			else {
				throw_wrong_interface_error<details::interfaces::exec_op>();
				return self_type{};
			}
		}, left.holder, right.holder);
	}

	[[nodiscard]] friend constexpr bool operator==(const self_type& left, const self_type& right) {
		return left.holder.index() == right.holder.index() && visit([](const auto& l, const auto& r){
			if constexpr(requires{ l->is_eq(*r); }) return l->is_eq(*r);
			else if constexpr(requires{ l==r; }) return l==r;
			else return false;
			}, left.holder, right.holder);
	}
};

} // namespace absd

#include "absd/impl.ipp"
