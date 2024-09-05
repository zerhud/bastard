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

template<typename factory>
struct query_vertex {
	using string_t = factory::string_type;
	template<typename type> using vec = decltype(mk_vec<type>(factory{}));
	template<typename... list> using variant = factory::template variant<list...>;

	struct expr;
	using fwd_ast = factory::template forward_ast<expr>;

	struct ident { string_t val; };
	struct unary { fwd_ast left; };
	struct binary{ fwd_ast left; fwd_ast right; };
	struct eq : binary {};
	struct neq : binary {};
	struct _and : binary {};
	struct _or : binary {};
	struct _not : unary {};
	struct name_eq : unary {};
	struct type_name_eq : binary {};
	struct in {
		fwd_ast name;
		vec<fwd_ast> values;
	};

	struct expr : variant<
		  variant< eq, neq, _and, _or >
		, in
		, type_name_eq
		, name_eq
		, _not
		, ident
		, string_t
		, typename factory::integer_type
		, typename factory::float_point_type
		, bool
	> {};

	int arg_number=0;
	expr data;

	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr static auto mk_parser(const auto& df) {
		auto mk_fwd = [&df](auto& v){ return df.mk_fwd(v); };
		constexpr auto ident =
				lexeme(gh::alpha++ >> --(*(gh::alpha | gh::d10 | th<'_'>::char_)))
				- (gh::template lit<"and"> | gh::template lit<"in"> | gh::template lit<"or"> | gh::template lit<"true"> | gh::template lit<"false">);
		auto expr_parser = rv([&df](auto& v){ return df.mk_result(std::move(v)); }
				,( cast<binary>( gh::rv_lreq >> gh::template lit<"=="> >> ++gh::rv_rreq(mk_fwd) )
				|  cast<binary>( gh::rv_lreq >> gh::template lit<"!="> >> ++gh::rv_rreq(mk_fwd) )
				|  cast<binary>( gh::rv_lreq >> gh::template lit<"&&"> >> ++gh::rv_rreq(mk_fwd) )
				|  cast<binary>( gh::rv_lreq >> gh::template lit<"||"> >> ++gh::rv_rreq(mk_fwd) )
				)
				, cast<in>( gh::rv_lreq >> gh::template lit<"in"> >> ++th<'('>::_char >> (gh::rv_rreq(mk_fwd) % ',') >> th<')'>::_char )
				, cast<binary>( gh::rv_lreq >> th<':'>::_char >> ++gh::rv_rreq(mk_fwd) )
				, cast<unary>( th<':'>::_char++ >> --gh::rv_rreq(mk_fwd) )
				, cast<unary>( th<'!'>::_char++ >> --gh::rv_rreq(mk_fwd) )
				, ident
				, gh::quoted_string
				, gh::int_
				, gh::fp
				, (as<true>(gh::template lit<"true">)|as<false>(gh::template lit<"false">))
				, rv_result(th<'('>::_char >> gh::rv_req >> th<')'>::_char)
		);
		return -gh::int_ >> ++th<'{'>::_char >> -expr_parser >> th<'}'>::_char;
	}
};

template<typename factory>
struct query_graph {
	template<typename type> using vec = decltype(mk_vec<type>(factory{}));
	template<typename... list> using variant = factory::template variant<list...>;

	struct expr;
	using fwd_ast = factory::template forward_ast<expr>;

	using qvertex = query_vertex<factory>;

	struct unary { fwd_ast left; };
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
	constexpr static auto mk_parser(const auto& df) {
		auto mk_fwd = [&df](auto& v){ return df.mk_fwd(v); };
		constexpr auto ident =
				lexeme(gh::alpha++ >> --(*(gh::alpha | gh::d10 | th<'_'>::char_)))
				- (gh::template lit<"and"> | gh::template lit<"in"> | gh::template lit<"or"> | gh::template lit<"true"> | gh::template lit<"false">);
		return rv([&df](auto& v){ return df.mk_result(std::move(v)); }
			, check<path>( gh::rv_lreq++ >> +(query_edge<factory>::template mk_parser<gh>() >> ++gh::rv_rreq(mk_fwd)) )
			, ( cast<binary>( gh::rv_lreq++ >> th<'+'>::_char >> gh::rv_rreq(mk_fwd) )
			  | cast<binary>( gh::rv_lreq++ >> th<'-'>::_char >> gh::rv_rreq(mk_fwd) )
			  )
			, check<qvertex>(qvertex::template mk_parser<gh>(df))
			, rv_result(th<'('>::_char >> gh::rv_req >> th<')'>::_char)
		);
	}
};

} // namespace details

} // namespace ast_graph
