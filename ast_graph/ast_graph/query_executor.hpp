/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "query.hpp"
#include "graph.hpp"

namespace ast_graph {

/* use cases:
 *  1. all graph 
 *  2. all graph with max-deep
 *  3. all curent graph (it was map_to) ?? how to detected?
 *  4. all nodes from dsl's root (or from custom) linked with custom node
 *  5. node by name from dsl's root
 *  6. whole dsl by name
 *  7. check if custom node is child of dsl's root or child of other custom node
 *  8. node by fields, link, children...
 */

template<typename factory, typename source>
struct query_executor {
	using graph_type = decltype(mk_graph(std::declval<factory>(), std::declval<source>()));
	using parser = typename factory::parser;

	factory f;
	const source& src;
	graph_type graph;
	graph_type result;

	constexpr query_executor(factory f, const source& src)
	: f(f)
	, src(src)
	, graph(mk_graph(f, src))
	, result(mk_empty_graph(f, src))
	{}

	template<typename pf>
	constexpr bool is_node_matched(const details::query_vertex<pf>& parser, const auto& vertex) const {
		return true;
	}
	template<typename pf>
	constexpr void remove_unmatched_children(const details::query_edge<pf>* parser, auto& vertex) const {
		vertex.children.clear();
	}

	constexpr void remove_unmatched_children(unsigned levels_count, auto& vertex) {
		if(levels_count==0) return;
		vertex.children.clear();
	}

	template<typename parser_factory>
	constexpr auto next(const details::query<parser_factory>* next, auto& prev_vertex) {
		;
	}

	template<typename parser_factory>
	constexpr auto exec(const details::query<parser_factory>& parser) {
		//TODO: fuck souciety
		auto& vertex_query = get<0>(parser.data);
		for(auto& v:graph) if(is_node_matched(vertex_query, *v.base)) remove_unmatched_children(parser.next ? &get<2>(parser.next->data) : nullptr, *result.emplace_back(v).base);
		auto ret = mk_empty_graph(f, src);
		swap(ret, result);
		return ret;
	}	
	constexpr auto operator()(auto&& query) {
		auto parsed = details::parse_query<parser>(f, query);
		return exec(parsed);
	}
};

} // namespace ast_graph
