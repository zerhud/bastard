/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "concepts.hpp"
#include "reflection.hpp"
#include "node.hpp"

namespace ast_graph {

template<typename factory>
constexpr auto mk_children_types(const factory& f, vector auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, variant auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, smart_ptr auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, auto&& o)
requires( factory::template is_field_type<details::ref::decay_t<decltype(o)>>() ) {
	return details::type_list<details::ref::decay_t<decltype(o)>>{};
}

template<typename factory>
constexpr auto mk_children_types(const factory& f, auto&& o) {
	using cur_obj_t = details::ref::decay_t<decltype(o)>;
	struct node<factory, cur_obj_t> node{ f, &o };
	auto ret = push_front<cur_obj_t>(node.children_types());
	auto folded = fold(ret, details::type_list<cur_obj_t>{}, [](auto r, auto t){
		using ttype = typename decltype(t)::type;
		constexpr bool is_recursive_ptr = any_ptr_to<ttype, cur_obj_t>;
		if constexpr (t==details::type_c<cur_obj_t>) return r;
		else if constexpr(is_recursive_ptr) return r;
		else {
			using next = decltype(mk_children_types(f, details::lref<ttype>()));
			return push_back(r, next{});
		}
	});
	return transform_uniq(folded);
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, smart_ptr auto&& o) {
	using cur = details::ref::decay_t<typename details::ref::decay_t<decltype(o)>::element_type>;
	return decltype(mk_children_types(f,details::lref<cur>())){};
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, vector auto&& o) {
	using cur = details::ref::decay_t<typename details::ref::decay_t<decltype(o)>::value_type>;
	return push_front_if_not_contains(details::type_c<cur>, decltype(mk_children_types(f, details::lref<cur>())){});
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, variant auto&& o) {
	constexpr auto list = []<template<typename...>class var, typename... types>(const var<types...>&){
		return (decltype(mk_children_types(f, details::lref<details::ref::decay_t<types>>())){} + ... );
	};
	return list(o);
}

}