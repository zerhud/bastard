/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <utility>
#include <tref.hpp>
#include "mk_children_types.hpp"
#include "node.hpp"

namespace ast_graph {

namespace details{
template<typename type, typename... types> struct index_of;
template<typename type, typename... types> struct index_of<type, type, types...> : std::integral_constant<unsigned, 0> {};
template<typename type, typename cur, typename... types> struct index_of<type, cur, types...> : std::integral_constant<unsigned, 1+index_of<type, types...>::value> {};
template<typename type, typename... types> constexpr auto index_of_v = index_of<type, types...>::value;
}


namespace details{

// eq neq and not in ()
// (a:b && c:d) || ((d):())
// 0{} -[]-> (3!{a=b} -0-> {b>a})

template<typename factory> struct query_ident { typename factory::string_type val; };
template<typename expr> struct query_unary { expr val; };
template<typename expr> struct query_binary { expr left; expr right; };
template<typename expr> struct query_eq : query_binary<expr> { };
template<typename expr> struct query_neq : query_binary<expr> { };
template<typename expr> struct query_and : query_binary<expr> { };
template<typename expr> struct query_or : query_binary<expr> { };
template<typename expr> struct query_not : query_unary<expr> { };
template<typename expr> struct query_name_eq : query_unary<expr> { };
template<typename expr, typename factory> struct query_in {
	template<typename type> using vec = decltype(mk_vec<type>(factory{}));
	expr name;
	vec<expr> values;
};

template<typename factory> struct query_expr;
template<typename factory> using fwd_ast = typename factory::template forward_ast<query_expr<factory>>;
template<typename factory>
struct query_expr : factory::template variant<
		  typename factory::template variant<
		  query_eq <fwd_ast<factory>>
		, query_neq<fwd_ast<factory>>
		, query_and<fwd_ast<factory>>
		, query_or<fwd_ast<factory>>
		>
		, query_in <fwd_ast<factory>, factory>
		, query_not<fwd_ast<factory>>
		, query_name_eq<fwd_ast<factory>>
		, query_ident<factory>
		, typename factory::string_type, typename factory::integer_type, typename factory::float_point_type, bool
		>
{
};

template<typename factory>
struct query_vertex {
	query_expr<factory> data;
};
template<typename factory>
struct query_edge {
	using string_t = typename factory::string_t;
	int stop_on_match = -1;
	int max_deep = 1;
	string_t name;
};
template<typename factory>
struct query_path {
	using vertex_type = query_vertex<factory>;
	struct path_element {
		query_edge<factory> edge;
		vertex_type to;
	};
	using tail_type = std::decay_t<decltype(mk_vec<path_element>(std::declval<factory>()))>;

	vertex_type base;
	tail_type tail;
};

template<typename factory>
struct query_path_plus {
	query_path<factory> left;
	query_path<factory> right;
};
template<typename factory>
struct query_path_minus {
	query_path<factory> left;
	query_path<factory> right;
};

template<typename factory>
struct query2 {
	template<typename... types> using variant = typename factory::template variant<types...>;
	template<typename type> using forward_ast = typename factory::template forward_ast<type>;
	using self_forward = forward_ast<query2>;

	factory f;
	variant<query_path_plus<factory>, query_path_minus<factory>, query_path<factory>, self_forward> data;
};

template<typename factory>
struct query {
	template<typename... types> using variant = typename factory::template variant<types...>;
	template<typename type> using forward_ast = typename factory::template forward_ast<type>;
	template<typename type> using optional = typename factory::template optional<type>;

	using ident_type = query_ident<factory>;
	using name_eq_type = query_name_eq<fwd_ast<factory>>;

	using self_forward = forward_ast<query>;

