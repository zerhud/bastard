/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "ast_graph/mk_children_types.hpp"
#include "node.hpp"
#include <utility>

namespace ast_graph {

template<typename... types>
struct vertex {
	void* data = nullptr;
	unsigned int index = 0;

	template<typename type> constexpr void set_index() {
		index = 0;
		( (++index, std::is_same_v<type, types>) || ... );
	}
};

template<typename string_view, typename... types>
struct edge {
	struct vertex<types...>* child=nullptr, parent=nullptr;
	string_view name;
};

template<typename factory, typename... types>
struct graph {
	static constexpr auto mk_vlist(const factory& f) {
		return f.template mk_vec<vertex<types...>>();
	}

	static constexpr auto mk_elist(const factory& f) {
		return f.template mk_vec<edge<typename factory::string_view, types...>>();
	}

	using vertex_type = decltype(mk_vlist(factory{}));
	using edge_type = decltype(mk_elist(factory{}));

	graph(const factory& f)
		: vlist(mk_vlist(f))
		, elist(mk_elist(f))
	{}

	vertex_type vlist;
	edge_type elist;
};

template<typename factory, template<typename...>class tlist, typename... types>
constexpr auto mk_empty_result(const factory& f, tlist<types...>) {
	return graph<factory, types...>(f);
}

template<typename factory, typename origin>
constexpr auto make_graph(auto& result, const factory& f, const origin& top) {
	using tlist = decltype(mk_children_types(f, top));
	node<factory, origin> info(top);
	info.for_each_child_value([](auto&& n, auto&& v){
		;
	});
}


template<typename factory, typename origin>
constexpr auto make_graph(const factory& f, const origin& top) {
	using tlist = decltype(mk_children_types(f, top));
	auto result = mk_empty_result(f, tlist{});
	make_graph(result, f, top);
	return result;
}

constexpr auto query(const auto& factory, const auto& top, auto&& src) {
	using tlist = decltype(mk_children_types(factory, top));
	return 1;
}

} // namespace ast_graph
