/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"

#include "ast_graph/query.hpp"
#include "ascip.hpp"
#include "absd.hpp"

#include <variant>

struct inner_vertex_expr : std::variant<int, bool>{
	template<typename ind>
	constexpr friend auto& create(inner_vertex_expr& v) { return v.emplace<ind>(); }
};

struct factory : tests::factory{
	using vertex_expression_parsed_type = inner_vertex_expr;

	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr friend auto create_vertex_parser(const factory&) {
		return gh::int_ | as<true>(th<'t'>::_char);
	}
};

using data = absd::data<factory>;

using parser = ascip;
using qedge = ast_graph::details::query_edge<factory>;
using qgraph = ast_graph::details::query_graph<factory>;
using vertex = qgraph::qvertex;

int main(int,char**) {

	static_assert( []{
		qedge r;
		parse(qedge::mk_parser<parser>(), +parser::space, parser::make_source("-[2:name]->"), r);
		return 1*(r.max_deep==2) + 2*(r.name=="name");
	}() == 3 );
	static_assert( []{
		qedge r;
		parse(qedge::mk_parser<parser>(), +parser::space, parser::make_source("-2->"), r);
		return 1*(r.max_deep==2) + 2*(r.name.empty());
	}() == 3 );
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
		return parse(qgraph::mk_parser<parser>(factory{}), +parser::space, parser::make_source("{{1}}->{{}}->({{}}+3{{}})"), r1.data);
	}() == 25 );
	static_assert( []{
		vertex r;
		parse(vertex::mk_parser<parser>(factory{}), +parser::space, parser::make_source("{}"), r);
		return get<1>(r.data).size();
	}() == 0, "empty braces is the empty vector in result" );
	static_assert( []{
		vertex r;
		parse(vertex::mk_parser<parser>(factory{}), +parser::space, parser::make_source("{{}}"), r);
		return holds_alternative<bool>(get<0>(r.data)) + 2*(get<bool>(get<0>(r.data))==true);
	}() == 3, "empty braces is the bool value true in result" );

	static_assert( []{
		vertex r;
		parse(vertex::mk_parser<parser>(factory{}), +parser::space, parser::make_source("{'name'}"), r);
		return (get<1>(r.data).size()==1) + 2*(get<2>(get<1>(r.data)[0]).name == "name");
	}() == 3, "empty braces is the bool value true in result" );
	static_assert( []{
		vertex r;
		parse(vertex::mk_parser<parser>(factory{}), +parser::space, parser::make_source("{'type':'name'}"), r);
		return (get<1>(r.data).size()==1) + 2*(get<0>(get<1>(r.data)[0]).name == "name") + 4*(get<0>(get<1>(r.data)[0]).type == "type");
	}() == 7, "empty braces is the bool value true in result" );
	static_assert( []{
		vertex r;
		parse(vertex::mk_parser<parser>(factory{}), +parser::space, parser::make_source("{'field'='value'}"), r);
		return (get<1>(r.data).size()==1) + 2*(get<1>(get<1>(r.data)[0]).field == "field") + 4*(get<2>(get<1>(get<1>(r.data)[0]).value) == "value");
	}() == 7, "empty braces is the bool value true in result" );
}
