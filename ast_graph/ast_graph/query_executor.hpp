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

template<typename factory>
struct vertex_evaluator {
	using vertex = ast_vertex<factory>;
	using qgraph = details::query_graph<factory>;
	using qvertex = qgraph::qvertex;

	factory f;
	const vertex* input;

	constexpr bool operator()(const qvertex& q) const { return visit(*this, q.data); }
	constexpr bool operator()(const qvertex::emb_expr& q) const {
		return (bool)(solve_vertex(f, q, input));
	}
	constexpr bool operator()(const qvertex::own_expr& q) const {
		bool result = true;
		for(auto& i:q) result &= visit(*this, i);
		return result;
	}
	constexpr auto operator()(const qvertex::by_name& q) const {
		auto value = input->field("name");
		return value.is_string() && value == q.name;
	}
	constexpr auto operator()(const qvertex::by_type_and_name& q) const {
		auto name_value = input->field("name");
		auto type_value = input->field("type");
		if(!type_value.is_string()) type_value = typename vertex::data_type{f, mk_str(f, input->type_name())};
		return
		     name_value.is_string() && name_value == q.name
		  && type_value == q.type
		  ;
	}
	constexpr auto operator()(const qvertex::by_field_value& q) const {
		auto value = input->field(q.field);
		return visit([&](const auto& v){return value==v;}, q.value);
	}
};

template<typename factory>
struct path_evaluator {
	using evertex = vertex_evaluator<factory>;
	using qgraph = evertex::qgraph;
	using qedge = details::query_edge<factory>;
	using vertex = evertex::vertex;
	using holder = decltype(mk_vec<const vertex*>(std::declval<factory>()));
	using graph_type = common_graph<factory>;

	factory f;
	const graph_type* graph;
	holder cur_input;
	holder output;
	holder cur_chain;

	int cur_deep{0};
	int cur_match{0};

	template<auto line>
	constexpr void debug() const {
		if !consteval {
			std::cout << line << ": " << output.size() << ' ' << cur_input.size() << std::endl;
		}
	}

	constexpr bool check_edge(const qedge& q, int iter, const vertex* par, holder& out) const {
		auto links = graph->links_of(par);
		for(auto& link:links) {}
		return true;
	}
	constexpr bool check_edge(const qedge& q) {
		debug<__LINE__>();
		for(auto& i:cur_input) {
			auto cur_out = mk_vec<const vertex*>(f);
			if(check_edge(q, 0, i, cur_out)) for(auto& v:cur_out) output.emplace_back(v);
		}
		debug<__LINE__>();
		return true;
	}

	constexpr int operator()(const qgraph& q) {
		debug<__LINE__>();
		return (*this)(q.data);
	}
	template<typename... types>
	constexpr int operator()(const qgraph::template variant<types...>& v) {
		//v.foo();
		debug<__LINE__>();
		return visit(*this, v);
	}
	constexpr int operator()(const qgraph::add& ) { debug<__LINE__>(); return __LINE__; }
	constexpr int operator()(const qgraph::sub& ) { debug<__LINE__>(); return __LINE__; }
	constexpr int operator()(const qgraph::qvertex& q) {
		debug<__LINE__>();
		for(auto& i:cur_input) {
			if(vertex_evaluator{f, i}(q)) {
				output.emplace_back(i);
			}
		}
		debug<__LINE__>();
		return __LINE__;
	}
	constexpr int operator()(const qgraph::path& q) {
		debug<__LINE__>();
		// make input
		(*this)(*q.start);
		cur_input = std::move(output);
		output = mk_vec<const vertex*>(f);
		debug<__LINE__>();
		for(auto& vertex:cur_input) {
			cur_deep = 0;
			cur_match = 0;
			cur_chain.clear();
			for(auto& i:q.tail) if(!check_path_end(i, vertex)) break;
		}
		return __LINE__;
	}
	constexpr bool check_path_end(const qgraph::path_end& q, const vertex* cur_parent) {
		/*
		if(-1 < q.edge.max_deep && q.edge.max_deep < ++cur_deep) return false;
		path_evaluator eval{ f, graph, cur_input };
		eval(*q.vertex);
		auto next_layer = std::move(eval.output);
		if(next_layer.empty()) return false;
		cur_chain.emplace_back(cur_parent);
		*/
		return false;
	}
	constexpr bool operator()(const qgraph::path_end& q) {
		debug<__LINE__>();
		debug<__LINE__>();
		if(!check_edge(q.edge)) return false;
		debug<__LINE__>();
		(*this)(*q.vertex);
		debug<__LINE__>();
		return true;
	}
};

template<typename factory>
struct graph_evaluator {
	using evertex = vertex_evaluator<factory>;
	using vertex = evertex::vertex;

	factory f;
};

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
	using qgraph = details::query_graph<factory>;
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
			if(solve_vertex(*pf, f, q, i.base)) result.add_vertex(&i);
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
