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
			if(r.is_fp()) return data_type{ float_point_t( op((integer_t)l,(float_point_t)r) ) };
			return data_type{ integer_t( op((integer_t)l,(integer_t)r) ) };
		}
		else {
			if(r.is_fp()) return data_type{ float_point_t( op((float_point_t)l,(float_point_t)r) ) };
			return data_type{ float_point_t( op((float_point_t)l,(integer_t)r) ) };
		}
	}

	template<typename to_type>
	constexpr static auto div(const auto& l, const auto& r) {
		using data_type = std::decay_t<decltype(l)>;
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		if(l.is_int()) {
			if(r.is_fp()) return data_type{ to_type((integer_t)l / (float_point_t)r) };
			return data_type{ to_type((integer_t)l / (integer_t)r) };
		}
		else {
			if(r.is_fp()) return data_type{ to_type((float_point_t)l / (float_point_t)r) };
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
		else if( val.is_fp() ) return data_type{ !!((float_point_t)val) };
		else if( val.is_str() ) return data_type{ !((string_t)val).empty() };
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
	template<typename... types> using variant_t = typename data_type::template variant_t<types...>;
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
	template<typename expr_t> struct op_power    : binary_op<expr_t> {};

	template<typename expr_t> struct op_eq       : binary_op<expr_t> {};
	template<typename expr_t> struct op_neq      : binary_op<expr_t> {};
	template<typename expr_t> struct op_gt       : binary_op<expr_t> {};
	template<typename expr_t> struct op_lt       : binary_op<expr_t> {};
	template<typename expr_t> struct op_get      : binary_op<expr_t> {};
	template<typename expr_t> struct op_let      : binary_op<expr_t> {};

	template<typename expr_t> struct op_and      : binary_op<expr_t> {};
	template<typename expr_t> struct op_or       : binary_op<expr_t> {};
	template<typename expr_t> struct op_not      : unary_op<expr_t> {};

	template<typename expr_t>
	struct list_expr {
		decltype(std::declval<data_factory>().template mk_vec<expr_t>()) list;
	};

	using string_t = typename data_type::string_t;
	using integer_t = typename data_type::integer_t;
	using float_point_t = typename data_type::float_point_t;

	template<typename type> using ast_forwarder = typename data_factory::template ast_forwarder<type>;

	template<typename... operators>
	using parse_result = typename data_type::template variant_t<std::decay_t<operators>...,string_t,integer_t,float_point_t,bool>;

	template<template<class>class fa> struct expr_type : parse_result<
	       op_and      < fa<expr_type<fa>> >
	     , op_or       < fa<expr_type<fa>> >
	     , op_addition < fa<expr_type<fa>> >
	     , op_substruct< fa<expr_type<fa>> >
	     , op_multipli < fa<expr_type<fa>> >
	     , op_division < fa<expr_type<fa>> >
	     , op_fp_div   < fa<expr_type<fa>> >
	     , op_power    < fa<expr_type<fa>> >
	     , op_not      < fa<expr_type<fa>> >
	     , list_expr   < fa<expr_type<fa>> >
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
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_division> ) {
		if(!op.left) std::unreachable();
		if(!op.right) std::unreachable();
		return ops.template int_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_fp_div> ) {
		return ops.template fp_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_multipli> ) {
		return ops.template mul<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_substruct> ) {
		return ops.template sub<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_addition> ) {
		return ops.template add<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_power> ) {
		return ops.template pow<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_not> ) {
		return ops.template negate<data_type>( visit(*this,*op.expr) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_and> ) {
		return ops.template do_and<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_or> ) {
		return ops.template do_or<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, list_expr> ) {
		return data_type{ (integer_t)3 };
	}
	constexpr data_type operator()(const auto& op) const {
		std::unreachable(); // your specialization dosen't work :(
		return data_type{ (integer_t)__LINE__ };
	}

	template<typename gh, template<auto>class th=gh::template tmpl>
	constexpr auto parse_str(auto&& src) const {
		using expr_t = ast_forwarder<expr_type<ast_forwarder>>;
		auto mk_fwd = [this](auto& v){ return df.mk_fwd(v); }; 
		auto expr_p = rv([this](auto& v){ return df.mk_result(v); }
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"and"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"or">  >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'+'>::_char >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'-'>::_char >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'*'>::_char >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"//"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"/"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"**"> >> ++gh::rv_rreq(mk_fwd))
			, cast<unary_op<expr_t>>(th<'!'>::_char++ >> --gh::rv_rreq(mk_fwd))
			, th<'['>::_char++ >> --((gh::rv_req(mk_fwd)) % ',') >> th<']'>::_char // TODO: initialize list member with data factory?
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
	constexpr static auto test_terms(auto src) {
		data_type env;
		operators_factory ops;
		bastard ev{&env, ops};
		auto parsed = ev.parse_str<gh>(src);
		return ev(parsed);
	}

	template<typename gh>
	constexpr static bool test_terms() {
		static_assert( (integer_t)test_terms<gh>("1") == 1 );
		static_assert( (integer_t)test_terms<gh>("2") == 2 );
		static_assert( ((string_t)test_terms<gh>("'ok'"))[1] == 'k' );
		static_assert( (float_point_t)test_terms<gh>("0.2") == (float_point_t)0.2 );
		static_assert( (bool)test_terms<gh>("true") == true );
		static_assert( (bool)test_terms<gh>("false") == false );
		static_assert( (integer_t)test_terms<gh>("4 // 2") == 2 );
		static_assert( (integer_t)test_terms<gh>("5 // 2") == 2 );
		static_assert( (integer_t)test_terms<gh>("6 // 2") == 3 );
		static_assert( (integer_t)test_terms<gh>("5 * 2") == 10 );
		static_assert( (float_point_t)test_terms<gh>("5 * 0.5") == 2.5 );
		static_assert( (float_point_t)test_terms<gh>("0.5 * 5") == 2.5 );
		static_assert( (float_point_t)test_terms<gh>("5 / 2") == 2.0 );
		static_assert( (float_point_t)test_terms<gh>("5 / 2.0") == 2.5 );
		static_assert( (float_point_t)test_terms<gh>("5 - 2.0") == 3 );
		static_assert( (integer_t)test_terms<gh>("5 + 2") == 7 );
		static_assert( (integer_t)test_terms<gh>("5 + 2 * 3") == 11 );
		static_assert( (integer_t)test_terms<gh>("10 ** 2") == 100 );
		static_assert( (integer_t)test_terms<gh>("5+5 ** 2") == 30 ); // 5 + 25

		static_assert( ((integer_t)test_terms<gh>("(3 + 2) * 2 + 3 + 1 + 2 + 3 + 4 + 5")) == 28 );

		static_assert( (bool)test_terms<gh>("!true") == false , "to bool and invert" );
		static_assert( (bool)test_terms<gh>("!0") == true , "to bool and invert" );
		static_assert( (bool)test_terms<gh>("!1") == false , "to bool and invert" );
		static_assert( (bool)test_terms<gh>("!.05") == false , "to bool and invert (c++ rules used)" );
		static_assert( (bool)test_terms<gh>("!'str'") == false , "to bool and invert" );
		static_assert( (bool)test_terms<gh>("!''") == true , "to bool and invert" );

		static_assert( (bool)test_terms<gh>("true and !true") == false );
		static_assert( (bool)test_terms<gh>("true or !true") == true );

		//static_assert( (integer_t)(test_terms<gh>("[1,2,3]")[2]) == 3 );

		return true;
	}

	template<typename gh>
	constexpr static bool test_parse() {
		data_type env;
		operators_factory ops;
		//static_assert( ({ auto r = bastard{&env,ops}.parse_str<gh>("true"); r.index(); }) == 13 );
		//static_assert( ({ auto r = bastard{&env,ops}.parse_str<gh>("[1,2]"); r.index(); }) == 9 );
		/*
		static_assert( ({ auto r = parse_str<gh>("1", rm,mk_fwd); r.index(); }) == 7 );
		static_assert( ({ auto r = parse_str<gh>("3.14", rm,mk_fwd); r.index(); }) == 8 );
		static_assert( ({ auto r = parse_str<gh>("'ok'", rm,mk_fwd); r.index(); }) == 6 );
		static_assert( ({ auto r = parse_str<gh>("1 + 3", rm,mk_fwd); r.index(); }) == 0 );
		static_assert( ({ auto r = parse_str<gh>("1 + 3", rm,mk_fwd); get<0>(r).left->index(); }) == 7 );
		static_assert( ({ auto r = parse_str<gh>("1 // 3", rm,mk_fwd); r.index(); }) == 3 );
		static_assert( ({ auto r = parse_str<gh>("1 // 3", rm,mk_fwd); get<7>(*get<3>(r).right); }) == 3 );
		static_assert( ({ auto r = parse_str<gh>("1 // 3", rm,mk_fwd); get<3>(r).left->index(); }) == 7 );
		static_assert( ({ auto r = parse_str<gh>("1 // 3", rm,mk_fwd); get<7>(*get<3>(r).left); }) == 1 );
		static_assert( ({ auto r = parse_str<gh>("1*3+2", rm,mk_fwd); r.index(); }) == 0 );
		*/
		return true;
	}

	template<typename gh>
	constexpr static bool test() {
		return test_parse<gh>() && test_terms<gh>();
	}
};

