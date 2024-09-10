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

template<typename factory, typename source>
struct query_vertex_executor {

	template<template<typename...>class variant, typename... list>
	constexpr static auto mk_pointer_variant(const variant<list...>&) {
		return variant<const list*...>{};
	}

	using qvertex = details::query_vertex<factory>;
	using graph_type = decltype(mk_graph(std::declval<factory>(), std::declval<source>()));
	using string_t = qvertex::string_t;
	using result_t = qvertex::expr;

	factory f;
	const ast_vertex<factory>* node;

	constexpr bool check_name(const auto& name) const {
		auto fields = node->fields();
		if(auto* str_name = get_if<string_t>(&name);str_name) {
			//for(auto& cur:fields) if(match(f, cur, *str_name)) return true;
			for(auto& f:fields) if(f==*str_name) return true;
		}
		else {
			auto& name_id = get<typename qvertex::ident>(name).val;
			for(auto& f:fields) if(f==name_id) return true;
		}
		return false;
	}

	constexpr auto operator()(const auto& op) { return result_t{false}; }
	/*
	constexpr auto operator()(const qvertex::eq& op) { }
	constexpr auto operator()(const qvertex::neq& op) { }
	constexpr auto operator()(const qvertex::_and& op) { }
	constexpr auto operator()(const qvertex::_or& op) { }
	constexpr auto operator()(const qvertex::in& op) { }
	*/
	constexpr auto operator()(const qvertex::type_name_eq& op) {
		auto type = get<string_t>(visit(*this, *op.left));
		return result_t{check_name(*op.left) && node->type_name() == type};
	}
	constexpr auto operator()(const qvertex::name_eq& op) {
		return result_t{check_name(*op.left)};
	}
	/*
	constexpr auto operator()(const qvertex::_not& op) { }
	*/
	constexpr auto operator()(const qvertex::ident& op) {
		return result_t{op.val};
	}
	constexpr auto operator()(const qvertex::string_t& op) { return result_t{op}; }
	/*
	constexpr auto operator()(const qvertex::integer_t& op) { }
	constexpr auto operator()(const qvertex::float_point_t& op) { }
	*/
	constexpr auto operator()(const bool& op) { return result_t{op}; }
	constexpr auto operator()(const auto* n, const auto& op) {
		node = n;
		return get<bool>(visit( *this, op.data ));
	}
};
template<typename factory, typename source>
struct query_executor {
	using graph_type = decltype(mk_graph(std::declval<factory>(), std::declval<source>()));
	using parser = typename factory::parser;
	using qgraph = details::query_graph<factory>;
	using qvertex = details::query_vertex<factory>;
	using vertex_type = typename graph_type::value_type;
	using vertex_executor = query_vertex_executor<factory, source>;

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

	constexpr auto operator()(const auto& q) requires requires { visit([](const auto&){}, q); } {
		return visit( *this, q );
	}
	constexpr auto operator()(const qgraph& q) { return visit(*this, q.data); }
	constexpr auto operator()(const qvertex& q) { }
	constexpr auto operator()(const qgraph::path& q) {
	}
	constexpr auto operator()(const qgraph::add& q) {
	}
	constexpr auto operator()(const qgraph::sub& q) {
	}
	/*
	constexpr bool check_vertex(const vertex_type& v, const qvertex& q) {
		if(holds_alternative<bool>(q.data)) return get<bool>(q.data);
		if(holds_alternative<typename qvertex::name_eq>(q.data)) {
			return v.fields().contains(get<>(q.data).
			if !consteval { std::cout << q.data.index() << std::endl; }
		}
		//auto& eq = get<0>(get<0>(q.data));
		return false;
	}
	*/
	constexpr auto exec(auto&& parsed) {
		graph_type ret;
		vertex_executor qv;
		for(auto& i:graph) {
		//	if(check_vertex(i, get<2>(parsed.data))) ret.emplace_back(i);
			if(qv(i.base, get<2>(parsed.data))) ret.emplace_back(i);
		}
		return ret;
	}
	constexpr auto operator()(auto&& query) {
		qgraph r;
		parse( qgraph::template mk_parser<parser>(f), +parser::space, parser::make_source(std::forward<decltype(query)>(query)), r.data );
		return exec(std::move(r));
	}
};

} // namespace ast_graph
