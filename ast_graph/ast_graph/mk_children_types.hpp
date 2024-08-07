/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <tref.hpp>
#include "node.hpp"

namespace ast_graph {

template<typename factory>
constexpr auto mk_children_types(const factory& f, tref::vector auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, tref::variant auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, tref::smart_ptr auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, auto&& o)
requires( factory::template is_field_type<tref::decay_t<decltype(o)>>() ) {
	return tref::type_list<tref::decay_t<decltype(o)>>{};
}

template<typename factory>
constexpr auto mk_children_types(const factory& f, auto&& o) {
	using cur_obj_t = tref::decay_t<decltype(o)>;
	struct node<factory, cur_obj_t> node{ f, &o };
	auto ret = push_front<cur_obj_t>(node.children_types());
	auto folded = fold(ret, tref::type_list<cur_obj_t>{}, [](auto r, auto t){
		using ttype = typename decltype(t)::type;
		constexpr bool is_recursive_ptr = tref::any_ptr_to<ttype, cur_obj_t>;
		if constexpr (t==tref::type_c<cur_obj_t>) return r;
		else if constexpr(is_recursive_ptr) return r;
		else {
			using next = decltype(mk_children_types(f, tref::lref<ttype>()));
			return push_back(r, next{});
		}
	});
	return transform_uniq(folded);
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, tref::smart_ptr auto&& o) {
	using cur = tref::decay_t<typename tref::decay_t<decltype(o)>::element_type>;
	return decltype(mk_children_types(f,tref::lref<cur>())){};
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, tref::vector auto&& o) {
	using o_type = tref::decay_t<decltype(o)>;
	using cur = tref::decay_t<typename o_type::value_type>;
	return push_front<o_type>(push_front<cur>(decltype(mk_children_types(f, tref::lref<cur>())){}));
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, tref::variant auto&& o) {
	constexpr auto list = []<template<typename...>class var, typename... types>(const var<types...>&){
		return (decltype(mk_children_types(f, tref::lref<tref::decay_t<types>>())){} + ... );
	};
	return list(o);
}

}
