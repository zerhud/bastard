#include <vector>
#include <variant>

#include "tests/factory.hpp"
#include "absd/iostream_op.hpp"
#include "jiexpr.hpp"
#include "jiexpr/default_operators.hpp"
#include "ast_graph/query_executor.hpp"
#include "ast_graph/absd_object.hpp"
#include "ascip.hpp"


namespace test_data {
struct variant_leaf1 {
	int v1f = 0;
};
struct variant_leaf2 {
	int v2f = 1;
	std::string name;
};
struct pointer_leaf {
	int pf = 3;
};
struct test_leaf {
	int ff = 3;
	std::variant<variant_leaf1, variant_leaf2> vl;
	std::unique_ptr<pointer_leaf> pl;
};
static_assert(tref::gs::size<test_leaf> == 3);

struct test_fields {
	int field_1 = 1;
	int field_2 = 2;
	std::vector<test_leaf> leafs;
};

} // namespace test_data

struct factory : tests::factory {
	using data_type = absd::data<factory>;
	using parser = ascip<std::tuple>;
};

using data_type = factory::data_type;
using graph_type = decltype(ast_graph::mk_graph(factory{}, test_data::test_fields{}));
using jiexpr_test = jiexpr<data_type, jiexpr_details::expr_operators_simple, factory>;

struct parser_factory {
	data_type env;
	jiexpr_test* jiexpr;

	constexpr friend const auto& vertex_expression(const parser_factory& f) { return *f.jiexpr; }
	constexpr friend auto solve_vertex(parser_factory& self, const auto& f, const auto& expr, const auto* graph) {
		data_type env;
		env.mk_empty_object();
		env.put(data_type{"v"}, data_type::mk(ast_graph::absd_object(f, graph)));
		jiexpr_test::solve_info info{};
		info.env = &env;
		return expr.solve(info);
	}
};

struct fixture {
	factory f;
	test_data::test_fields src_st;
	graph_type graph = ast_graph::mk_graph(f, src_st);
	jiexpr_test jiexpr;
	parser_factory pf;

	constexpr fixture() {
		pf.jiexpr = &jiexpr;
	}

	constexpr auto mk_executor() {
		return ast_graph::query_executor( f, &pf, src_st );
	}

	constexpr auto mk_obj(auto view, const auto* g) const {
		return data_type::mk(ast_graph::graph_absd(f, view, g));
	}
	constexpr auto mk_obj() const {
		return mk_obj(graph.create_view(), graph.root().base);
	}
};

static_assert( fixture{}.mk_executor()("{{v.field_1==1}}").size() == 1 );
static_assert( [f=fixture{}]mutable{
	auto q = f.mk_executor();
	auto r = q("{{}}");
	return (r.size() > 0) + 2*(f.mk_obj() == f.mk_obj(r, r.root()));
}() == 3 );

int main(int,char**) {
	return 0;
}