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

#if defined(__clang__) || defined(JIEXPR_ONLY_RT_TESTS)
#define JIEXPR_CTRT(param) assert( param );
#else
#define JIEXPR_CTRT(param) static_assert( param ); assert( param );
#endif

namespace bastard_details {

template<typename... types> struct overloaded : types... { using types::operator()... ;} ;
template<typename... types> overloaded(types...) -> overloaded<types...>;

template<typename type, template<typename...>class tmpl> constexpr const bool is_specialization_of = false;
template<template<typename...>class type, typename... args> constexpr const bool is_specialization_of<type<args...>, type> = true;

template<typename data_type, typename type>
concept data_vector = requires(type& v, const data_type& d){ v.clear(); v.push_back(d); } && requires(const type& v){ { v.at(0) } -> std::same_as<data_type>; } ;

struct expr_operators_simple {
	template<typename data_type>
	constexpr static auto math_op(auto&& l, auto&& r, auto&& op) {
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		if(l.is_int()) {
			if(r.is_float_point()) return data_type{ float_point_t( op((integer_t)l,(float_point_t)r) ) };
			return data_type{ integer_t( op((integer_t)l,(integer_t)r) ) };
		}
		else {
			if(r.is_float_point()) return data_type{ float_point_t( op((float_point_t)l,(float_point_t)r) ) };
			return data_type{ float_point_t( op((float_point_t)l,(integer_t)r) ) };
		}
	}

	template<typename to_type>
	constexpr static auto div(const auto& l, const auto& r) {
		using data_type = std::decay_t<decltype(l)>;
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		if(l.is_int()) {
			if(r.is_float_point()) return data_type{ to_type((integer_t)l / (float_point_t)r) };
			return data_type{ to_type((integer_t)l / (integer_t)r) };
		}
		else {
			if(r.is_float_point()) return data_type{ to_type((float_point_t)l / (float_point_t)r) };
			return data_type{ to_type((float_point_t)l / (integer_t)r) };
		}
	}

	template<typename data_type>
	constexpr static auto to_bool(auto&& val) {
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		using string_t = typename data_type::string_t;
		if( val.is_bool() ) return val;
		else if( val.is_int() ) return data_type{ !!((integer_t)val) };
		else if( val.is_float_point() ) return data_type{ !!((float_point_t)val) };
		else if( val.is_string() ) return data_type{ !((string_t)val).empty() };
		else return data_type{ false };
	}

	template<typename data_type>
	constexpr static auto int_div(auto&& left, auto&& right) {
		return div<typename data_type::integer_t>( left, right );
	}

	template<typename data_type>
	constexpr static auto fp_div(auto&& left, auto&& right) {
		return div<typename data_type::float_point_t>( left, right );
	}

	template<typename data_type>
	constexpr static auto mul(auto&& l, auto&& r) {
		return math_op<data_type>(l,r, [](const auto& l, const auto& r){ return l * r; });
	}

	template<typename data_type>
	constexpr static auto sub(auto&& l, auto&& r) {
		return math_op<data_type>(l,r, [](const auto& l, const auto& r){ return l - r; });
	}

	template<typename data_type>
	constexpr static auto add(auto&& l, auto&& r) {
		return math_op<data_type>(l,r, [](const auto& l, const auto& r){ return l + r; });
	}

	constexpr static auto do_concat(auto&& left, auto&& right) {
		return std::forward<decltype(left)>(left) + std::forward<decltype(right)>(right);
	}

