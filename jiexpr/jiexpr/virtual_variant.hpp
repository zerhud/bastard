#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

namespace jiexpr_details {

template<typename _factory, typename _solve_info>
struct expression_base {
	using factory = _factory;
	using data_type = factory::data_type;
	using solve_info = _solve_info;

	virtual ~expression_base() noexcept =default ;
	virtual data_type solve(const solve_info&) const =0 ;
};

template<typename factory, typename solve_info, typename item_type>
struct expression_item_wrapper : expression_base<factory, solve_info> {
	using base_type = expression_base<factory, solve_info>;
	using data_type = base_type::data_type;
	item_type item{};
	constexpr data_type solve(const solve_info&) const override {
		return data_type{item};
	}
};

template<template<typename...>class variant, typename base_type, typename... types>
struct virtual_variant : base_type {
	template<typename...> struct type_list {};
	template<typename t> struct type_c{using type=t;};

	using data_type = base_type::data_type;
	using solve_info = base_type::solve_info;
	using factory = base_type::factory;

	template<typename type> constexpr static auto mk_content_type() {
		if constexpr (std::is_base_of_v<base_type, type>) return type_c<type>{};
		else return type_c<expression_item_wrapper<factory, solve_info, type>>{};
	}

	template<typename... result, typename first, typename... tail>
	constexpr static auto mk_holder(type_list<first, tail...>) {
		using next = decltype(mk_content_type<first>())::type;
		if constexpr (sizeof...(tail)==0) return variant<result..., next> {};
		else return mk_holder<result..., next>(type_list<tail...>{});
	}

	using holder_type = decltype(mk_holder<>(type_list<types...>{}));

	factory f;
	base_type* pointer=nullptr;
	holder_type holder;

	template<typename type>
	friend constexpr type& create(virtual_variant& v) {
		using cur_type = decltype(mk_content_type<type>())::type;
		auto& ret = v.holder.template emplace<cur_type>();
		v.pointer = &ret;
		if constexpr (std::is_base_of_v<base_type, type>) return ret;
		else return ret.item;
	}

	template<auto type>
	friend constexpr auto& create(virtual_variant& v) {
		return create<__type_pack_element<type, types...>>(v);
	}

	constexpr virtual_variant() {
		create<__type_pack_element<0, types...>>(*this);
	}

	constexpr data_type solve(const solve_info& i) const { return pointer->solve(i); }
};

} // namespace jiexpr_details
