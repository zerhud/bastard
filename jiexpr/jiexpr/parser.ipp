#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

template< typename data_factory >
template<typename gh>
constexpr auto jiexpr<data_factory>::create_parser() const {
	using result_t = expression_type<ast_forwarder>;
	using expr_t = ast_forwarder<expression_type<ast_forwarder>>;
	return _create_parser<gh, result_t, expr_t>();
}

template< typename data_factory >
template<typename gh, typename result_t, typename expr_t, template<auto>class th>
constexpr auto jiexpr<data_factory>::_create_parser() const {
	auto fwd = [this](auto& v){ return mk_fwd(df, v); };
	auto fwd_emp = [this](auto& v){ return mk_fwd(df, v.emplace_back()); };
	//TODO: initialize string (ident, quoted_string, array) and vectors (array only) with data factory
	constexpr auto ident =
			lexeme(gh::alpha >> *(gh::alpha | gh::d10 | th<'_'>::char_))([](auto& v){return &create<string_t>(v);})
			- (gh::template lit<"and"> | gh::template lit<"is"> | gh::template lit<"in"> | gh::template lit<"or">);
	auto var_expr_mk_result = [this](auto& v){result_t r; return v.path.emplace_back(mk_result(df, std::move(r))).get();};
	auto var_expr_parser = cast<var_expr<expr_t>>(ident(var_expr_mk_result) >> *((th<'.'>::_char >> ident(var_expr_mk_result)) | (th<'['>::_char >> th<0>::rv_rec(var_expr_mk_result) >> th<']'>::_char)));
	auto fnc_call = var_expr_parser++ >> -(th<'('>::_char >> -(gh::rv_rrec(fwd) % ',') >> th<')'>::_char);
	return rv([this](auto& v){ return mk_result(df, std::move(v)); }
			,    ++gh::rv_lrec
			  >> lexeme(omit(gh::template lit<"if"> >> +gh::space)) >> fnum<0>(gh::rv_rrec(fwd))
			  >> fnum<2>(-(lexeme(omit(gh::template lit<"else"> >> +gh::space)) >> gh::rv_rrec(fwd)))
			, gh::rv_lrec >> th<'|'>::_char++ >> cast<common_fnc_expr<expr_t>>(fnc_call)
			, cast<binary_op<expr_t>>(gh::rv_lrec >> lexeme(omit(gh::template lit<"and"> >> +gh::space)) >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> lexeme(omit(gh::template lit<"or"> >> +gh::space))  >> ++gh::rv_rrec(fwd))
			, cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<"=="> >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<"!="> >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<"<"> >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<">"> >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<">="> >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<"<="> >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> lexeme(omit(gh::template lit<"in"> >> +gh::space)) >> ++gh::rv_rrec(fwd))
			  | cast<is_test_expr<expr_t>>(gh::rv_lrec >> lexeme(omit(gh::template lit<"is"> >> +gh::space))++ >> cast<common_fnc_expr<expr_t>>(fnc_call))
			, cast<binary_op<expr_t>>(gh::rv_lrec >> th<'-'>::_char >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> th<'+'>::_char >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> th<'~'>::_char >> ++gh::rv_rrec(fwd))
			, cast<binary_op<expr_t>>(gh::rv_lrec >> th<'*'>::_char >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<"//"> >> ++gh::rv_rrec(fwd))
			  | cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<"/"> >> ++gh::rv_rrec(fwd))
			, cast<binary_op<expr_t>>(gh::rv_lrec >> gh::template lit<"**"> >> ++gh::rv_rrec(fwd))
			, cast<op_not<expr_t>>(th<'!'>::_char++ >> --gh::rv_rrec(fwd))
			, th<'['>::_char >> fnum<0>(-((th<0>::rv_rec(fwd)) % ',')) >> th<']'>::_char
			, th<'{'>::_char >> th<'}'>::_char | th<'{'>::_char >> (th<0>::rv_rec(fwd_emp)++ >> th<':'>::_char >> th<0>::rv_rec(fwd_emp)) % ',' >> th<'}'>::_char
			, var_expr_parser
			, cast<fnc_call_expr<expr_t>>(var_expr_parser++ >> th<'('>::_char >> -(gh::rv_rrec(fwd) % ',') >> th<')'>::_char)
			, cast<op_eq<expr_t>>(var_expr_parser >> th<'='>::_char >> ++th<0>::rv_rec(fwd))
			, gh::quoted_string
			, gh::int_
			, gh::fp
			, (as<true>(gh::template lit<"true">)|as<false>(gh::template lit<"false">))
			, rv_result(th<'('>::_char >> th<0>::rv_rec >> th<')'>::_char)
	);
}