	template<typename data_type>
	constexpr static auto negate(auto&& val) {
		return data_type{ !to_bool<data_type>( std::forward<decltype(val)>(val) ) };
	}
	template<typename data_type>
	constexpr static auto do_and(auto&& left, auto&& right) {
		return data_type{ to_bool<data_type>(left) && to_bool<data_type>(right) };
	}
	template<typename data_type>
	constexpr static auto do_or(auto&& left, auto&& right) {
		return data_type{ to_bool<data_type>(left) || to_bool<data_type>(right) };
	}
	template<typename data_type>
	constexpr static auto do_ceq(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l==r;}{return l==r;});
	}
	template<typename data_type>
	constexpr static auto do_neq(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l!=r;}{return l!=r;});
	}
	template<typename data_type>
	constexpr static auto do_lt(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l<r;}{return l<r;});
	}
	template<typename data_type>
	constexpr static auto do_gt(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l>r;}{return l>r;});
	}
	template<typename data_type>
	constexpr static auto do_let(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l<=r;}{return l<=r;});
	}
	template<typename data_type>
	constexpr static auto do_get(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l>=r;}{return l>=r;});
	}
	template<typename data_type>
	constexpr static auto do_in(data_type&& left, data_type&& right) {
		return right.contains(std::forward<decltype(left)>(left));
	}

	template<typename data_type>
	constexpr static auto pow(auto&& l, auto&& r) {
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		auto right = (integer_t)r;
		if(l.is_int()) {
			auto left = (integer_t)l;
			while(--right > 0) left *= left;
			return data_type{ left };
		}
		else {
			auto left = (float_point_t)l;
			while(--right > 0) left *= left;
			return data_type{ left };
		}
	}
};

} // namespace bastard_details

