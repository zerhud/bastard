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
struct bastard {
	template<typename... types> using variant_t = typename data_factory::template variant_t<types...>;
	using self_type = bastard<data_type, operators_factory, data_factory>;
	using operators_executer = operators_factory;

	template<typename expr_t>
	struct unary_op { std::decay_t<expr_t> expr; };
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
	template<typename expr_t> struct op_not      : unary_op<expr_t> {};

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

	constexpr void mk_params(auto& params, const auto& op) const {
		typename data_type::integer_t ind=params.size();
		for(auto& param:op.params) {
			if(!jiexpr_details::variant_holds<op_eq_tag>(*param)) params.put(data_type{ind++}, visit(*this, *param));
			else {
				auto& [name,val] = jiexpr_details::get_by_tag<op_eq_tag>(*param);
				params.put(data_type{get<string_t>(*name.path.at(0))}, visit(*this, *val));
			}
		}
	}

	template<template<class>class fa> constexpr data_type operator()(const expr_type<fa>& e) {
		return visit(*this, e);
	}
	constexpr data_type operator()(string_t v) const { return data_type{ v }; }
	constexpr data_type operator()(integer_t v) const { return data_type{ v }; }
	constexpr data_type operator()(float_point_t v) const { return data_type{ v }; }
	constexpr data_type operator()(bool v) const { return data_type{ v }; }
	constexpr data_type operator()(const auto& op) const {
		if constexpr (requires{!op.left;}) if(!op.left) std::unreachable();
		if constexpr (requires{!op.right;}) if(!op.right) std::unreachable();
		if constexpr (requires{visit([](const auto&){}, op);}) {
			return visit(*this, op);
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_division> ) {
			return ops.template int_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_fp_div> ) {
			return ops.template fp_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_multiply> ) {
			return ops.template mul<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_subtract > ) {
			return ops.template sub<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_addition> ) {
			return ops.template add<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_power> ) {
			return ops.template pow<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_not> ) {
			return ops.template negate<data_type>( visit(*this,*op.expr) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_and> ) {
			return ops.template do_and<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_or> ) {
			return ops.template do_or<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_ceq> ) {
			return ops.template do_ceq<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_neq> ) {
			return ops.template do_neq<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_lt> ) {
			return ops.template do_lt<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_gt> ) {
			return ops.template do_gt<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_let> ) {
			return ops.template do_let<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_get> ) {
			return ops.template do_get<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_in> ) {
			return data_type{ ops.template do_in<data_type>( visit(*this,*op.left), visit(*this,*op.right) ) };
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_concat> ) {
			auto left_str = df.mk_str();
			auto right_str = df.mk_str();
			back_insert_format(df.back_inserter(left_str), visit(*this, *op.left));
			back_insert_format(df.back_inserter(right_str), visit(*this, *op.right));
			return data_type{ ops.template do_concat( std::move(left_str), std::move(right_str) ) };
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_eq> ) {
			auto cur = *env;
			auto& left = op.name.path;
			for(auto i=0;i<left.size()-1;++i) cur = cur[data_type{get<string_t>(*left[i])}];
			data_type key{get<string_t>(*left[left.size()-1])};
			cur.put(key, visit(*this, *op.value));
			return cur[key];
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, list_expr> ) {
			data_type ret;
			ret.mk_empty_array();
			for(auto&& item:op.list) ret.push_back(visit(*this, *item));
			return ret;
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, dict_expr> ) {
			data_type ret;
			ret.mk_empty_object();
			for(auto i=0;i<op.names.size();++i)
				ret.put(visit(*this, *op.names[i]), visit(*this, *op.values.at(i)));
			return ret;
		}
		else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, var_expr> ) {
			auto cur = (*env)[data_type{get<string_t>(*op.path.at(0))}];
			for(auto pos = ++op.path.begin();pos!=op.path.end();++pos) {
				auto& item = **pos;
				if(holds_alternative<string_t>(item)) cur = cur[data_type{get<string_t>(item)}];
				else {
					auto key = visit(*this, item);
					if(key.is_int()) cur = cur[(integer_t)key];
					else cur = cur[key];
				}
			}
			return cur;
		}
		else if constexpr (jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, fnc_call_expr>) {
			auto fnc =(*this)(op.name);
			data_type params;
			params.mk_empty_object();
			mk_params(params, op);
			return fnc.call(std::move(params));
		}
		else if constexpr (
				jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, apply_filter_expr>
			|| 	jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, is_test_expr>
				        ) {
			data_type params;
			params.put(data_type{0}, visit(*this, *op.object));
			auto ret = visit([this,&params](const auto& op){
				if constexpr (requires{op.path;}) {
					return (*this)(op).call(params);
				} else {
					auto fnc = (*this)(op.name);
					mk_params(params, op);
					return fnc.call(std::move(params));
				}
			}, [&op]->auto& {
				if constexpr(requires{op.filter;}) return op.filter;
				else return op.test;
			}());
			if constexpr (requires{op.test;}) return ops.template to_bool<data_type>(ret);
			else return ret;
		}
		else if constexpr (jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, ternary_op>) {
			if(ops.template to_bool<data_type>(visit(*this,*op.cond))) return visit(*this,*op.left);
			if(op.right) return visit(*this, *op.right);
			return data_type{};
		}
		else {
			op.wasnt_specialized();
			//std::unreachable(); // your specialization doesn't work :(
			return data_type{(integer_t) __LINE__};
		}
	}

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
};

#include "jiexpr/parser.ipp"
