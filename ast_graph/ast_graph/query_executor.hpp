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

template<typename factory, typename graph_type>
struct path_evaluator {
	using evertex = vertex_evaluator<factory>;
	using qgraph = evertex::qgraph;
	using qedge = details::query_edge<factory>;
	using vertex = evertex::vertex;
	using holder = decltype(mk_vec<const vertex*>(std::declval<factory>()));
	using graph_view = decltype(std::declval<graph_type>().create_view());

	factory f;
	const graph_type* graph;
	graph_view cur_input;
	graph_view output;

	int cur_deep{0};
	int cur_match{0};

	constexpr path_evaluator(factory f, const graph_type* graph, graph_view input)
	: f(std::move(f))
	, graph(graph)
	, cur_input(std::move(input))
	, output(graph->create_empty_view())
	{}

	template<auto line>
	constexpr void debug() const {
		if !consteval {
			std::cout << line << ": " << output.size() << ' ' << cur_input.size() << std::endl;
		}
	}

	constexpr auto& operator()(const qgraph& q) { (*this)(q.data); return *this; }
	template<typename... types> constexpr auto operator()(const qgraph::template variant<types...>& v) {
		return visit(*this, v);
	}
	constexpr void operator()(const qgraph::add& q) {
		visit(*this, *q.left);
		auto left = output;
		visit(*this, *q.right);
		output += left;
	}
	constexpr void operator()(const qgraph::sub& q) {
		visit(*this, *q.right);
		auto right = output;
		visit(*this, *q.left);
		output -= right;
	}
	constexpr void operator()(const qgraph::qvertex& q) {
		output = graph->create_empty_view();
		for(auto& i:cur_input.vertices) {
			if(vertex_evaluator{f, i}(q)) output.add_vertex(i);
		}
	}
	constexpr void operator()(const qgraph::path& q) {
		visit(*this, *q.start);
		auto matched = std::move(output);
		output = graph->create_empty_view();
		check_path_end(q.tail.begin(), q.tail.end(), matched);
	}
	constexpr void check_path_end(auto pos, auto end, graph_view src) {
		for(auto& v:src.vertices) {
			if(check_path_end(pos, end, v)) output.add_vertex(v);
		}
	}
	constexpr bool check_path_end(auto pos, auto end, const vertex* parent) {
		if(pos==end) return true;
		bool result = false;
		auto& cur_info = *pos;
		pos += 1;
		path_evaluator matched{ f, graph, cur_input };
		matched(*cur_info.vertex);
		for(auto& target:matched.output.vertices)
			result |= check_path_end(pos, end, cur_info, parent, target);
		return result;
	}
	constexpr bool check_path_end(auto pos, auto end, const qgraph::path_end& info, const vertex* parent, const vertex* child) {
		auto path = graph->path(parent, child);
		if(check_path_end(info, path) && check_path_end(pos, end, child)) {
			for(auto& part:path) output.add_vertex(part.child);
			return true;
		}
		return false;
	}
	constexpr bool check_path_end(const qgraph::path_end& info, const auto& path) const {
		if(!info.edge.name.empty()) for(auto& p:path) if(p.name != info.edge.name) return false;
		return !path.empty() && (info.edge.max_deep < 0 || info.edge.max_deep==path.size());
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
