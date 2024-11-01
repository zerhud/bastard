#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

template<template<typename...>class variant, typename base_type, template<typename>class item_wrapper, typename... types>
struct heap_variant : base_type {
	using size_t = decltype(sizeof...(types));
	using factory = base_type::factory;
	using self_type = heap_variant<variant, base_type, item_wrapper, types...>;

	template<typename...> struct type_list {};
	template<typename t> struct type_c{using type=t;};

	template<typename type> constexpr static auto mk_content_type() {
		if constexpr (std::is_base_of_v<base_type, type>) return type_c<type>{};
		else {
			using result_type = item_wrapper<type>;
			static_assert( std::is_base_of_v<base_type, result_type>, "the item_wrapper should to be derived from the base_type" );
			return type_c<result_type>{};
		}
	}

	template<typename... result, typename first, typename... tail>
	constexpr static auto mk_holder(type_list<first, tail...>) {
		using next = decltype(mk_content_type<first>())::type*;
		if constexpr (sizeof...(tail)==0) return variant<result..., next> {};
		else return mk_holder<result..., next>(type_list<tail...>{});
	}

	template<typename type> using content_type = decltype(mk_content_type<type>())::type;

	constexpr heap_variant() : heap_variant(factory{}) {}
	constexpr heap_variant(factory f) : f(std::move(f)) {
		create<0>(*this);
	}
	constexpr ~heap_variant() noexcept {
		dispose();
	}

	// copy we don't provide a copy: it should work as shared_ptr or intrusive_ptr
	constexpr heap_variant(const heap_variant&) =delete ;
	constexpr heap_variant& operator=(const heap_variant&) =delete ;

	constexpr heap_variant(heap_variant&& other) noexcept
	: pointer(other.pointer)
	, pointer_variant(other.pointer_variant) {
		other.pointer = nullptr;
	}
	constexpr heap_variant& operator=(heap_variant&& other) noexcept {
		dispose();
		pointer = other.pointer;
		other.pointer = nullptr;
		pointer_variant = other.pointer_variant;
		return *this;
	}

	template<typename type>
	friend constexpr auto& create(heap_variant& v) {
		static_assert( (std::is_same_v<type, types> + ...) > 0, "the type should to be one of present types" );
		v.dispose();
		auto* ptr = allocate<content_type<type>>(v.f, v.f);
		v.pointer = ptr;
		v.shift_index<type>();
		v.pointer_variant = ptr;
		return *ptr;
		//if constexpr (std::is_base_of_v<base_type, type>) return *ptr;
		//else return ptr->item;
	}
	template<auto ind>
	friend constexpr auto& create(heap_variant& v) {
		return create<__type_pack_element<ind, types...>>(v);
	}

	template<typename type, typename cur_self_type>
	friend constexpr auto&& get(cur_self_type&& v) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		//TODO: dynamic_cast should to be used, and no variant of pointers field required,
		//      but there is a gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107744
		static_assert( (std::is_same_v<type, types> + ...) > 0, "the type should to be one of present types" );
		return *get<content_type<type>*>(v.pointer_variant); // use dynamic_cast<inner_type&>(*v.pointer); (gcc crashes)
	}
	template<auto ind, typename cur_self_type>
	friend constexpr auto&& get(cur_self_type&& v) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		return get<__type_pack_element<ind, types...>>(std::forward<decltype(v)>(v));
	}

	template<typename type, typename cur_self_type>
	friend constexpr bool holds_alternative(cur_self_type&& v) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		return v.ind == index_of<type>();
	}
	template<auto ind, typename cur_self_type>
	friend constexpr bool holds_alternative(cur_self_type&& v) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		return ind == v.ind;
	}

	template<typename type, typename cur_self_type>
	friend constexpr auto first_index_of(cur_self_type&&) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		static_assert( ((std::is_base_of_v<type, types>||std::is_same_v<type,types>) + ... ) > 0, "there is no such type" );
		size_t ind=0;
		(void)( ((std::is_base_of_v<type, types>||std::is_same_v<type,types>)||(++ind,false)) || ... );
		return ind;
	}

	factory f;
protected:
	using holder_type = decltype(mk_holder<>(type_list<types...>{}));

	template<typename type> constexpr static size_t index_of() {
		size_t i=0;
		(void)((std::is_same_v<type,types>||(++i,0))||...);
		return i;
	}
	template<typename type> constexpr void shift_index() {
		ind = index_of<type>();
	}
	constexpr auto dispose() {
		//deallocate(f, pointer);
		//visit([this](auto*p){deallocate(f, p);}, pointer_variant);
		visit([this](auto*p){delete p;}, pointer_variant);
	}
	size_t ind{0};
	base_type* pointer{nullptr};
	holder_type pointer_variant;
};