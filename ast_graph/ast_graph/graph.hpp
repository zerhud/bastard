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
#include "mk_children_types.hpp"

namespace ast_graph {

namespace details {

template<template<typename...>typename list, typename... types>
constexpr auto mk_variant(const auto& f, const list<types...>&) {
	return f.template mk_variant<const ref::decay_t<types>*...>();
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
							mk_children_types(
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
