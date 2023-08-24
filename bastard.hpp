/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of modegen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 **************************************************************************/

#include <utility>
#include <iostream>

namespace bastard_details {

template<typename type, template<typename...>class tmpl> constexpr const bool is_specialization_of = false;
template<template<typename...>class type, typename... args> constexpr const bool is_specialization_of<type<args...>, type> = true;

struct expr_operators_simple {
	template<typename data_type>
	constexpr static auto math_op(auto&& l, auto&& r, auto&& op) {
		using integer_t = data_type::integer_t;
		using float_point_t = data_type::float_point_t;
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
		using integer_t = data_type::integer_t;
		using float_point_t = data_type::float_point_t;
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
		using integer_t = data_type::integer_t;
		using float_point_t = data_type::float_point_t;
		using string_t = data_type::string_t;
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
		return data_type{ !to_bool<data_type>(std::move(val)) };
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
		using integer_t = data_type::integer_t;
		using float_point_t = data_type::float_point_t;
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

template<
	template<class...>class variant,
	typename string,
	typename integer=int,
	typename float_point=double
	>
struct abstract_data {
	using self_type = abstract_data<variant,string,integer,float_point>;
	using holder_type = variant<string,integer,float_point,bool>;

	using string_t = string;
	using integer_t = integer;
	using float_point_t = float_point;
	template<typename... types> using variant_t = variant<types...>;

	holder_type holder;

	constexpr bool is_fp() const { return get_if<float_point_t>(&holder) != nullptr; }
	constexpr bool is_str() const { return holder.index() == 0; }
	constexpr bool is_int() const { return get_if<integer>(&holder) != nullptr; }
	constexpr bool is_bool() const { return holder.index() == 3; }

	constexpr operator string() const { return get<string>(holder); }
	constexpr explicit operator bool() const { return get<bool>(holder); }

	constexpr operator integer() const {
		auto* fp = get_if<float_point_t>(&holder);
		return fp ? (integer_t)(*fp) : get<integer_t>(holder);
	}
	constexpr operator float_point() const {
		auto* fp = get_if<float_point_t>(&holder);
		return fp ? *fp : get<integer_t>(holder);
	}
};

template< typename data_type, typename operators_factory >
struct bastard {
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

	using string_t = data_type::string_t;
	using integer_t = data_type::integer_t;
	using float_point_t = data_type::float_point_t;

	template<typename... operators>
	using parse_result = data_type::template variant_t<std::decay_t<operators>...,string_t,integer_t,float_point_t,bool>;

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
	> {};

	data_type* env;
	operators_factory ops;

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
	constexpr data_type operator()(const auto& op) const {
		std::unreachable(); // your specialization dosen't work :(
		return data_type{ (integer_t)__LINE__ };
	}

	template<template<class>class ast_forwarder,typename gh, template<auto>class th=gh::template tmpl>
	constexpr static auto parse_str(auto&& src, auto result_maker, auto mk_fwd) {
		using expr_t = ast_forwarder<expr_type<ast_forwarder>>;
		auto expr_p = rv(result_maker
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"and"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"or">  >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'+'>::_char >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'-'>::_char >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> th<'*'>::_char >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"//"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"/"> >> ++gh::rv_rreq(mk_fwd))
			, cast<binary_op<expr_t>>(gh::rv_lreq >> gh::template lit<"**"> >> ++gh::rv_rreq(mk_fwd))
			, cast<unary_op<expr_t>>(th<'!'>::_char++ >> --gh::rv_rreq(mk_fwd))
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

	template<template<class>class af,typename gh>
	constexpr static auto test_terms(auto mk_fwd, auto result_maker, auto src) {
		data_type env;
		operators_factory ops;
		auto parsed = parse_str<af,gh>(src, result_maker, mk_fwd);
		auto ret = bastard{&env, ops}(parsed);
		return ret;
	}

	template<template<class>class af,typename gh>
	constexpr static bool test_terms(auto mk_fwd, auto rm) {
		static_assert( get<integer_t>(parse_str<af,gh>("1",rm,mk_fwd)) == 1 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "1") == 1 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "2") == 2 );
		static_assert( ((string_t)test_terms<af,gh>(mk_fwd, rm, "'ok'"))[1] == 'k' );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, rm, "0.2") == (float_point_t)0.2 );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "true") == true );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "false") == false );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "4 // 2") == 2 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "5 // 2") == 2 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "6 // 2") == 3 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "5 * 2") == 10 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, rm, "5 * 0.5") == 2.5 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, rm, "0.5 * 5") == 2.5 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, rm, "5 / 2") == 2.0 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, rm, "5 / 2.0") == 2.5 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, rm, "5 - 2.0") == 3 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "5 + 2") == 7 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "5 + 2 * 3") == 11 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "10 ** 2") == 100 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, rm, "5+5 ** 2") == 30 ); // 5 + 25

		static_assert( ((integer_t)test_terms<af,gh>(mk_fwd, rm, "(3 + 2) * 2 + 3 + 1 + 2 + 3 + 4 + 5")) == 28 );

		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "!true") == false , "to bool and invert" );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "!0") == true , "to bool and invert" );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "!1") == false , "to bool and invert" );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "!.05") == false , "to bool and invert (c++ rules used)" );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "!'str'") == false , "to bool and invert" );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "!''") == true , "to bool and invert" );

		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "true and !true") == false );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, rm, "true or !true") == true );

		return true;
	}

	template<template<class>class af,typename gh>
	constexpr static bool test_parse(auto mk_fwd, auto rm) {
		/*
		static_assert( ({ auto r = parse_str<af,gh>("true", rm,mk_fwd); r.index(); }) == 9 );
		static_assert( ({ auto r = parse_str<af,gh>("1", rm,mk_fwd); r.index(); }) == 7 );
		static_assert( ({ auto r = parse_str<af,gh>("3.14", rm,mk_fwd); r.index(); }) == 8 );
		static_assert( ({ auto r = parse_str<af,gh>("'ok'", rm,mk_fwd); r.index(); }) == 6 );
		static_assert( ({ auto r = parse_str<af,gh>("1 + 3", rm,mk_fwd); r.index(); }) == 0 );
		static_assert( ({ auto r = parse_str<af,gh>("1 + 3", rm,mk_fwd); get<0>(r).left->index(); }) == 7 );
		static_assert( ({ auto r = parse_str<af,gh>("1 // 3", rm,mk_fwd); r.index(); }) == 3 );
		static_assert( ({ auto r = parse_str<af,gh>("1 // 3", rm,mk_fwd); get<7>(*get<3>(r).right); }) == 3 );
		static_assert( ({ auto r = parse_str<af,gh>("1 // 3", rm,mk_fwd); get<3>(r).left->index(); }) == 7 );
		static_assert( ({ auto r = parse_str<af,gh>("1 // 3", rm,mk_fwd); get<7>(*get<3>(r).left); }) == 1 );
		static_assert( ({ auto r = parse_str<af,gh>("1*3+2", rm,mk_fwd); r.index(); }) == 0 );
		*/
		return true;
	}

	template<template<class>class ast_forwarder,typename gh>
	constexpr static bool test(auto mk_fwd, auto result_maker) {
		return test_parse<ast_forwarder,gh>(mk_fwd, result_maker) && test_terms<ast_forwarder,gh>(mk_fwd, result_maker);
	}
};