template< typename data_type, typename operators_factory, typename data_factory >
struct bastard {
	template<typename... types> using variant_t = typename data_factory::template variant_t<types...>;
	using self_type = bastard<data_type, operators_factory, data_factory>;

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
	template<typename expr_t> struct op_multipli : binary_op<expr_t> {};
	template<typename expr_t> struct op_fp_div   : binary_op<expr_t> {};
	template<typename expr_t> struct op_substruct: binary_op<expr_t> {};
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
		constexpr op_eq() {}
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
		variant_t<var_expr<expr_t>, fnc_call_expr<expr_t>> filter;
	};

	template<typename type> using ast_forwarder = typename data_factory::template ast_forwarder<type>;

	template<typename... operators>
	using parse_result = variant_t<std::decay_t<operators>...,string_t,integer_t,float_point_t,bool>;

	template<template<class>class fa> struct expr_type : parse_result<
	       apply_filter_expr< fa<expr_type<fa>> >
	     , variant_t< op_and< fa<expr_type<fa>> >, op_or< fa<expr_type<fa>> > >
	     , variant_t
	        < op_ceq<fa<expr_type<fa>>>, op_neq<fa<expr_type<fa>>>, op_lt<fa<expr_type<fa>>>
	        , op_gt<fa<expr_type<fa>>>, op_get<fa<expr_type<fa>>>, op_let<fa<expr_type<fa>>>, op_in<fa<expr_type<fa>>>
	        >
	     , variant_t< op_substruct< fa<expr_type<fa>> >, op_addition< fa<expr_type<fa>> >, op_concat< fa<expr_type<fa>> > >
	     , variant_t< op_multipli < fa<expr_type<fa>> >, op_division< fa<expr_type<fa>> >, op_fp_div< fa<expr_type<fa>> > >
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
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_division> ) {
			return ops.template int_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_fp_div> ) {
			return ops.template fp_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_multipli> ) {
			return ops.template mul<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_substruct> ) {
			return ops.template sub<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_addition> ) {
			return ops.template add<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_power> ) {
			return ops.template pow<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_not> ) {
			return ops.template negate<data_type>( visit(*this,*op.expr) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_and> ) {
			return ops.template do_and<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_or> ) {
			return ops.template do_or<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_ceq> ) {
			return ops.template do_ceq<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_neq> ) {
			return ops.template do_neq<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_lt> ) {
			return ops.template do_lt<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_gt> ) {
			return ops.template do_gt<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_let> ) {
			return ops.template do_let<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_get> ) {
			return ops.template do_get<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_in> ) {
			return data_type{ ops.template do_in<data_type>( visit(*this,*op.left), visit(*this,*op.right) ) };
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_concat> ) {
			auto left_str = df.mk_str();
			auto right_str = df.mk_str();
			back_insert_format(df.back_inserter(left_str), visit(*this, *op.left));
			back_insert_format(df.back_inserter(right_str), visit(*this, *op.right));
			return data_type{ ops.template do_concat( std::move(left_str), std::move(right_str) ) };
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_eq> ) {
			auto cur = *env;
			auto& [name,value] = op;
			auto& left = name.path;
			for(auto i=0;i<left.size()-1;++i) cur = cur[data_type{get<string_t>(*left[i])}];
			data_type key{get<string_t>(*left[left.size()-1])};
			cur.put(key, visit(*this, *value));
			return cur[key];
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, list_expr> ) {
			data_type ret;
			ret.mk_empty_array();
			for(auto&& item:op.list) ret.push_back(visit(*this, *item));
			return ret;
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, dict_expr> ) {
			data_type ret;
			ret.mk_empty_object();
			for(auto i=0;i<op.names.size();++i)
				ret.put(visit(*this, *op.names[i]), visit(*this, *op.values.at(i)));
			return ret;
		}
		else if constexpr ( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, var_expr> ) {
			//cmpget_workwaroud for constexpr bug with strings
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
		else if constexpr (bastard_details::is_specialization_of<std::decay_t<decltype(op)>, fnc_call_expr>) {
			auto fnc = (*this)(op.name);
			data_type params;
			params.mk_empty_object();
			typename data_type::integer_t ind=0;
			for(auto& param:op.params) {
				if(!jiexpr_details::variant_holds<op_eq_tag>(*param)) params.put(data_type{ind++}, visit(*this, *param));
				else {
					auto& [name,val] = jiexpr_details::get_by_tag<op_eq_tag>(*param);
					params.put(data_type{get<string_t>(*name.path.at(0))}, visit(*this, *val));
				}
			}
			return fnc.call(std::move(params));
		}
		else if constexpr (bastard_details::is_specialization_of<std::decay_t<decltype(op)>, apply_filter_expr>) {
			return data_type{__LINE__};
		}
		else {
			std::unreachable(); // your specialization doesn't work :(
			return data_type{(integer_t) __LINE__};
		}
	}

	/*
	 * - is: performs a test
	 * - . and [] operators after literal
	 * - if operator
	 */
	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr auto parse_str(auto&& src) const {
		using result_t = expr_type<ast_forwarder>;
		using expr_t = ast_forwarder<expr_type<ast_forwarder>>;
		auto mk_fwd = [this](auto& v){ return df.mk_fwd(v); };
		auto mk_fwd_emp = [this](auto& v){ return df.mk_fwd(v.emplace_back()); };
		//TODO: initialize string (ident, quoted_string, array) and vectors (array only) with data factory
		constexpr auto ident = lexeme(gh::alpha >> *(gh::alpha | gh::d10 | th<'_'>::char_))([](auto& v){return &v.template emplace<string_t>();});
		auto var_expr_mk_result = [this](auto& v){result_t r; return v.path.emplace_back(df.mk_result(r)).get();};
		auto var_expr_parser = cast<var_expr<expr_t>>(ident(var_expr_mk_result) >> *((th<'.'>::_char >> ident(var_expr_mk_result)) | (th<'['>::_char >> gh::rv_req(var_expr_mk_result) >> th<']'>::_char)));
		auto fnc_call = cast<fnc_call_expr<expr_t>>(var_expr_parser++ >> th<'('>::_char >> -(gh::rv_rreq(mk_fwd) % ',') >> th<')'>::_char);
		auto expr_p = rv([this](auto& v){ return df.mk_result(v); }
			, gh::rv_lreq >> th<'|'>::_char++ >> (var_expr_parser | fnc_call)
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"and"> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"or">  >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"=="> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"!="> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"<"> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<">"> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<">="> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"<="> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"in"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'-'>::_char >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> th<'+'>::_char >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> th<'~'>::_char >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'*'>::_char >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"//"> >> ++gh::rv_rreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"/"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"**"> >> ++gh::rv_rreq(mk_fwd))
			, cast<unary_op<expr_t>>(th<'!'>::_char++ >> --gh::rv_rreq(mk_fwd))
			, th<'['>::_char++ >> --(-((gh::rv_req(mk_fwd)) % ',')) >> th<']'>::_char
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
		expr_type<ast_forwarder> r;
		parse(expr_p, +gh::space, gh::make_source(src), r);
		return r;
	}

	template<typename gh>
	constexpr static auto test_terms(auto src, auto& env) {
		operators_factory ops;
		bastard ev{&env, ops};
		auto parsed = ev.parse_str<gh>(src);
		return ev(parsed);
	}

	template<typename gh>
	constexpr static auto test_terms(auto src) {
		data_type env;
		return test_terms<gh>(src, env);
	}

};

