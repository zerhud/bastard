#include <vector>
#include <variant>

#include "tests/factory.hpp"
#include "absd/iostream_op.hpp"
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

struct fake_emb_expr_variant : std::variant<bool, int>{};
template<typename type>
constexpr auto& create(fake_emb_expr_variant& v){
	return v.emplace<type>();
}

struct fake_emb_expr {
	using parsed_expression = fake_emb_expr_variant;

	parsed_expression data;

	template<typename gh>
	constexpr static auto create_parser() {
		constexpr auto bp = (as<true>(gh::template lit<"true">) | as<false>(gh::template lit<"false">));
		return use_variant_result(bp) | gh::int_;
	}
};

struct parser_factory : tests::factory {
	using data_type = absd::data<parser_factory>;
	using parser = ascip;
	using vertex_expression_parsed_type = fake_emb_expr_variant;

	constexpr friend auto eval_vertex(const parser_factory&, const auto& expr, const auto* vertex) {
		return visit([](const auto& v){
			return data_type{(bool)v};
		}, expr);
	}
	template<typename parser>
	constexpr friend auto create_vertex_parser(const parser_factory& f) {
		return fake_emb_expr::create_parser<parser>();
	}
};

using data_type = parser_factory::data_type;
using graph_type = decltype(ast_graph::mk_graph(parser_factory{}, test_data::test_fields{}));

template<typename test>
struct test_fixture : test {
	parser_factory pf;
	test_data::test_fields src_st;
	graph_type graph = ast_graph::mk_graph(pf, src_st);

	constexpr test_fixture& recreate() {
		graph = ast_graph::mk_graph(pf, src_st);
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

using vertex_evaluator = ast_graph::vertex_evaluator<parser_factory>;
template<typename... types>
using path_evaluator = ast_graph::path_evaluator<types...>;

static_assert( test_fixture{[](auto& f){
	auto parsed = ast_graph::parse_from(f.pf, "{}");
	auto& qv = get<2>(parsed.data);
	return
	   vertex_evaluator{f.pf, f.graph.root().base}(qv) +
	2*(vertex_evaluator{f.pf, f.graph.links_of(f.graph.root().base)[0].child}(qv))
	;
}}() == 3 );

static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto parsed = ast_graph::parse_from(f.pf, "{'test'}");
	auto& qv = get<2>(parsed.data);
	return
	    (vertex_evaluator{f.pf, f.first_named()}(qv)==true) +
	  2*(vertex_evaluator{f.pf, f.first_leaf()}(qv)==false)
	;
}}() == 3, "can find field by name");
static_assert( test_fixture{[](auto& f){
	f.create_typed("test_name", "test_type");
	auto parsed = ast_graph::parse_from(f.pf, "{'test_type':'test_name'}");
	auto& qv = get<2>(parsed.data);
	return
	    (vertex_evaluator{f.pf, f.first_named()}(qv)==true) +
	  2*(vertex_evaluator{f.pf, f.first_leaf()}(qv)==false)
	  ;
}}() == 3, "can find field by name and type if present");
static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto parsed = ast_graph::parse_from(f.pf, "{'test_data::variant_leaf_with_name':'test'}");
	auto& qv = get<2>(parsed.data);
	return
	    (vertex_evaluator{f.pf, f.first_named()}(qv)==true) +
	  2*(vertex_evaluator{f.pf, f.first_leaf()}(qv)==false)
	;
}}() == 3, "can find field by name and type using the class name as type name");
static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto parsed = ast_graph::parse_from(f.pf, "{'ff'=3}");
	auto& qv = get<2>(parsed.data);
	return
	  (vertex_evaluator{f.pf, f.first_named()}(qv)==false) +
	2*(vertex_evaluator{f.pf, f.first_leaf()}(qv)==true)
	;
}}() == 3, "can find field by field value");
static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto parsed_true = ast_graph::parse_from(f.pf, "{{true}}");
	auto parsed_false = ast_graph::parse_from(f.pf, "{{false}}");
	auto& qt = get<2>(parsed_true.data);
	auto& qf = get<2>(parsed_false.data);
	return
	  (vertex_evaluator{f.pf, f.first_named()}(qf)==false) +
	2*(vertex_evaluator{f.pf, f.first_named()}(qt)==true)
	;
}}() == 3 );

static_assert( test_fixture{[](auto& f){
	auto parsed = ast_graph::parse_from(f.pf, "{'field_1'=1}");
	auto& qv = get<2>(parsed.data);
	return
	  (vertex_evaluator{f.pf, f.graph.root().base}(qv)==true) +
	2*(vertex_evaluator{f.pf, f.graph.links_of(f.graph.root().base)[0].child}(qv)==false)
	;
}}() == 3, "can find field by name and type if present");
static_assert( test_fixture{[](auto&f){
	auto parsed = ast_graph::parse_from(f.pf, "{'field_1'=1}");
	path_evaluator e{f.pf, &f.graph, f.graph.create_view()};
	e(parsed);
	return e.output.size();
}}() == 1 );
static_assert( test_fixture{[](auto&f){
	f.create_named("test");
	auto parsed = ast_graph::parse_from(f.pf, "{'field_1'=1}+{'test'}");
	path_evaluator e{f.pf, &f.graph, f.graph.create_view()};
	e(parsed);
	return e.output.size();
}}() == 2 );
static_assert( test_fixture{[](auto&f){
	f.create_named("test");
	auto parsed = ast_graph::parse_from(f.pf, "{}-{'test'}");
	path_evaluator e{f.pf, &f.graph, f.graph.create_view()};
	e(parsed);
	return e.output.size();
}}() == 3 );
static_assert( test_fixture{[](auto&f){
	auto parsed = ast_graph::parse_from(f.pf, "{'field_1'=1}->{}");
	path_evaluator e{f.pf, &f.graph, f.graph.create_view()};
	e(parsed);
	return e.output.size();
}}() == 2, "can get node and it's child" );
static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto parsed = ast_graph::parse_from(f.pf, "{'field_1'=1}-->{'test'}");
	auto& qp = get<0>(parsed.data);
	path_evaluator e{f.pf, &f.graph, f.graph.create_view()};
	e(qp);
	return e.output.size();
}}() == 4 );
static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto parsed = ast_graph::parse_from(f.pf, "{'field_1'=1}->{'test'}");
	auto& qp = get<0>(parsed.data);
	path_evaluator e{f.pf, &f.graph, f.graph.create_view()};
	e(qp);
	return e.output.size();
}}() == 0 );
static_assert( test_fixture{[](auto& f){
	f.create_named("test");
	auto parsed = ast_graph::parse_from(f.pf, "{'field_1'=1}-['leafs']->{}-2->{'test'}");
	auto& qp = get<0>(parsed.data);
	path_evaluator e{f.pf, &f.graph, f.graph.create_view()};
	e(qp);
	return e.output.size();
}}() == 4 );

int main(int,char**) {
}
