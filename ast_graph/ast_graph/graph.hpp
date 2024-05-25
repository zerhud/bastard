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

namespace details {

template<template<typename...>typename list, typename... types>
constexpr auto mk_variant(const auto& f, const list<types...>&) {
	return f.template mk_variant<const ref::decay_t<types>*...>();
}

template<typename factory>
constexpr auto mk_children_types(const factory& f, vector auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, variant auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, smart_ptr auto&& o) ;
template<typename factory>
constexpr auto mk_children_types(const factory& f, auto&& o)
requires( factory::template is_field_type<ref::decay_t<decltype(o)>>() ) {
	return details::type_list<ref::decay_t<decltype(o)>>{};
}

template<typename factory>
constexpr auto mk_children_types(const factory& f, auto&& o) {
	using cur_obj_t = ref::decay_t<decltype(o)>;
	struct node<factory, cur_obj_t> node{ f, &o };
	auto ret = push_front<cur_obj_t>(node.children_types());
	auto folded = fold(ret, details::type_list<cur_obj_t>{}, [](auto r, auto t){
		using ttype = typename decltype(t)::type;
		constexpr bool is_recursive_ptr = any_ptr_to<ttype, cur_obj_t>;
		if constexpr (t==type_c<cur_obj_t>) return r;
		else if constexpr(is_recursive_ptr) return r;
		else {
			using next = decltype(mk_children_types(f, lref<ttype>()));
			return push_back(r, next{});
		}
	});
	return transform_uniq(folded);
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, smart_ptr auto&& o) {
	using cur = ref::decay_t<typename ref::decay_t<decltype(o)>::element_type>;
	return decltype(mk_children_types(f,lref<cur>())){};
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, vector auto&& o) {
	using cur = ref::decay_t<typename ref::decay_t<decltype(o)>::value_type>;
	return push_front_if_not_contains(details::type_c<cur>, decltype(mk_children_types(f, lref<cur>())){});
}
template<typename factory>
constexpr auto mk_children_types(const factory& f, variant auto&& o) {
	constexpr auto list = []<template<typename...>class var, typename... types>(const var<types...>&){
		return (decltype(mk_children_types(f, lref<ref::decay_t<types>>())){} + ... );
	};
	return list(o);
}
} // namespace details

struct no_value{};
template<typename factory, typename graph>
struct edge {
	using name_type = decltype(factory{}.mk_edge_name());
	using value_type = decltype(factory{}.alloc_smart(details::lref<graph>()));

	name_type name;
	value_type value;
};

template<typename factory, typename origin>
struct graph {
	using root_type = decltype(
			details::mk_variant(
					factory{},
					push_front<no_value>(
							details::mk_children_types(
									factory{},
									details::lref<origin>()
							)
					)
			)
		);
	using child_type = edge<factory, graph<factory, origin>>;
	using children_type = decltype(factory{}.template mk_vec<child_type>());

	root_type root;
	children_type children;
};

template<typename factory, typename origin>
constexpr void exec_for_ptr(const graph<factory, origin>& g, auto&& fnc) {
	visit([&fnc]<typename t>(const t* v){
		if constexpr (!details::is_same<t,no_value>()) fnc(v);
	}, g.root);
}

template<typename factory, typename origin>
constexpr auto fields_count(const graph<factory, origin>& g) {
	return visit( []<typename t>(const t* v) {
		if constexpr (!details::is_same<t,no_value>()) return node<factory, t>{ {}, v }.fields_count();
		else {
			factory::throw_no_value_access();
			return 0; // for type deduction
		}
	}, g.root );
}

template<typename origin, typename factory, vector cur_node_type>
constexpr auto mk_graph(const factory& f, const cur_node_type& top_level) ;
template<typename origin, typename factory, variant cur_node_type>
constexpr auto mk_graph(const factory& f, const cur_node_type& top_level) ;
template<typename origin, typename factory, any_ptr cur_node_type>
constexpr auto mk_graph(const factory& f, const cur_node_type& top_level) ;
template<typename origin, typename factory, typename cur_node_type>
constexpr graph<factory, origin> mk_graph(const factory& f, const cur_node_type& top_level) {
	using graph_type = graph<factory, origin>;
	graph_type g{ &top_level, {} };
	struct node<factory, cur_node_type> node{f, &top_level};
	g.children.reserve(node.children_count()); // clang constexpr compilation workaround
	node.for_each_child_value([&f,&g](auto&& n, const auto& v){
		g.children.emplace_back().name = static_cast<decltype(n)&&>(n);
		g.children.back().value = f.alloc_smart(mk_graph<origin>(f, v));
	});
	return g;
}
template<typename origin, typename factory, vector cur_node_type>
constexpr auto mk_graph(const factory& f, const cur_node_type& top_level) {
	graph<factory, origin> ret;//{ &top_level, {} };
	for(auto&& i:top_level) {
		ret.children.emplace_back();
		ret.children.back().value = f.alloc_smart(mk_graph<origin>(f, i));
	}
	return ret;
}
template<typename origin, typename factory, variant cur_node_type>
constexpr auto mk_graph(const factory& f, const cur_node_type& top_level) {
	return visit([](auto& val){
		return graph<factory, origin>{ &val, {} };
	}, top_level);
}
template<typename origin, typename factory, any_ptr cur_node_type>
constexpr auto mk_graph(const factory& f, const cur_node_type& top_level) {
	graph<factory, origin> g;
	if(top_level) {
		g = mk_graph<origin>(f, *top_level);
	}
	return g;
}

} // namespace ast_graph
