/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of bastard.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <iostream>

#include <utility>
#include <cassert> //TODO: remove after gcc bug will be fixed
#include "absd/callable.hpp"
#include "absd/type_erasure.hpp"
#include "absd/default_array.hpp"
#include "absd/default_object.hpp"
#include "absd/default_callable.hpp"

namespace absd {

namespace details {

template<typename factory> constexpr auto mk_integer_type() {
	if constexpr(!requires{ typename factory::integer_t; }) return int{};
	else return typename factory::integer_t{};
}
template<typename factory> constexpr auto mk_float_point_type() {
	if constexpr(!requires{ typename factory::float_pint_t; }) return double{};
	else return typename factory::float_pint_t{};
}

} // namespace details

// просто создать класс данных от фабрики, он должен содержать
//
// - вызываемый объект с двумя параметрами:
//   - список позиционных параметров и
//   - карта именованных параметров
//   - также должен возращать дефолтные параметы (получит в карте именнованых, если не указаны)
//   - дефолтный объект должен работать на подобии std::function - type erasure
// 
// - так как std::{,unordered_}map (похоже) не может быть использована за неимением constexpr
//   методов, то операции с ней нужно проверить в runtime

template<typename factory, typename crtp>
struct data {
	static_assert( noexcept( std::declval<factory>().deallocate((int*)nullptr) ), "for safity delete allocated objects the dealocate method must to be noexcept" );

	using factory_t = factory;
	using self_type = crtp;//data<factory, crtp>;
	template<typename functor> using callable2 = details::callable2<self_type, functor>;
	using base_data_type = data<factory, crtp>;
	using array_t = decltype(details::mk_array_type<crtp>(std::declval<factory>()));
	using object_t = decltype(details::mk_object_type<crtp>(std::declval<factory>()));
	using string_t = typename factory::string_t;
	using integer_t = decltype(details::mk_integer_type<factory>());
	using float_point_t = decltype(details::mk_float_point_type<factory>());

	using holder_t = typename factory::template variant<
		typename factory::empty_t, bool, integer_t, float_point_t,
		string_t, array_t*, object_t*,
		details::multiobject_tag*
	>;

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
		auto v = mk_ca_val(std::forward<decltype(_v)>(_v), std::forward<decltype(args)>(args)...);
		using val_type = std::decay_t<decltype(v)>;

		constexpr bool is_callable = details::is_specialization_of<val_type, details::callable2>;

		auto arr = details::mk_te_array<self_type>(f, counter_maker < details::origin<val_type> > {std::move(v)});
		auto arr_obj = details::mk_te_object<self_type>(f, std::move(arr));
		auto aoc = details::mk_te_callable<is_callable, self_type>(f, std::move(arr_obj));

		return mk_ptr_and_assign<details::multiobject_tag>(f, std::move(aoc));
	}

private:
	template<typename type> constexpr static const bool is_inner_counter_exists = requires(std::remove_pointer_t<std::decay_t<type>>& v){ v.increase_counter(); };

	holder_t holder;
	details::type_erasure_callable<factory_t, self_type>* multi_callable=nullptr;
	details::type_erasure_array<self_type>* multi_array=nullptr;
	details::type_erasure_object<factory_t, self_type>* multi_object=nullptr;

	template<typename to>
	constexpr static self_type mk_ptr_and_assign(const auto& f, auto&& v) {
		self_type ret;
		auto tmp = f.mk_ptr(std::forward<decltype(v)>(v));
		auto* ptr = tmp.get();
		ret.assign(std::move(tmp));
		if constexpr (requires{ret.multi_callable = ptr;}) ret.multi_callable = ptr;
		if constexpr (requires{ret.multi_array = ptr;}) ret.multi_array = ptr;
		if constexpr (requires{ret.multi_object = ptr;}) ret.multi_object = ptr;
		return ret;
	}
	constexpr static void mk_map_impl(const auto& f, self_type& result, auto&& key, auto&& val, auto&&... tail) {
		result.put(self_type{std::forward<decltype(key)>(key)}, self_type{std::forward<decltype(val)>(val)});
		if constexpr (sizeof...(tail) != 0) mk_map_impl(f, result, std::forward<decltype(tail)>(tail)...);
	}

