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
#include "graph_absd.hpp"

#include <iostream>

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

template<typename factory, typename parser_factory, typename source>
struct query_executor {
	using graph_type = decltype(mk_graph(std::declval<factory>(), std::declval<source>()));
	using graph_view = decltype(std::declval<graph_type>().create_view());
	using parser = typename factory::parser;
	using vertex_expr = std::decay_t<decltype(vertex_expression(std::declval<parser_factory>()))>;
	using qgraph = details::query_graph<factory, vertex_expr>;
	using qvertex = qgraph::qvertex;
	using vertex_type = typename graph_type::vertex_type;

	factory f;
	parser_factory* pf;
	const source& src;
	graph_type graph;
	graph_view result;

	constexpr query_executor(factory f, parser_factory* pf, const source& src)
	: f(f)
	, pf(pf)
	, src(src)
	, graph(mk_graph(f, src))
	, result(f)
	{}

	constexpr auto operator()(const auto& q) requires requires { visit([](const auto&){}, q); } {
		return visit( *this, q );
	}
	// graph
	constexpr auto operator()(const qgraph& q) { return visit(*this, q.data); }
	constexpr auto operator()(const qgraph::path& q) {
		return graph.create_view();
	}
	constexpr auto operator()(const qgraph::add& q) {
		return graph.create_view();
	}
	constexpr auto operator()(const qgraph::sub& q) {
		return graph.create_view();
	}

	// vertex
	constexpr auto operator()(const qvertex& q) { return visit(*this, q.data); }
	constexpr auto operator()(const qvertex::emb_expr& q) {
		for(auto& i:graph.vertices) {
			if(solve_vertex(*pf, f, q, i.base)) add_vertex(result, &i);
		}
		return result;
	}
	constexpr auto operator()(const qvertex::own_expr& q) {
		for(auto& i:q) visit(*this, i);
		return result;
	}
	constexpr auto operator()(const qvertex::by_name& q) {
		//for(auto& i:graph) if()
		return graph.create_view();
	}
	constexpr auto operator()(const qvertex::by_type_and_name& q) {
		return graph.create_view();
	}
	constexpr auto operator()(const qvertex::by_field_value& q) {
		return graph.create_view();
	}
	/*
	constexpr bool check_vertex(const vertex_type& v, const qvertex& q) {
		if(holds_alternative<bool>(q.data)) return get<bool>(q.data);
		if(holds_alternative<typename qvertex::name_eq>(q.data)) {
			return v.fields().contains(get<>(q.data).
			if !consteval { std::cout << q.data.index() << std::endl; }
		}
		//auto& eq = get<0>(get<0>(q.data));
	}
	*/
	constexpr auto operator()(auto&& query) requires requires{ parser::make_source(std::forward<decltype(query)>(query)); } {
		qgraph r;
		const auto& vs = vertex_expression(*pf);
		parse( qgraph::template mk_parser<parser>(f, vs), +parser::space, parser::make_source(std::forward<decltype(query)>(query)), r.data );
		return (*this)(r);
		//return exec(std::move(r));
	}
};

} // namespace ast_graph