	int input_number = 0; // the number before {
	bool to_output = true; // ! == false, nothing == true
	variant<query_vertex<factory>, unsigned int, query_edge<factory>, self_forward> data;
	//      {}                     -0->          -[]->                ({} --> {})
	//                             -> == -0->
	self_forward next;
};

template<typename factory, typename gh, template<auto>class th=gh::template tmpl>
constexpr auto make_query_parser(const factory& df) {
	auto mk_fwd = [&df](auto& v){ return df.mk_fwd(v); };
	using string_t = typename factory::string_t;
	using expr_type = fwd_ast<factory>;
	using binary = query_binary<expr_type>;
	constexpr auto ident =
			lexeme(gh::alpha++ >> --(*(gh::alpha | gh::d10 | th<'_'>::char_)))
			- (gh::template lit<"and"> | gh::template lit<"in"> | gh::template lit<"or"> | gh::template lit<"true"> | gh::template lit<"false">);
	constexpr auto query_edge_name_parser = lexeme(gh::alpha >> *(gh::alpha | gh::d10 | th<'_'>::char_)) | gh::quoted_string;
	auto query_expr = rv([&df](auto& v){ return df.mk_result(std::move(v)); }
			,(cast<binary>(gh::rv_lreq >> (gh::template lit<"==">|th<':'>::_char) >>  ++gh::rv_rreq(mk_fwd))
			| cast<binary>(gh::rv_lreq >> gh::template lit<"!="> >>  ++gh::rv_rreq(mk_fwd))
			| cast<binary>(gh::rv_lreq >> (gh::template lit<"and">|gh::template lit<"&&">) >> ++gh::rv_rreq(mk_fwd))
			| cast<binary>(gh::rv_lreq >> (gh::template lit<"or">|gh::template lit<"||">) >>  ++gh::rv_rreq(mk_fwd))
			 )
			, cast<query_in <expr_type, factory>>(gh::rv_lreq >> gh::template lit<"in"> >>  ++(gh::rv_rreq(mk_fwd) % ',') )
			, cast<query_unary<expr_type       >>(th<'!'>::_char++ >> --gh::rv_rreq(mk_fwd))
			, cast<query_unary<expr_type       >>(th<':'>::_char++ >> --gh::rv_rreq(mk_fwd))
			, ident
			, gh::quoted_string
			, gh::int_
			, gh::fp
			, (as<true>(gh::template lit<"true">)|as<false>(gh::template lit<"false">))
			, rv_result(th<'('>::_char >> gh::rv_req >> th<')'>::_char)
	);

	auto vertex_parser = (th<'{'>::_char >> th<'}'>::_char) | (th<'{'>::_char >> query_expr >> th<'}'>::_char);
	auto edge_parser =
		 ( as<-1>(th<'-'>::_char)++ >> as<1>(th<'['>::_char)++ >> query_edge_name_parser >> gh::template lit<"]->"> )
		|( th<'-'>::_char >> th<'['>::_char >> -gh::int_ >> th<':'>::_char++ >> -gh::int_ >> th<':'>::_char++ >> query_edge_name_parser >> gh::template lit<"]->"> )
		;
	return th<';'>::_char++ >> 
	rv([&df](auto& v){return df.mk_result(std::move(v));}
	//TODO: add {} in ({} + {}) parser as more priority
	 //TODO: use simple variant for + and - theay are same priority
	 , cast<query_path_plus<factory>>(++gh::rv_lreq >> th<'+'>::_char >> ++gh::rv_rreq(mk_fwd))
	 , cast<query_path_minus<factory>>(++gh::rv_lreq >> th<'-'>::_char >> ++gh::rv_rreq(mk_fwd))
	 , cast<query_path<factory>>( vertex_parser++ >> *(edge_parser++ >> vertex_parser) )
	 //, rv_result(th<'('>::_char++ >> gh::rv_req >> th<')'>::_char)
	 )
	;
	/*
	return
	(-gh::int_)++ >> (-as<false>(th<'!'>::_char))++ >>
	( cast<query_vertex<factory>>((th<'{'>::_char++ >> --th<'}'>::_char) | (th<'{'>::_char >> query_expr++ >> --th<'}'>::_char))
	| (as<0>(gh::template lit<"->">) | (th<'-'>::_char >> gh::int_ >> gh::template lit<"->">))
	| check<query_edge<factory>>(
		 (as<-1>(th<'-'>::_char)++ >> as<1>(th<'['>::_char)++ >> query_edge_name_parser >> gh::template lit<"]->">)
		|( th<'-'>::_char >> th<'['>::_char >> -gh::int_ >> th<':'>::_char++ >> -gh::int_ >> th<':'>::_char++ >> query_edge_name_parser >> gh::template lit<"]->">)
		)
	)++ >> -th<0>::req([]<typename type>(type& v){v.reset(new typename type::element_type{});return v.get();})
	//TODO: use factory instead of new in req semact
	;
	*/
}

template<typename gh, typename factory>
constexpr auto parse_query(const factory& f, auto&& src) {
	query2<factory> result{ f };
	auto parsed_sz = parse(make_query_parser<factory, gh>(f), +gh::space, gh::make_source(std::forward<decltype(src)>(src)), result);
	return result;
}

} // namespace details

constexpr auto query(const auto& f, const auto& graph, auto&& _q) {
	/*
	auto q = details::parse_query(f, std::forward<decltype(_q)>(_q));
	for(auto& node:graph);
	*/
	return graph;
	/*
	auto result = details::mk_empty_result(f, mk_children_types(f, source));
	details::fill_with_all_data(result, f, source, details::add_vertex(result, source, f));
	result.root = &result.vertexes[0];
	return result;
	*/
}

} // namespace ast_graph
