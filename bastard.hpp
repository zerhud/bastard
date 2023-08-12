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
	using self_type = abstract_data<variant,string>;
	using holder_type = variant<string,float_point,integer,bool>;

	using string_t = string;
	using integer_t = integer;
	using float_point_t = float_point;
	template<typename... types> using variant_t = variant<types...>;

	holder_type holder;

	constexpr bool is_fp() const { return get_if<float_point_t>(&holder) != nullptr; }
	constexpr bool is_int() const { return get_if<integer>(&holder) != nullptr; }

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
	struct binary_op {
		std::decay_t<expr_t> left;
		std::decay_t<expr_t> right;
	};
	template<typename expr_t> struct op_division : binary_op<expr_t> {};
	template<typename expr_t> struct op_multipli : binary_op<expr_t> {};
	template<typename expr_t> struct op_fpdivision : binary_op<expr_t> {};
	template<typename expr_t> struct op_substruct : binary_op<expr_t> {};
	template<typename expr_t> struct op_addition : binary_op<expr_t> {};
	template<typename expr_t> struct op_power : binary_op<expr_t> {};

	using string_t = data_type::string_t;
	using integer_t = data_type::integer_t;
	using float_point_t = data_type::float_point_t;

	template<typename... operators>
	using parse_result = data_type::template variant_t<std::decay_t<operators>...,string_t,float_point_t,integer_t,bool>;

	template<template<class>class fa> struct expr_type : parse_result<
	       op_addition < fa<expr_type<fa>> >
	     , op_substruct< fa<expr_type<fa>> >
	     , op_multipli < fa<expr_type<fa>> >
	     , op_division < fa<expr_type<fa>> >
	     , op_fpdivision< fa<expr_type<fa>> >
	     , op_power     < fa<expr_type<fa>> >
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
		return ops.template int_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	constexpr data_type operator()(const auto& op) requires( bastard_details::is_specialization_of<std::decay_t<decltype(op)>, op_fpdivision> ) {
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
	constexpr data_type operator()(const auto& op) const {
		std::unreachable(); // your specialization dosen't work :(
		return data_type{ (integer_t)__LINE__ };
	}

	template<template<class>class ast_forwarder,typename gh, template<auto>class th=gh::template tmpl>
	constexpr static auto parse_str(auto&& src, auto mk_fwd) {
		using expr_t = ast_forwarder<expr_type<ast_forwarder>>;
		auto expr_p =
			  cast<binary_op<expr_t>>(gh::lreq(mk_fwd) >> th<'+'>::_char >> ++gh::lrreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::lreq(mk_fwd) >> th<'-'>::_char >> ++gh::lrreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::lreq(mk_fwd) >> th<'*'>::_char >> ++gh::lrreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::lreq(mk_fwd) >> gh::template lit<"//"> >> ++gh::lrreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::lreq(mk_fwd) >> gh::template lit<"/"> >> ++gh::lrreq(mk_fwd))
			| cast<binary_op<expr_t>>(gh::lreq(mk_fwd) >> gh::template lit<"**"> >> ++gh::lrreq(mk_fwd))
			| gh::quoted_string
			| gh::fp
			| gh::int_
			| (as<true>(gh::template lit<"true">)|as<false>(gh::template lit<"false">))
			// после ( +space хочет узнать парситья ли выражение, а это целый вариант, который не знает парсится он или нет
			| use_variant_result(th<'('>::_char >> gh::lvreq >> th<')'>::_char)
			;
		expr_type<ast_forwarder> r;
		parse(expr_p, +gh::space, gh::make_source(src), r);
		return r;
	}

	template<template<class>class af,typename gh>
	constexpr static auto test_terms(auto mk_fwd, auto src) {
		data_type env;
		operators_factory ops;
		auto parsed = parse_str<af,gh>(src,mk_fwd);
		//parsed.index() / (parsed.index() == 1);
		if !consteval {
			auto& op_mul = get<2>(parsed);
			std::cout << "right: " << get<8>(*op_mul.right) << std::endl;
			auto& op_add = get<0>(*op_mul.left);
			std::cout << "left: " << get<8>(*op_add.left) << "+" << get<8>(*op_add.right) << std::endl;
		}
		auto ret = bastard{&env, ops}(parsed);
		return ret;
	}

	template<template<class>class af,typename gh>
	constexpr static bool test_terms(auto mk_fwd) {
		static_assert( get<integer_t>(parse_str<af,gh>("1",mk_fwd)) == 1 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "1") == 1 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "2") == 2 );
		static_assert( ((string_t)test_terms<af,gh>(mk_fwd, "'ok'"))[1] == 'k' );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, "0.2") == (float_point_t)0.2 );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, "true") == true );
		static_assert( (bool)test_terms<af,gh>(mk_fwd, "flase") == false );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "4 // 2") == 2 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "5 // 2") == 2 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "5 * 2") == 10 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, "5 * 0.5") == 2.5 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, "0.5 * 5") == 2.5 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, "5 / 2.0") == 2.5 );
		static_assert( (float_point_t)test_terms<af,gh>(mk_fwd, "5 - 2.0") == 3 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "5 + 2") == 7 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "5 + 2 * 3") == 11 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "10 ** 2") == 100 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "5+5 ** 2") == 30 );
		static_assert( (integer_t)test_terms<af,gh>(mk_fwd, "5+5 ** 2") == 30 );

		//static_assert( ((integer_t)test_terms<af,gh>(mk_fwd, "(3 + 2) * 2 + 3 + 1 + 2 + 3 + 4 + 5")) == 13 );

		return true;
	}

	template<template<class>class af,typename gh>
	constexpr static bool test_parse(auto mk_fwd) {
		static_assert( ({ auto r = parse_str<af,gh>("true", mk_fwd); r.index(); }) == 9 );
		static_assert( ({ auto r = parse_str<af,gh>("1", mk_fwd); r.index(); }) == 8 );
		static_assert( ({ auto r = parse_str<af,gh>("3.14", mk_fwd); r.index(); }) == 7 );
		static_assert( ({ auto r = parse_str<af,gh>("'ok'", mk_fwd); r.index(); }) == 6 );
		static_assert( ({ auto r = parse_str<af,gh>("1 + 3", mk_fwd); r.index(); }) == 0 );
		static_assert( ({ auto r = parse_str<af,gh>("1*3+2", mk_fwd); r.index(); }) == 0 );
		return true;
	}

	template<template<class>class ast_forwarder,typename gh>
	constexpr static bool test(auto mk_fwd) {
		return test_parse<ast_forwarder,gh>(mk_fwd) && test_terms<ast_forwarder,gh>(mk_fwd);
	}
};