	constexpr static auto mk_ca_val(auto&& v, auto&&... args) {
		constexpr bool np = requires{ v(); };
		constexpr bool wp = sizeof...(args) > 0;
		if constexpr (np || wp)
			return mk_ca(std::forward<decltype(v)>(v), std::forward<decltype(args)>(args)...);
		else return std::move(v);
	}

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
		constexpr counter_maker(auto&&... args) : type(std::forward<decltype(args)>(args)...) {}
		constexpr decltype(inner_counter::ref_counter) increase_counter() override { return inner_counter::increase_counter(); }
		constexpr decltype(inner_counter::ref_counter) decrease_counter() override { return inner_counter::decrease_counter(); }
	};
	template<typename from, typename type>
	constexpr static self_type mk_coutner_and_assign(const factory& f, auto&& v) {
		self_type ret;
		auto tmp = f.mk_ptr(counter_maker<type>(std::forward<decltype(v)>(v)));
		ret.assign(static_cast<from*>(tmp.get()));
		tmp.release();
		return ret;
	}

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
public:

	constexpr data() {}
	constexpr data(auto v) requires (std::is_same_v<decltype(v), bool>) : holder(v) {}

	constexpr data(string_t v) : holder(v) {} //NOTE: bug in gcc https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111284
	constexpr data(const typename string_t::value_type* v) : holder(string_t(v)) {}

	constexpr data(integer_t v) : holder(v) {}
	constexpr data(float_point_t v) : holder(v) {}
	constexpr data(self_type&& v) : holder(std::move(v.holder)) { v.holder=typename factory::empty_t{}; copy_multi_pointers(v); }
	constexpr data(const self_type& v) : holder(v.holder) { allocate(); copy_multi_pointers(v); }
	constexpr data(const data& v) : holder(v.holder) { allocate(); copy_multi_pointers(v); }
	//NOTE: bug in gcc workaround: use holder = declytpe(holder){v.holder} ?
	constexpr data(data&& v) : holder(std::move(v.holder)) { v.holder=typename factory::empty_t{}; copy_multi_pointers(v); }
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
	constexpr auto& operator=(const self_type& v) { copy_multi_pointers(v); return assign(v.holder); }
	constexpr auto& operator=(data&& v) { copy_multi_pointers(v); return assign(std::move(v.holder)); }
	constexpr auto& operator=(self_type&& v) { copy_multi_pointers(v); return assign(std::move(v.holder)); }

	constexpr auto& operator=(string_t v){ null_multi_pointers(); return assign(std::move(v)); }
	constexpr auto& operator=(integer_t v){ null_multi_pointers(); return assign(v); }
	constexpr auto& operator=(float_point_t v){ null_multi_pointers(); return assign(v); }

	constexpr explicit operator bool() const { return get<bool>(holder); }
	constexpr operator string_t() const { return get<string_t>(holder); }
	constexpr operator integer_t() const { return get<integer_t>(holder); }
	constexpr operator float_point_t() const { return get<float_point_t>(holder); }

	constexpr auto cur_type_index() const { return holder.index(); }
	constexpr bool is_int() const { return holder.index() == 2; }
	constexpr bool is_none() const { return holder.index() == 0; }
	constexpr bool is_bool() const { return holder.index() == 1; }
	constexpr bool is_string() const { return holder.index() == 4; }
	constexpr bool is_float_point() const { return holder.index() == 3; }
	constexpr bool is_array() const {
		return visit( [](const auto& v){ return is_multiptr_arr(v) || requires(std::decay_t<decltype(v)> vv){ vv->at( integer_t{} ); }; }, holder);
	}
	constexpr bool is_object() const {
		return visit( [](const auto& v){ return is_multiptr_obj(v) || requires(std::decay_t<decltype(v)> vv){ vv->at( crtp{} ); }; }, holder);
	}
	constexpr bool is_callable() const {
		return visit( [](const auto& v) { return is_multiptr_cll(v) || requires{ v->call({}); }; }, holder);
	}

	constexpr bool contains(const auto& val) const {
		return !is_none() && visit([this,&val](const auto& v){
			if(is_multiptr_obj(v)) return multi_object->contains(val);
			if constexpr(requires{v->contains(val);}) return v->contains(val);
			else if constexpr(requires{v.contains(typename std::decay_t<decltype(v)>::key_type{});}) return v.contains(val);
			else if constexpr(requires{v==val;}) return v==val;
			else {
				factory::throw_wrong_interface_error("contains");
				std::unreachable();
				return false;
			}
		}, holder);
	}
	constexpr auto size() const {
		return visit( [this](const auto& v){
			if(is_multiptr_obj(v)) return multi_object->size();
			else if(is_multiptr_arr(v)) return multi_array->size();
			if constexpr(requires{ v.size(); }) return v.size();
			else if constexpr(requires{ v->size(); }) return v->size();
			else return sizeof(v); }, holder);
	}
	constexpr auto keys() const {
		return visit( [this](const auto& v){
			if(is_multiptr_obj(v)) return multi_object->keys(factory{});
			if constexpr(requires{ v->keys(factory{}); }) return v->keys(factory{});
			else {
				factory::throw_wrong_interface_error("keys");
				std::unreachable();
				return self_type{};
			}
			}, holder);
	}

	constexpr self_type& mk_empty_array() { return assign(factory::mk_ptr(details::mk_array_type<crtp>(factory{}))); }
	constexpr self_type& mk_empty_object() { return assign(factory::mk_ptr(details::mk_object_type<crtp>(factory{}))); }
	constexpr self_type& push_back(self_type d) {
		if(is_none()) mk_empty_array();
		return visit([this,d=std::move(d)](auto& v) -> self_type& {
			if(is_multiptr_arr(v)) return multi_array->emplace_back(std::move(d));
			if constexpr(requires{ v->emplace_back(std::move(d)); }) { return v->emplace_back(std::move(d)); }
			else {
				factory::throw_wrong_interface_error("push_back");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}
	constexpr self_type& put(self_type key, self_type value) {
		if(is_none()) mk_empty_object();
		return visit([this,&key,&value](auto& v) -> self_type& {
			if(is_multiptr_obj(v)) return multi_object->put(key, value);
			if constexpr(requires{ v->put(key,value); }) return v->put(key, value);
			else {
				factory::throw_wrong_interface_error("put");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}
	constexpr const self_type& operator[](integer_t ind) const {return const_cast<base_data_type&>(*this)[ind];}
	constexpr self_type& operator[](integer_t ind){
		return visit([this,ind](auto& v)->self_type&{
			if(is_multiptr_arr(v)) return multi_array->at(ind);
			if constexpr(requires{ v->emplace_back(self_type{}); v->at(ind); }) { return v->at(ind); }
			else {
				factory::throw_wrong_interface_error("operator[ind]");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}
	constexpr const self_type& operator[](const self_type& key) const { return const_cast<base_data_type&>(*this)[std::move(key)]; }
	constexpr self_type& operator[](const self_type& key){
		return visit([this,&key](auto&v)->self_type& {
			if(is_multiptr_obj(v)) return multi_object->at(key);
			if constexpr(requires{ v->at(key); }) return v->at(key);
			else {
				factory::throw_wrong_interface_error("operator[key]");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}

	constexpr self_type cmpget_workaround(const string_t& key) const {
		return visit([this,&key](auto&v)->self_type {
			if(is_multiptr_obj(v)) return multi_object->contains(self_type{key});
			if constexpr(requires{ v->cmpget_workaround(key); }) return v->cmpget_workaround(key);
			else {
				factory::throw_wrong_interface_error("cmpget_workaround");
				std::unreachable();
				return self_type{false};
			}
		}, holder);
	}

	constexpr self_type call(auto&& params) {
		return visit([this,&params](auto& v) -> self_type {
			if(is_multiptr_cll(v)) return multi_callable->call(params);
			if constexpr(requires{ {v->call(params)}->std::same_as<self_type>; }) return v->call(params);
			else if constexpr(requires{ v->call(params); }) return (v->call(params), self_type{});
			else {
				factory::throw_wrong_interface_error("operator()");
				std::unreachable();
				return self_type{};
			}
		}, holder);
	}

	friend constexpr bool operator==(const self_type& left, const self_type& right) {
		return left.holder.index() == right.holder.index() && visit([](const auto& l, const auto& r){
			if constexpr(requires{ l==r; }) return l==r;
			else return false;
			}, left.holder, right.holder);
	}

	constexpr static bool test_simple_cases() ;
	constexpr static bool test_array_cases() ;
	constexpr static bool test_object_cases() ;

	constexpr static bool test() ;
};

} // namespace absd

#include "absd/impl_tests.ipp"
