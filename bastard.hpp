/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of modegen.
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

template<template<typename>class vector, typename key, typename value>
struct constexpr_kinda_map {
	struct chunk {
		key k;
		value v;
	};
	vector<chunk> store;

	using value_type = chunk;

	constexpr void insert(auto&& _v) {
		auto&& [k,v] = _v;
		store.emplace_back( chunk{ std::move(k), std::move(v) } );
	}

	constexpr auto& at(const key& fk) const {
		for(auto&[k,v]:store) if(k==fk) return v;
		throw __LINE__;
	}
};

template<typename container_t, typename factory_t, typename data_type>
struct vectorized_data {
	using value_type = data_type;
	//using used_type = decltype(std::declval<factory_t>().mk_ptr(std::declval<data_type>()));

	factory_t factory;
	//vector<used_type> container;
	container_t container;

	constexpr auto clear() { return container.clear(); }
	constexpr auto& at(auto ind) const { return *container.at(ind); }
	constexpr auto push_back(auto&& d) requires( std::is_same_v<std::decay_t<decltype(d)>, data_type> ){
		return container.push_back(factory.template mk_ptr<data_type>(std::forward<decltype(d)>(d)));
	}
};

//template<template<typename...>class variant_t, typename integer, typename float_point, typename string>
struct absdata_raw_ptr_factory {
	//template<typename type> using smart_ptr = type*;
	//template<typename... types> using variant = variant_t<types...>;
	//using float_point_t = float_point;
	//using integer_t = integer;
	//using string_t = string;

	template<typename type> constexpr auto mk_ptr(auto&&... args) const { return new type(std::forward<decltype(args)>(args)...); }
	[[noreturn]] constexpr static void throw_wrong_interface_error(auto&& op){ throw __LINE__; }
};

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

template< typename factory, typename final_type, typename... implementations >
struct abstract_data {
	using factory_t = factory;
	template<typename... types> using variant_t = typename factory_t::template variant<types...>;

	//using self_type = abstract_data<factory_t, final_type, implementations...>;
	using self_type = final_type;

	using string_t = typename factory_t::string_t;
	using integer_t = typename factory_t::integer_t;
	using float_point_t = typename factory_t::float_point_t;

	using holder_type = variant_t<string_t,integer_t,float_point_t,bool, implementations...>;

	static_assert( (std::copyable<implementations> && ...), "all implementations type must to be copyable" );
	//static_assert( ((!requires{ typename implementations::value_type; } || std::copyable<typename implementations::value_type>) && ...), "all value_tye in implementations type must to be copyable also" );

	template<typename type>
	constexpr static bool is_array =
		   (requires(const type& t){ self_type{ t.at(0) }; } || requires(const type& t){ self_type{ *t.at(0) }; })
		&& requires(type& t){ t.push_back(std::declval<typename type::value_type>()); }
	;
	template<typename type>
	constexpr static bool is_map = requires(const type& t){ self_type{ t.at("some_key") }; } ;

	template<typename type>
	constexpr static bool is_data_container = std::is_same_v<typename std::decay_t<type>::value_type, self_type>;

	holder_type holder;

	constexpr bool is_fp() const { return get_if<float_point_t>(&holder) != nullptr; }
	constexpr bool is_str() const { return holder.index() == 0; }
	constexpr bool is_int() const { return get_if<integer_t>(&holder) != nullptr; }
	constexpr bool is_bool() const { return holder.index() == 3; }
	constexpr bool is_arr() const {
		return exec_for_arr([](const auto&){ return true; }, []{ return false; });
	}

	constexpr explicit operator string_t() const { return get<string_t>(holder); }
	constexpr explicit operator bool() const { return get<bool>(holder); }

	constexpr explicit operator integer_t() const {
		auto* fp = get_if<float_point_t>(&holder);
		return fp ? (integer_t)(*fp) : get<integer_t>(holder);
	}
	constexpr explicit operator float_point_t() const {
		auto* fp = get_if<float_point_t>(&holder);
		return fp ? *fp : get<integer_t>(holder);
	}
	constexpr void push_back(self_type data) {
		exec_for_arr(
			[data=std::move(data)](auto& v)mutable{
				using ar_t = std::decay_t<decltype(v)>;
				//static_assert( std::is_same_v<typename ar_t::value_type, self_type> );
				//if constexpr ( std::is_same_v<typename ar_t::value_type, self_type> )
				if constexpr ( requires{ v.push_back(std::move(data)); } ) {
					static_assert( bastard_details::is_specialization_of<std::decay_t<decltype(v)>, bastard_details::vectorized_data> );
					v.push_back(std::move(data));
				} else v.push_back( (typename ar_t::value_type)data );
			}, []{ factory_t::throw_wrong_interface_error("push_back"); }
			);
	}
	constexpr void put(string_t key, auto data) {
		exec_for_map(
			[&key,&data](auto& v){v.insert(typename std::decay_t<decltype(v)>::value_type{ key, std::move(data) });},
			[]{});
	}
	constexpr self_type operator[](integer_t ind) {
		return exec_for_arr(
			[ind](auto&v){ return self_type{ v.at(ind) }; },
			[]()->self_type{ factory_t::throw_wrong_interface_error("operator[](ind)"); }
			);
	}
	constexpr self_type operator[](string_t key) {
		return exec_for_map(
			[key](auto&v){ return self_type{ v.at(key) }; },
			[]()->self_type{ factory_t::throw_wrong_interface_error("operator[](key)"); }
			);
	}
private:

