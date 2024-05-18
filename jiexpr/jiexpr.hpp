#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>
#include <concepts>
#include <cassert>

#include "jiexpr/details.hpp"

template< typename data_type, typename operators_factory, typename data_factory >
struct jiexpr {
	template<typename... types> using variant_t = typename data_factory::template variant_t<types...>;
	using self_type = jiexpr<data_type, operators_factory, data_factory>;
	using operators_executer = operators_factory;

	template<typename expr_t>
	struct binary_op {
		std::decay_t<expr_t> left;
		std::decay_t<expr_t> right;
	};
	template<typename expr_t>
	struct ternary_op {
		std::decay_t<expr_t> cond;
		std::decay_t<expr_t> left;
		std::decay_t<expr_t> right;
	};
	template<typename expr_t> struct op_division : binary_op<expr_t> {};
	template<typename expr_t> struct op_multiply : binary_op<expr_t> {};
	template<typename expr_t> struct op_fp_div   : binary_op<expr_t> {};
	template<typename expr_t> struct op_subtract : binary_op<expr_t> {};
	template<typename expr_t> struct op_addition : binary_op<expr_t> {};
	template<typename expr_t> struct op_concat   : binary_op<expr_t> {};
	template<typename expr_t> struct op_power    : binary_op<expr_t> {};

	template<typename expr_t> struct op_ceq      : binary_op<expr_t> {};
	template<typename expr_t> struct op_neq      : binary_op<expr_t> {};
	template<typename expr_t> struct op_gt       : binary_op<expr_t> {};
	template<typename expr_t> struct op_lt       : binary_op<expr_t> {};
	template<typename expr_t> struct op_get      : binary_op<expr_t> {};
	template<typename expr_t> struct op_let      : binary_op<expr_t> {};
	template<typename expr_t> struct op_in       : binary_op<expr_t> {};

	template<typename expr_t> struct op_and      : binary_op<expr_t> {};
	template<typename expr_t> struct op_or       : binary_op<expr_t> {};

	template<typename expr_t> struct op_not { std::decay_t<expr_t> expr; };

	template<typename expr_t>
	struct list_expr {
		decltype(std::declval<data_factory>().template mk_vec<expr_t>()) list;
	};
	template<typename expr_t>
	struct dict_expr {
		decltype(std::declval<data_factory>().template mk_vec<expr_t>()) names;
		decltype(std::declval<data_factory>().template mk_vec<expr_t>()) values;
	};


	using string_t = typename data_type::string_t;
	using integer_t = typename data_type::integer_t;
	using float_point_t = typename data_type::float_point_t;

	template<typename expr_t>
	struct var_expr {
		decltype(std::declval<data_factory>().template mk_vec<expr_t>()) path;
	};

	struct op_eq_tag{};
	template<typename expr_t> struct op_eq : op_eq_tag {
		constexpr op_eq() =default ;
		constexpr op_eq(var_expr<expr_t> name, expr_t value) : name(std::move(name)), value(std::move(value)) {}
		var_expr<expr_t> name;
		expr_t value;
	};

	template<typename expr_t>
	struct fnc_call_expr {
		var_expr<expr_t> name;
		decltype(std::declval<data_factory>().template mk_vec<expr_t>()) params;
	};

	template<typename expr_t>
	struct apply_filter_expr {
		std::decay_t<expr_t> object;
		variant_t<fnc_call_expr<expr_t>, var_expr<expr_t>> filter;
	};

	template<typename expr_t>
	struct is_test_expr {
		std::decay_t<expr_t> object;
		variant_t<fnc_call_expr<expr_t>, var_expr<expr_t>> test;
	};

	template<typename type> using ast_forwarder = typename data_factory::template ast_forwarder<type>;

	template<typename... operators>
	using parse_result = variant_t<std::decay_t<operators>...,string_t,integer_t,float_point_t,bool>;

	template<template<class>class fa> struct expr_type : parse_result<
	       ternary_op< fa<expr_type<fa>> >
	     , apply_filter_expr< fa<expr_type<fa>> >
	     , variant_t< op_and< fa<expr_type<fa>> >, op_or< fa<expr_type<fa>> > >
	     , variant_t
	        < op_ceq<fa<expr_type<fa>>>, op_neq<fa<expr_type<fa>>>, op_lt<fa<expr_type<fa>>>
	        , op_gt<fa<expr_type<fa>>>, op_get<fa<expr_type<fa>>>, op_let<fa<expr_type<fa>>>
	        , op_in<fa<expr_type<fa>>>, is_test_expr<fa<expr_type<fa>>>
	        >
	     , variant_t< op_subtract < fa<expr_type<fa>> >, op_addition< fa<expr_type<fa>> >, op_concat< fa<expr_type<fa>> > >
	     , variant_t< op_multiply < fa<expr_type<fa>> >, op_division< fa<expr_type<fa>> >, op_fp_div< fa<expr_type<fa>> > >
	     , op_power    < fa<expr_type<fa>> >
	     , op_not      < fa<expr_type<fa>> >
	     , list_expr   < fa<expr_type<fa>> >
	     , dict_expr   < fa<expr_type<fa>> >
	     , var_expr    < fa<expr_type<fa>> >
	     , fnc_call_expr < fa<expr_type<fa>> >
	     , op_eq       < fa<expr_type<fa>> >
	> {};

	data_type* env;
	operators_factory ops;
	data_factory df;

	template<template<class>class fa> constexpr data_type operator()(const expr_type<fa>& e) {
		return visit(*this, e);
		//return jiexpr_details::inner_visit(*this,e);
	}
	constexpr data_type operator()(string_t v) const { return data_type{ v }; }
	constexpr data_type operator()(integer_t v) const { return data_type{ v }; }
	constexpr data_type operator()(float_point_t v) const { return data_type{ v }; }
	constexpr data_type operator()(bool v) const { return data_type{ v }; }
	template<typename... types> constexpr data_type operator()(const op_concat<types...>& op) const ;
	template<typename... types> constexpr data_type operator()(const op_eq<types...>& op) const ;
	template<typename... types> constexpr data_type operator()(const list_expr<types...>& op) const ;
	template<typename... types> constexpr data_type operator()(const dict_expr<types...>& op) const ;
	template<typename... types> constexpr data_type operator()(const var_expr<types...>& op) const ;
	template<typename... types> constexpr data_type operator()(const fnc_call_expr<types...>& op) const ;
	template<typename... types> constexpr data_type operator()(const ternary_op<types...>& op) const ;
	constexpr data_type operator()(const auto& op) const ;

	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr auto create_parser() const ;

	/*
	 * - . and [] operators after literal
	 */
	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr auto parse_str(auto&& src) const {
		expr_type<ast_forwarder> r;
		parse(create_parser<gh,th>(), +gh::space, gh::make_source(src), r);
		return r;
	}
private:
	constexpr void mk_params(auto& params, const auto& op) const ;
};

#include "jiexpr/parser.ipp"
#include "jiexpr/evaluators.ipp"
