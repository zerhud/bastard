#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "reflection.hpp"

namespace ast_graph::details {

template<typename type> type&& lref() ;

template<auto> struct _object_c{};
template<auto obj> constexpr const auto object_c = _object_c<obj>{};
template<typename t> struct _type_c{ using type = t; };
template<typename t> constexpr auto type_c = _type_c<t>{};

template<typename type> constexpr auto decay(const type&) { return type_c<type>; }

template<typename...> struct type_list {};
template<typename... left, typename... right>
constexpr bool operator == (type_list<left...>, type_list<right...>) { return false; }
template<typename... list>
constexpr bool operator == (type_list<list...>, type_list<list...>) { return true; }
template<typename... left, typename... right>
constexpr bool operator != (type_list<left...> l, type_list<right...> r) { return !(l==r); }
template<typename t> constexpr bool operator == (_type_c<t>, _type_c<t>) { return true; }
template<typename t, typename r> constexpr bool operator == (_type_c<t>, _type_c<r>) { return false; }
template<typename t, typename r> constexpr bool operator != (_type_c<t> ll, _type_c<r> rr) { return !(ll==rr); }
template<typename... left, typename... right>
constexpr auto operator + (type_list<left...>, type_list<right...>) { return type_list<left..., right...>{}; }

template<typename type, typename... list>
constexpr auto push_back(type_list<list...>) { return type_list<list..., type>{}; }
template<typename type, typename... list>
constexpr auto push_back(type_list<list...>, _type_c<type>) { return type_list<list..., type>{}; }
template<typename... left, typename... right>
constexpr auto push_back(type_list<left...>, type_list<right...>) { return type_list<left..., right...>{}; }
template<typename type, typename... list>
constexpr auto push_front(type_list<list...>) { return type_list<type, list...>{}; }
template<typename type, typename... list>
constexpr auto push_front(type_list<list...>, _type_c<type>) { return type_list<type, list...>{}; }

template<typename type, typename... list>
constexpr bool contains(type_list<list...>) { return ((type_c<type> == type_c<list>) + ... ); }
template<typename type, typename... list>
constexpr bool contains(_type_c<type> t, type_list<list...>) { return ((t == type_c<list>) + ...);}
template<typename type, typename... list>
constexpr auto push_front_if_not_contains(_type_c<type>, type_list<list...> l) {
	if constexpr (contains<type>(type_list<list...>{})) return l;
	else return push_front<type>(l);
}

constexpr auto transform_uniq(type_list<> l) { return l; }
template<typename first,typename... list>
constexpr auto transform_uniq(type_list<first,list...> l) {
	if constexpr(sizeof...(list)==0) return l;
	else return push_front_if_not_contains(type_c<first>, transform_uniq(type_list<list...>{}));
}

template<typename initial>
constexpr auto fold(type_list<> v, initial, auto&&) { return v; }
template<typename first, typename... list, typename initial>
constexpr auto fold(type_list<first, list...>, initial init, auto&& fnc) {
	if constexpr (sizeof...(list)==0) return fnc(init, type_c<first>);
	else return fold(type_list<list...>{}, fnc(init, type_c<first>), fnc);
}

} // namespace ast_graph::details
