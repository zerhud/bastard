/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include "tests/factory.hpp"

#include "absd.hpp"
#include "ast_graph/graph.hpp"
#include "ast_graph/graph_absd.hpp"

#include <memory>

struct inner_factory : tests::factory {
	using data_type = absd::data<inner_factory>;
};

struct variant_leaf1 { int v1f = 0; };
struct variant_leaf2 { int v2f = 1; };
struct pointer_leaf { int pf = 3; };
struct test_leaf{
	int ff=3;
	std::variant<variant_leaf1, variant_leaf2> vl;
	std::unique_ptr<pointer_leaf> pl;
};
static_assert( tref::gs::size<test_leaf> == 3 );

struct test_fields {
	int field_1=1;
	int field_2=2;
	std::vector<test_leaf> leafs;
};

using absd_data = inner_factory::data_type;

static_assert( []{
	test_fields src{};
	auto g = ast_graph::mk_graph(inner_factory{}, src);
	auto d = absd_data::mk(ast_graph::graph_absd(inner_factory{}, g.create_view(), g.root().base));
	return (d.size()==3) + 2*(d[absd_data{"field_1"}]==absd_data{1}) + 4*(d[absd_data{"field_2"}]==absd_data{2});
}() == 7 );

static_assert( []{
	test_fields src{};
	src.leafs.emplace_back().ff=5;
	src.leafs.emplace_back().ff=7;
	auto g = ast_graph::mk_graph(inner_factory{}, src);
	auto d = absd_data::mk(ast_graph::graph_absd(inner_factory{}, g.create_view(), g.root().base));
	auto ff5 = d[absd_data{"leafs"}][absd_data{0}][absd_data{"ff"}];
	auto ff7 = d[absd_data{"leafs"}][absd_data{1}][absd_data{"ff"}];
	return
		   ((absd_data::integer_t)ff5 == 5)
		+2*((absd_data::integer_t)ff7 == 7);
}() == 3 );

static_assert( []{
	test_fields src{};
	src.leafs.emplace_back().ff=5;
	src.leafs.emplace_back().ff=7;
	auto g = ast_graph::mk_graph(inner_factory{}, src);
	auto d = absd_data::mk(ast_graph::graph_absd(inner_factory{}, g.create_view(), g.root().base));
	auto g_keys = d.keys();
	return
		  (g_keys.size()==3) +
		2*(g_keys[0] == absd_data{"field_1"}) +
	    4*(g_keys[1] == absd_data{"field_2"}) +
		8*(g_keys[2] == absd_data{"leafs"})
		;
}() == 15);

static_assert( []{
	test_fields src{};
	auto g = ast_graph::mk_graph(inner_factory{}, src);
	auto d = absd_data::mk(ast_graph::graph_absd(inner_factory{}, g.create_view(), g.root().base));
	return
		  ( d.contains(absd_data{"field_1"})) +
		2*( d.contains(absd_data{"leafs"})) +
		4*(!d.contains(absd_data{"not_exists"}))
		;
}() == 7 );
static_assert( []{
	test_fields src{};
	auto g = ast_graph::mk_graph(inner_factory{}, src);
	auto d = absd_data::mk(ast_graph::graph_absd(inner_factory{}, g.create_view(), g.root().base));
	return d[absd_data{"not_exists"}].is_none() + !d[absd_data{"field_1"}].is_none();
}() == 2 );
static_assert( []{
	test_fields src{};
	auto g = ast_graph::mk_graph(inner_factory{}, src);
	auto d = absd_data::mk(ast_graph::graph_absd(inner_factory{}, g.create_view(), g.root().base));
	auto via_parent = (absd_data::integer_t)d[absd_data{"leafs"}][absd_data{"__parent"}][absd_data{"field_2"}];
	return d[absd_data{"__parent"}].is_none() + 2*(via_parent==2);
}() == 3 );

int main(int,char**) {
	return 0;
}