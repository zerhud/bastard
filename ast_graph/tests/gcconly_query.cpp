/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

//#include "ast_graph/query.hpp"
#include "ast_graph/query2.hpp"
#include "factory.hpp"
#include "ascip.hpp"

struct factory : ast_graph_tests::query_factory{ };

using parser = ascip<std::tuple, factory>;
using qedge = ast_graph::details::query_edge<factory>;
using vertex = ast_graph::details::query_vertex<factory>;
using qgraph = ast_graph::details::query_graph<factory>;

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
		vertex r1, r2;
		parse(vertex::mk_parser<parser>(factory{}), +parser::space, parser::make_source("2{1==2}"), r1);
		auto rr2 = parse(vertex::mk_parser<parser>(factory{}), +parser::space, parser::make_source("{1==2}"), r2);
		return (r1.arg_number==2) + 2*(r1.data.index()==0) + 4*(r2.arg_number==0) + 8*(rr2==6);
	}() == 15 );
	static_assert( []{
		qgraph r1;
		return parse(qgraph::mk_parser<parser>(factory{}), +parser::space, parser::make_source("{1==2}->{}->{}"), r1.data);
	}() == 14 );

	return 0;
}