	constexpr auto exec_for_arr(auto&& ar, auto&& other) {
		return visit( bastard_details::overloaded {
			[&other](string_t&){ return other(); },
			[&ar,&other](auto& v) {
			if constexpr (is_array<std::decay_t<decltype(v)>>) return ar(v);
			else return other();
			}
		}, holder );
	}
	constexpr auto exec_for_arr(auto&& ar, auto&& other) const {
		return visit( bastard_details::overloaded {
			[&other](const string_t&){ return other(); },
			[&ar,&other](const auto& v) {
			if constexpr (is_array<std::decay_t<decltype(v)>>) return ar(v);
			else return other();
			}
		}, holder );
	}
	constexpr auto exec_for_map(auto&& map, auto&& other) const {
		return visit( [&map,&other](const auto& v) {
			if constexpr (is_map<std::decay_t<decltype(v)>>) return map(v);
			else return other();
			}, holder);
	}
	constexpr auto exec_for_map(auto&& map, auto&& other) {
		return visit( [&map,&other](auto& v) {
			if constexpr (is_map<std::decay_t<decltype(v)>>) return map(v);
			else return other();
			}, holder);
	}
};

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

	constexpr static bool test_abstract_data() {
		data_factory df;
		using int_vec_type = decltype(df.template mk_vec<typename data_type::integer_t>());
		using int_map_type = decltype(df.template mk_map<typename data_type::string_t, typename data_type::integer_t>());
		using integer_t = typename data_type::integer_t;
		struct data_t : abstract_data< typename data_type::factory_t, data_t, int_vec_type, int_map_type> {};

		static_assert( data_t{ true }.is_bool() == true );
		static_assert( data_t{ (integer_t)1 }.is_int() == true );
		static_assert( data_t{ (integer_t)1 }.is_str() == false );
		static_assert( data_t{ (integer_t)1 }.is_arr() == false );
		static_assert( data_t{ (integer_t)1 }.is_bool() == false );
		static_assert( data_t{ int_vec_type{} }.is_arr() == true );
		static_assert( (integer_t)([]{data_t d{int_vec_type{}};d.push_back(data_t{(integer_t)10});return d[0];}()) == 10 );
		static_assert( (integer_t)([]{data_t d{int_vec_type{}};d.push_back(data_t{(integer_t)15});return d[0];}()) == 15 );
		static_assert( (integer_t)([]{data_t d{int_map_type{}};d.put("key",(integer_t)15);return d["key"];}()) == 15 );
		static_assert( (integer_t)([]{data_t d{int_map_type{}};
				d.put("key",(integer_t)15);
				d.put("key2",(integer_t)10);
				return d["key2"];}()) == 10 );
		/*
		*/
		return true;
	}

	constexpr static bool test_sc_abstract_data() {
		struct data_sc_t;
		data_factory df;
		using ptr_type = data_sc_t*;//decltype(df.template mk_ptr<data_sc_t>());
		using sc_vec_type = decltype(df.template mk_vec<ptr_type>());
		using vectorized_type = bastard_details::vectorized_data<sc_vec_type, bastard_details::absdata_raw_ptr_factory, data_sc_t>;
		struct data_sc_t : abstract_data< typename data_type::factory_t, data_sc_t, vectorized_type> {};

		/*
		static_assert( []{ vectorized_type v; v.push_back(data_sc_t{ (integer_t)10 }); auto vv = v; return (integer_t)vv.at(0); }() == 10 );
		static_assert( []{ data_sc_t d{vectorized_type{}};
			d.push_back(data_sc_t{(integer_t)10});
			integer_t ret = (integer_t)d[0];
			return ret;
			}() == 10);
		*/

		return true;
	}

	template<typename gh>
	constexpr static bool test() {
		return test_abstract_data() && test_sc_abstract_data() && test_parse<gh>() && test_terms<gh>();
	}
};

