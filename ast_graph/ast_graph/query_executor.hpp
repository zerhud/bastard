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
	using qgraph = details::query_graph<factory>;

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

	constexpr auto exec(auto&& parsed) {
		return 1;
	}
	constexpr auto operator()(auto&& query) {
		qgraph r;
		parse( qgraph::template mk_parser<parser>(f), +parser::space, parser::make_source(std::forward<decltype(query)>(query)), r.data );
		return exec(std::move(r));
	}
};

} // namespace ast_graph
