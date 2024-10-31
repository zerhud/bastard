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
// {} -[1:2:name]-> {} - match_count:max_deep
//                       max_deep: 0 match nothing, -1 match all (last)
//                       match_count: 0 or omit - all, number - match number

template<typename factory>
struct query_edge {
	using string_t = typename factory::string_t;
	int stop_on_match = -1;
	int max_deep = 1;
	string_t name;

	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr static auto mk_parser() {
		constexpr auto query_edge_name_parser = lexeme(gh::alpha >> *(gh::alpha | gh::d10 | th<'_'>::char_)) | gh::quoted_string;
		return 
		 ( as<-1>(th<'-'>::_char)++ >> as<1>(th<'['>::_char)++ >> query_edge_name_parser >> gh::template lit<"]->"> )
		|( th<'-'>::_char >> th<'['>::_char >> -gh::int_ >> th<':'>::_char++ >> -gh::int_ >> th<':'>::_char++ >> query_edge_name_parser >> gh::template lit<"]->"> )
		|( th<'-'>::_char++ >> as<1>(th<'>'>::_char) )
		|( th<'-'>::_char++ >> as<-1>(th<'-'>::_char) >> th<'>'>::_char )
		;
	}
};

template<typename factory, typename inner_expr>
struct query_vertex {
	using string_t = factory::string_t;
	using integer_t = factory::integer_t;
	using float_point_t = factory::float_point_t;
	template<typename type> using vec = decltype(mk_vec<type>(factory{}));
	template<typename... list> using variant = factory::template variant<list...>;

	using own_literal = variant<integer_t, float_point_t, string_t, bool>;
	struct by_name { string_t name; };
	struct by_type_and_name { string_t type, name; };
	struct by_field_value { string_t field; own_literal value; };
	using own_expr = vec<variant<by_type_and_name, by_field_value, by_name>>;

	using emb_expr = inner_expr::parsed_expression;

	int arg_number=0;
	variant<emb_expr,own_expr> data;

	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr static auto mk_parser(auto&& inner_expr_parser) {
		constexpr auto bool_parser = as<true>(gh::template lit<"true">)|as<false>(gh::template lit<"false">);
		auto ip = create_parser<gh>(inner_expr_parser);
		auto op =
				  (gh::quoted_string >> th<':'>::_char >> ++gh::quoted_string)
				| (gh::quoted_string >> th<'='>::_char >> ++(gh::int_ | gh::fp | gh::quoted_string | bool_parser))
				| (gh::nop >> fnum<0>(gh::quoted_string))
				;
		constexpr auto own_expr_begin = th<'{'>::_char;
		constexpr auto own_expr_end = th<'}'>::_char;
		constexpr auto inner_expr_begin = gh::template lit<"{{">;
		constexpr auto inner_expr_end = gh::template lit<"}}">;
		auto braced_inner_expr = inner_expr_begin  >> (
				  use_variant_result(inner_expr_end([](auto,auto& v){ create<bool>(v) = true; }))
				| use_variant_result( ip >> inner_expr_end )
				);
		auto braced_own_expr = own_expr_begin >> (
				  use_variant_result( own_expr_end )
				| use_variant_result( op%',' >> own_expr_end )
				);
		return -gh::int_ >> ++(braced_inner_expr|braced_own_expr);
	}
};

template<typename factory, typename vertex_expr>
struct query_graph {
	template<typename type> using vec = decltype(mk_vec<type>(factory{}));
	template<typename... list> using variant = factory::template variant<list...>;

	struct expr;
	using fwd_ast = factory::template ast_forwarder<expr>;

	using qvertex = query_vertex<factory, vertex_expr>;

	struct binary{ fwd_ast left; fwd_ast right; };
	struct path_end {
		query_edge<factory> edge;
		fwd_ast vertex;
	};
	struct path {
		fwd_ast start;
		vec<path_end> tail;
	};
	struct add : binary {};
	struct sub : binary {};

	struct expr : variant<
		  path
		, variant< add, sub >
		, qvertex
	> {} ;

	expr data;

	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr static auto mk_parser(const auto& df, const auto& vertex_solver) {
		auto fwd = [&df](auto& v){ return mk_fwd(df, v); };
		return rv([&df](auto& v){ return mk_result(df, std::move(v)); }
			, check<path>( gh::rv_lreq++ >> +(query_edge<factory>::template mk_parser<gh>() >> ++gh::rv_rreq(fwd)) )
			, ( cast<binary>( gh::rv_lreq++ >> th<'+'>::_char >> gh::rv_rreq(fwd) )
			  | cast<binary>( gh::rv_lreq++ >> th<'-'>::_char >> gh::rv_rreq(fwd) )
			  )
			, check<qvertex>(qvertex::template mk_parser<gh>(vertex_solver))
			, rv_result(th<'('>::_char >> gh::rv_req >> th<')'>::_char)
		);
	}
};

} // namespace details

template<typename parser_factory>
constexpr auto parse_from(const parser_factory& pf, auto&& src) {
	//TODO: throws on parser error
	using vertex_expr = std::decay_t<decltype(vertex_expression(std::declval<parser_factory>()))>;
	using parser = parser_factory::parser;
	details::query_graph<parser_factory, vertex_expr> result;
	const auto& vs = vertex_expression(pf);
	parse(
			result.template mk_parser<parser>(pf, vs),
			+parser::space,
			parser::make_source(std::forward<decltype(src)>(src)),
			result.data);
	return result;
}

} // namespace ast_graph
