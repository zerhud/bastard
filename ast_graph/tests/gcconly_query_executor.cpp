#include <vector>
#include <variant>

#include "tests/factory.hpp"
#include "absd/iostream_op.hpp"
#include "jiexpr.hpp"
#include "ast_graph/query_executor.hpp"
#include "ast_graph/absd_object.hpp"
#include "ascip.hpp"


namespace test_data {
struct variant_leaf1 {
	int v1f = 0;
};
struct variant_leaf_with_name {
	int v2f = 1;
	std::string name;
};
struct variant_leaf_with_name_and_type {
	int v3f = 2;
	std::string name;
	std::string type; //TODO: what if we want to use enum here ?
};
struct pointer_leaf {
	int pf = 3;
};
struct test_leaf {
	int ff = 3;
	std::variant<variant_leaf1, variant_leaf_with_name, variant_leaf_with_name_and_type> vl;
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
using jiexpr_test = jiexpr<factory>;

struct parser_factory : factory {
	data_type env;
	jiexpr_test jiexpr;

	constexpr friend const auto& vertex_expression(const parser_factory& f) { return f.jiexpr; }
	constexpr friend auto solve_vertex(parser_factory& self, const auto& f, const auto& expr, const auto* graph) {
		data_type env;
		env.mk_empty_object();
		env.put(data_type{"v"}, data_type::mk(ast_graph::absd_object(f, graph)));
		jiexpr_test::solve_info info{};
		info.env = &env;
		return expr.solve(info);
	}
};

template<typename test>
struct test_fixture : test{
	factory f;
	parser_factory pf;
	test_data::test_fields src_st;
	graph_type graph = ast_graph::mk_graph(f, src_st);

	constexpr test_fixture& recreate() {
		graph = ast_graph::mk_graph(f, src_st);
		return *this;
	}

	constexpr test_fixture& create_named(auto&& name) {
		src_st.leafs.emplace_back().vl.template emplace<1>().name = std::forward<decltype(name)>(name);
		recreate();
		return *this;
	}
	constexpr test_fixture& create_typed(auto&& name, auto&& type) {
		using node_type = test_data::variant_leaf_with_name_and_type;
		node_type& node = src_st.leafs.emplace_back().vl.template emplace<2>();
		node.name = std::forward<decltype(name)>(name);
		node.type = std::forward<decltype(type)>(type);
		recreate();
		return *this;
	}

	constexpr auto* first_leaf(const graph_type& graph) const {
		return graph.links_of(graph.links_of(graph.root().base)[0].child)[0].child;
	}
	constexpr auto* first_leaf() const { return first_leaf(graph); }
	constexpr auto* first_named(const graph_type& graph) const {
		return graph.links_of(first_leaf(graph))[0].child;
	}
	constexpr auto* first_named() const { return first_named(graph); }

	using test::operator();
	constexpr test_fixture(test&& t) : test(std::forward<decltype(t)>(t)) {}
	constexpr auto operator()() { return (*this)(*this); }
};

using vertex_evaluator = ast_graph::vertex_evaluator<factory, parser_factory>;

static_assert( test_fixture{[](auto& f){
	auto qv = get<2>(ast_graph::parse_from(f.pf, "{}").data);
	return
	   vertex_evaluator{f.f, &f.pf, f.graph.root().base}(qv) +
	2*(vertex_evaluator{f.f, &f.pf, f.graph.links_of(f.graph.root().base)[0].child}(qv))
	;
}}() == 3 );

static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto qv = get<2>(ast_graph::parse_from(f.pf, "{'test'}").data);
	return
	    (vertex_evaluator{f.f, &f.pf, f.first_named()}(qv)==true) +
	  2*(vertex_evaluator{f.f, &f.pf, f.first_leaf()}(qv)==false)
	;
}}() == 3, "can find field by name");
static_assert( test_fixture{[](auto& f){
	f.create_typed("test_name", "test_type");
	auto qv = get<2>(ast_graph::parse_from(f.pf, "{'test_type':'test_name'}").data);
	return
	    (vertex_evaluator{f.f, &f.pf, f.first_named()}(qv)==true) +
	  2*(vertex_evaluator{f.f, &f.pf, f.first_leaf()}(qv)==false)
	  ;
}}() == 3, "can find field by name and type if present");
static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto qv = get<2>(ast_graph::parse_from(f.pf, "{'test_data::variant_leaf_with_name':'test'}").data);
	return
	    (vertex_evaluator{f.f, &f.pf, f.first_named()}(qv)==true) +
	  2*(vertex_evaluator{f.f, &f.pf, f.first_leaf()}(qv)==false)
	;
}}() == 3, "can find field by name and type using the class name as type name");
static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto qv = get<2>(ast_graph::parse_from(f.pf, "{'ff'=3}").data);
	return
			(vertex_evaluator{f.f, &f.pf, f.first_named()}(qv)==false) +
			2*(vertex_evaluator{f.f, &f.pf, f.first_leaf()}(qv)==true)
			;
}}() == 3, "can find field by field value");

struct fixture {
	factory f;
	test_data::test_fields src_st;
	graph_type graph = ast_graph::mk_graph(f, src_st);
	jiexpr_test jiexpr;
	parser_factory pf;

	constexpr fixture() {
		//pf.jiexpr = &jiexpr;
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

/*
static_assert( fixture{}.mk_executor()("{{v.field_1==1}}").size() == 1 );
static_assert( [f=fixture{}]mutable{
	auto q = f.mk_executor();
	auto r = q("{{}}");
	return (r.size() > 0) + 2*(f.mk_obj() == f.mk_obj(r, r.root()));
}() == 3 );
*/
/*
static_assert( [f=fixture{}]mutable{
	auto q = f.mk_executor();
	f.src_st.leafs.emplace_back().vl.emplace<test_data::variant_leaf_with_name>().name = "test";
	auto r = q("{'test'}");
	return r.size();
}() == 1 );
*/

int main(int,char**) {
	return 0;
}