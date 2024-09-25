/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "ast_graph/query.hpp"
#include "factory.hpp"
#include "ascip.hpp"

#include <variant>

struct inner_vertex_expr : std::variant<int, bool>{
	template<typename ind>
	constexpr friend auto& create(inner_vertex_expr& v) { return v.emplace<ind>(); }
};

struct vertex_solver {
	using parsed_expression = inner_vertex_expr;

	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr friend auto create_parser(const vertex_solver&) {
		return gh::int_ | as<true>(th<'t'>::_char);
	}

	constexpr friend auto copy_and_add_to_env(const vertex_solver& vs, auto&&, auto&&) {
		return vs;
	}
};

struct factory : ast_graph_tests::query_factory{ };

using data = absd::data<factory>;

using parser = ascip<std::tuple, factory>;
using qedge = ast_graph::details::query_edge<factory>;
using qgraph = ast_graph::details::query_graph<factory, vertex_solver>;
using vertex = qgraph::qvertex;

int main(int,char**) {

	static_assert( []{
		qedge r;
		parse(qedge::mk_parser<parser>(), +parser::space, parser::make_source("-[1:2:name]->"), r);
		return (r.stop_on_match==1) + 2*(r.max_deep==2) + 4*(r.name=="name");
	}() == 7 );
	static_assert( []{
		qedge r;
		parse(qedge::mk_parser<parser>(), +parser::space, parser::make_source("-[test]->"), r);
		return (r.name=="test");
	}() == true );
	static_assert( []{
		qedge r_min, r_max;
		auto rr1 = parse(qedge::mk_parser<parser>(), +parser::space, parser::make_source("->"), r_min);
		auto rr2 = parse(qedge::mk_parser<parser>(), +parser::space, parser::make_source("-->"), r_max);
		return ((r_min.max_deep==1) + 2*(r_max.max_deep==-1) + 4*(r_min.name == "")) / (rr1==2) / (rr2==3);
	}() == 7 );
	static_assert( []{
		qgraph r1;
		return parse(qgraph::mk_parser<parser>(factory{}, vertex_solver{}), +parser::space, parser::make_source("{1}->{}->({}+3{})"), r1.data);
	}() == 17 );
	static_assert( []{
		vertex r;
		parse(vertex::mk_parser<parser>(vertex_solver{}), +parser::space, parser::make_source("{}"), r);
		return holds_alternative<bool>(r.data) + 2*(get<bool>(r.data)==true);
	}() == 3, "empty braces is the bool value true in result" );

	return 0;
}
