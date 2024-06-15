/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <utility>
#include "concepts.hpp"
#include "mk_children_types.hpp"
#include "node.hpp"

namespace ast_graph {

namespace details{
template<typename type, typename... types> struct index_of;
template<typename type, typename... types> struct index_of<type, type, types...> : std::integral_constant<unsigned, 0> {};
template<typename type, typename cur, typename... types> struct index_of<type, cur, types...> : std::integral_constant<unsigned, 1+index_of<type, types...>::value> {};
template<typename type, typename... types> constexpr auto index_of_v = index_of<type, types...>::value;
}

template<typename factory>
struct base_vertex {
	using field_name_type = typename factory::field_name_type;
	using data_type = typename factory::data_type;
	constexpr virtual ~base_vertex() =default ;
	constexpr virtual data_type field(field_name_type fn) const =0 ;
	constexpr virtual bool is_array() const =0 ;
};

template<typename factory, typename type>
struct vertex_info : base_vertex<factory> {
	using field_name_type = typename factory::field_name_type;
	using data_type = typename factory::data_type;

	factory f;
	const type* data;
	constexpr explicit vertex_info(const factory& f, const type* data) : f(f), data(data) {}
	constexpr bool is_array() const override { return vector<type>; }
	constexpr virtual data_type field(field_name_type fn) const override {
		if constexpr (vector<type>) return data_type{} ;
		else {
			struct node<factory, type> node{f, data};
			data_type ret{};
			node.for_each_field_value([&](auto&& name, const auto& v) {
				if (name == fn) ret = v;
			});
			return ret;
		}
	}
};

template<typename factory, typename... types>
struct vertex {
	using holder_type = typename factory::template variant<vertex_info<factory, types>...>;

	template<typename current>
	constexpr explicit vertex(const factory& f, const current* val)
	: data(val)
	, index(details::index_of_v<current, types...>)
	, holder(vertex_info<factory, current>{f, val})
	, info(get<vertex_info<factory, current>>(holder))
	{
		static_assert( (std::is_same_v<current, types> || ...), "no type found in available types" );
	}

	const void* data;
	unsigned index;
	holder_type holder;
	const base_vertex<factory>& info;
};

template<typename factory, typename... types>
struct edge {
	using field_name_type = typename factory::field_name_type;
	field_name_type name;
	unsigned parent;
	unsigned child;
};

template<typename factory, typename... types>
struct graph {
	using edges_type = decltype(mk_vec<struct edge<factory, types...>>(factory{}));
	using vertexes_type = decltype(mk_list<struct vertex<factory, types...>>(factory{}));

	edges_type edges;
	vertexes_type vertexes;
	const struct vertex<factory, types...>* root;

	constexpr graph(const factory& f)
		: edges(mk_vec<struct edge<factory, types...>>(f))
		, vertexes(mk_list<struct vertex<factory, types...>>(f))
	{}
};
template<typename factory, typename... types>
constexpr auto child(const graph<factory, types...>& g, const vertex<factory, types...>* v, unsigned ind) {
	unsigned cur_ind = 0;
	for(auto& cur:g.edges) {
		if(&g.vertexes[cur.parent] == v) {
			if(cur_ind++ == ind) return &g.vertexes[cur.child];
		}
	}
	return static_cast<const vertex<factory, types...>*>(nullptr);
}

namespace details {
template<typename type, typename factory>
constexpr auto fill_with_all_data(auto& result, const factory& f, const type& source, unsigned parent) ;
template<typename factory, template<typename...> class holder, typename... types>
constexpr auto mk_empty_result(const factory &f, holder<types...>) {
	return graph<factory, types...>{f};
}
template<typename type, typename factory>
constexpr auto add_vertex(auto& result, const type& source, const factory& f) {
	result.vertexes.emplace_back(f, &source);
	return result.vertexes.size()-1;
}
template<typename type, typename factory>
constexpr auto add_child(auto& result, const factory& f, auto&& name, const type& child, unsigned parent) {
	auto cur = add_vertex(result, child, f);
	result.edges.emplace_back(name, parent, cur);
	fill_with_all_data(result, f, child, cur);
}
template<vector type, typename factory>
constexpr auto add_child(auto& result, const factory& f, auto&& name, const type& child, unsigned parent) {
	auto cur = add_vertex(result, child, f);
	result.edges.emplace_back(name, parent, cur);
	for(unsigned i=0;i<child.size();++i) {
		add_child(result, f, to_field_name(f, i), child[i], cur);
	}
}
template<variant type, typename factory>
constexpr auto add_child(auto& result, const factory& f, auto&& name, const type& child, unsigned parent) {
	return visit([&](const auto& child){ return add_child(result, f, std::forward<decltype(name)>(name), child, parent); }, child);
}
template<any_ptr type, typename factory>
constexpr auto add_child(auto& result, const factory& f, auto&& name, const type& child, unsigned parent) {
	return add_child(result, f, std::forward<decltype(name)>(name), *child, parent);
}
template<typename type, typename factory>
constexpr auto fill_with_all_data(auto& result, const factory& f, const type& source, unsigned parent) {
	node<factory, type>{f, &source}.for_each_child_value([&](auto&& name, auto& child) {
		add_child(result, f, name, child, parent);
	});
}
} // namespace details

constexpr auto query(const auto& f, const auto& source, auto&& q) {
	auto result = details::mk_empty_result(f, mk_children_types(f, source));
	details::fill_with_all_data(result, f, source, details::add_vertex(result, source, f));
	result.root = &result.vertexes[0];
	return result;
}

} // namespace ast_graph
