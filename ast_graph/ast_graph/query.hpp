/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <utility>
#include "mk_children_types.hpp"
#include "node.hpp"

namespace ast_graph {

template<typename... types>
struct vertex {
	const void* data;
	unsigned index;
};
template<typename type, typename... types> constexpr auto index_of(const vertex<types...>&) {
	static_assert( (std::is_same_v<type, types> || ...), "no type found in available types" );
	unsigned ret = 0;
	return ( (++ret, std::is_same_v<type, details::ref::decay_t<types>>) || ... ) ;
}
template<typename type, typename... types> constexpr void set_index(vertex<types...>& v) {
	static_assert( (std::is_same_v<type, types> || ...), "no type found in available types" );
	v.index = index_of<type>(v);
}
namespace details {
template<typename type, typename... types>
constexpr auto call_fnc(const vertex<types...>& v, auto&& fnc) {
	//return fnc(static_cast<const type*>(v.data));
	if(v.index==index_of<type>(v)) return fnc(static_cast<const type*>(v.data));
	return 1;
}
} // namespace details
template<typename... types>
constexpr auto exec(const vertex<types...>& v, auto&& fnc) {
	if(sizeof...(types) <= v.index) std::unreachable();
	return ( (v.index == index_of<types>(v) && details::call_fnc<types>(v, fnc)) || ... );
}

template<typename factory, typename... types>
struct edge {
	using string_view = typename factory::string_view;
	string_view name;
	unsigned parent;
	unsigned child;
};

template<typename factory, typename... types>
struct graph {
	using edges_type = decltype(mk_vec<struct edge<factory, types...>>(factory{}));
	using vertexes_type = decltype(mk_list<struct vertex<types...>>(factory{}));

	edges_type edges;
	vertexes_type vertexes;
	const struct vertex<types...>* root;

	constexpr graph(const factory& f)
		: edges(mk_vec<struct edge<factory, types...>>(f))
		, vertexes(mk_list<struct vertex<types...>>(f))
	{}
};
template<typename factory, typename... types>
constexpr auto child(const graph<factory, types...>& g, const vertex<types...>* v, unsigned ind) {
	unsigned cur_ind = 0;
	for(auto& cur:g.edges) {
		if(&g.vertexes[cur.parent] == v) {
			if(cur_ind++ == ind) return &g.vertexes[cur.child];
		}
	}
	return static_cast<const vertex<types...>*>(nullptr);
}

namespace details {
template<typename factory, template<typename...> class holder, typename... types>
constexpr auto mk_empty_result(const factory &f, holder<types...>) {
	return graph<factory, types...>{f};
}
template<typename type>
constexpr auto add_vertex(auto& result, const type& source) {
	result.vertexes.emplace_back().data = &source;
	set_index<type>(result.vertexes.back());
	return result.vertexes.size()-1;
}
template<typename type, typename factory>
constexpr auto fill_with_all_data(auto& result, const factory& f, const type& source, unsigned parent) {
	node<factory, type>{f, &source}.for_each_child_value([&](auto&& name, auto& child) {
		auto cur = add_vertex(result, child);
		result.edges.emplace_back(name, parent, cur);
		fill_with_all_data(result, f, child, cur);
	});
}
} // namespace details

constexpr auto query(const auto& f, const auto& source, auto&& q) {
	auto result = details::mk_empty_result(f, mk_children_types(f, source));
	details::fill_with_all_data(result, f, source, details::add_vertex(result, source));
	result.root = &result.vertexes[0];
	return result;
}

} // namespace ast_graph
