#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

template< typename data_type, typename operators_factory, typename data_factory >
template<typename gh, template<auto>class th>
constexpr auto jiexpr<data_type,operators_factory,data_factory>::create_parser() const {
	using result_t = expr_type<ast_forwarder>;
	using expr_t = ast_forwarder<expr_type<ast_forwarder>>;
	auto mk_fwd = [this](auto& v){ return df.mk_fwd(v); };
	auto mk_fwd_emp = [this](auto& v){ return df.mk_fwd(v.emplace_back()); };
	//TODO: initialize string (ident, quoted_string, array) and vectors (array only) with data factory
	constexpr auto ident =
			lexeme(gh::alpha >> *(gh::alpha | gh::d10 | th<'_'>::char_))([](auto& v){return &create<string_t>(v);})
			- (gh::template lit<"and"> | gh::template lit<"is"> | gh::template lit<"in"> | gh::template lit<"or">);
	auto var_expr_mk_result = [this](auto& v){result_t r; return v.path.emplace_back(df.mk_result(r)).get();};
	auto var_expr_parser = cast<var_expr<expr_t>>(ident(var_expr_mk_result) >> *((th<'.'>::_char >> ident(var_expr_mk_result)) | (th<'['>::_char >> gh::rv_req(var_expr_mk_result) >> th<']'>::_char)));
	auto fnc_call = cast<fnc_call_expr<expr_t>>(var_expr_parser++ >> th<'('>::_char >> -(gh::rv_rreq(mk_fwd) % ',') >> th<')'>::_char);
	return rv([this](auto& v){ return df.mk_result(v); }
			,    ++gh::rv_lreq
			  >> lexeme(omit(gh::template lit<"if"> >> +gh::space)) >> fnum<0>(gh::rv_rreq(mk_fwd))
			  >> fnum<2>(-(lexeme(omit(gh::template lit<"else"> >> +gh::space)) >> gh::rv_rreq(mk_fwd)))
			, gh::rv_lreq >> th<'|'>::_char++ >> (fnc_call | var_expr_parser)
			, cast<binary_op<expr_t>>(gh::rv_lreq >> lexeme(omit(gh::template lit<"and"> >> +gh::space)) >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> lexeme(omit(gh::template lit<"or"> >> +gh::space))  >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"=="> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"!="> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"<"> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<">"> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<">="> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"<="> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> lexeme(omit(gh::template lit<"in"> >> +gh::space)) >> ++gh::rv_rreq(mk_fwd))
			| cast<is_test_expr<expr_t>>(gh::rv_lreq >> lexeme(omit(gh::template lit<"is"> >> +gh::space))++ >> (fnc_call | var_expr_parser))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'-'>::_char >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> th<'+'>::_char >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> th<'~'>::_char >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'*'>::_char >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"//"> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"/"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"**"> >> ++gh::rv_rreq(mk_fwd))
			, cast<op_not<expr_t>>(th<'!'>::_char++ >> --gh::rv_rreq(mk_fwd))
			, th<'['>::_char >> fnum<0>(-((gh::rv_req(mk_fwd)) % ',')) >> th<']'>::_char
			, th<'{'>::_char >> th<'}'>::_char | th<'{'>::_char >> (gh::rv_req(mk_fwd_emp)++ >> th<':'>::_char >> gh::rv_req(mk_fwd_emp)) % ',' >> th<'}'>::_char
			, var_expr_parser
			, cast<fnc_call_expr<expr_t>>(var_expr_parser++ >> th<'('>::_char >> -(gh::rv_rreq(mk_fwd) % ',') >> th<')'>::_char)
			, cast<op_eq<expr_t>>(var_expr_parser >> th<'='>::_char >> ++gh::rv_req(mk_fwd))
			, gh::quoted_string
			, gh::int_
			, gh::fp
			, (as<true>(gh::template lit<"true">)|as<false>(gh::template lit<"false">))
			, rv_result(th<'('>::_char >> gh::rv_req >> th<')'>::_char)
	);
}
