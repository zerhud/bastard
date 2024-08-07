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

template<typename factory>
struct base_vertex {
	using field_name_type = typename factory::field_name_type;
	using data_type = typename factory::data_type;
	constexpr virtual ~base_vertex() =default ;
	constexpr virtual data_type field(field_name_type fn) const =0 ;
	constexpr virtual bool is_array() const =0 ;
};

template<typename factory, typename type>
struct vertex_info : base_vertex<factory> {
	using field_name_type = typename factory::field_name_type;
	using data_type = typename factory::data_type;

	factory f;
	const type* data;
	constexpr explicit vertex_info(const factory& f, const type* data) : f(f), data(data) {}
	constexpr bool is_array() const override { return tref::vector<type>; }
	constexpr virtual data_type field(field_name_type fn) const override {
		if constexpr (tref::vector<type>) return data_type{} ;
		else {
			struct node<factory, type> node{f, data};
			data_type ret{};
			node.for_each_field_value([&](auto&& name, const auto& v) {
				if (name == fn) ret = v;
			});
			return ret;
		}
	}
};

template<typename factory, typename... types>
struct vertex {
	using holder_type = typename factory::template variant<vertex_info<factory, types>...>;

	template<typename current>
	constexpr explicit vertex(const factory& f, const current* val)
	: data(val)
	, index(details::index_of_v<current, types...>)
	, holder(vertex_info<factory, current>{f, val})
	, info(get<vertex_info<factory, current>>(holder))
	{
		static_assert( (std::is_same_v<current, types> || ...), "no type found in available types" );
	}

	const void* data;
	unsigned index;
	holder_type holder;
	const base_vertex<factory>& info;
};

template<typename factory, typename... types>
struct edge {
	using field_name_type = typename factory::field_name_type;
	field_name_type name;
	unsigned parent;
	unsigned child;
};

template<typename factory, typename... types>
struct graph {
	using edges_type = decltype(mk_vec<struct edge<factory, types...>>(factory{}));
	using vertexes_type = decltype(mk_list<struct vertex<factory, types...>>(factory{}));

	edges_type edges;
	vertexes_type vertexes;
	const struct vertex<factory, types...>* root;

	constexpr graph(const factory& f)
		: edges(mk_vec<struct edge<factory, types...>>(f))
		, vertexes(mk_list<struct vertex<factory, types...>>(f))
	{}
};
template<typename factory, typename... types>
constexpr auto child(const graph<factory, types...>& g, const vertex<factory, types...>* v, unsigned ind) {
	unsigned cur_ind = 0;
	for(auto& cur:g.edges) {
		if(&g.vertexes[cur.parent] == v) {
			if(cur_ind++ == ind) return &g.vertexes[cur.child];
		}
	}
	return static_cast<const vertex<factory, types...>*>(nullptr);
}

namespace details {
template<typename type, typename factory>
constexpr auto fill_with_all_data(auto& result, const factory& f, const type& source, unsigned parent) ;
template<typename factory, template<typename...> class holder, typename... types>
constexpr auto mk_empty_result(const factory &f, holder<types...>) {
	return graph<factory, types...>{f};
}
template<typename type, typename factory>
constexpr auto add_vertex(auto& result, const type& source, const factory& f) {
	result.vertexes.emplace_back(f, &source);
	return result.vertexes.size()-1;
}
template<typename type, typename factory>
constexpr auto add_child(auto& result, const factory& f, auto&& name, const type& child, unsigned parent) {
	auto cur = add_vertex(result, child, f);
	result.edges.emplace_back(name, parent, cur);
	fill_with_all_data(result, f, child, cur);
}
template<tref::vector type, typename factory>
constexpr auto add_child(auto& result, const factory& f, auto&& name, const type& child, unsigned parent) {
	auto cur = add_vertex(result, child, f);
	result.edges.emplace_back(name, parent, cur);
	for(unsigned i=0;i<child.size();++i) {
		add_child(result, f, to_field_name(f, i), child[i], cur);
	}
}
template<tref::variant type, typename factory>
constexpr auto add_child(auto& result, const factory& f, auto&& name, const type& child, unsigned parent) {
	//TODO: what to do if it's in a monostate ?
	return visit([&](const auto& child){ return add_child(result, f, std::forward<decltype(name)>(name), child, parent); }, child);
}
//TODO: add optional
template<tref::any_ptr type, typename factory>
constexpr auto add_child(auto& result, const factory& f, auto&& name, const type& child, unsigned parent) {
	//TODO: what if it is a nullptr (same as in variant in monostate?)?
	return add_child(result, f, std::forward<decltype(name)>(name), *child, parent);
}
template<typename type, typename factory>
constexpr auto fill_with_all_data(auto& result, const factory& f, const type& source, unsigned parent) {
	node<factory, type>{f, &source}.for_each_child_value([&](auto&& name, auto& child) {
		add_child(result, f, name, child, parent);
	});
}

// eq neq and not in ()
// (a:b && c:d) || ((d):())
// 0{} -[]-> (3!{a=b} -0-> {b>a})

template<typename factory> struct query_ident { typename factory::string_type val; };
template<typename expr> struct query_unary { expr val; };
template<typename expr> struct query_binary { expr left; expr right; };
template<typename expr> struct query_eq : query_binary<expr> { };
template<typename expr> struct query_neq : query_binary<expr> { };
template<typename expr> struct query_and : query_binary<expr> { };
template<typename expr> struct query_or : query_binary<expr> { };
template<typename expr> struct query_not : query_unary<expr> { };
template<typename expr> struct query_name_eq : query_unary<expr> { };
template<typename expr, typename factory> struct query_in {
	template<typename type> using vec = decltype(mk_vec<type>(factory{}));
	expr name;
	vec<expr> values;
};

template<typename factory> struct query_expr;
template<typename factory> using fwd_ast = typename factory::template forward_ast<query_expr<factory>>;
template<typename factory>
struct query_expr : factory::template variant<
		  typename factory::template variant<
		  query_eq <fwd_ast<factory>>
		, query_neq<fwd_ast<factory>>
		, query_and<fwd_ast<factory>>
		, query_or<fwd_ast<factory>>
		>
		, query_in <fwd_ast<factory>, factory>
		, query_not<fwd_ast<factory>>
		, query_name_eq<fwd_ast<factory>>
		, query_ident<factory>
		, typename factory::string_type, typename factory::integer_type, typename factory::float_point_type, bool
		>
{
};

template<typename factory>
struct query_vertex {
	query_expr<factory> data;
};
template<typename factory>
struct query_edge {
	query_expr<factory> data;
};
template<typename factory>
struct query {
	template<typename... types> using variant = typename factory::template variant<types...>;
	template<typename type> using forward_ast = typename factory::template forward_ast<type>;
	template<typename type> using optional = typename factory::template optional<type>;

	using ident_type = query_ident<factory>;
	using name_eq_type = query_name_eq<fwd_ast<factory>>;

	using self_forward = forward_ast<query>;

	int input_number = 0; // the number before {
	bool to_output = true; // ! == false, nothing == true
	variant<query_vertex<factory>, unsigned int, query_edge<factory>, self_forward> data;
	//      {}                     -0->          -[]->                ({} --> {})
	//                             -> == -0->
	self_forward next;
};

template<typename factory, typename gh, template<auto>class th=gh::template tmpl>
constexpr auto make_query_parser(const factory& df) {
	auto mk_fwd = [&df](auto& v){ return df.mk_fwd(v); };
	using string_t = typename factory::string_t;
	using expr_type = fwd_ast<factory>;
	using binary = query_binary<expr_type>;
	constexpr auto ident =
			lexeme(gh::alpha++ >> --(*(gh::alpha | gh::d10 | th<'_'>::char_)))
			- (gh::template lit<"and"> | gh::template lit<"in"> | gh::template lit<"or"> | gh::template lit<"true"> | gh::template lit<"false">);
	auto query_expr = rv([&df](auto& v){ return df.mk_result(std::move(v)); }
			,(cast<binary>(gh::rv_lreq >> (gh::template lit<"==">|th<':'>::_char) >>  ++gh::rv_rreq(mk_fwd))
			| cast<binary>(gh::rv_lreq >> gh::template lit<"!="> >>  ++gh::rv_rreq(mk_fwd))
			| cast<binary>(gh::rv_lreq >> (gh::template lit<"and">|gh::template lit<"&&">) >> ++gh::rv_rreq(mk_fwd))
			| cast<binary>(gh::rv_lreq >> (gh::template lit<"or">|gh::template lit<"||">) >>  ++gh::rv_rreq(mk_fwd))
			 )
			, cast<query_in <expr_type, factory>>(gh::rv_lreq >> gh::template lit<"in"> >>  ++(gh::rv_rreq(mk_fwd) % ',') )
			, cast<query_unary<expr_type       >>(th<'!'>::_char++ >> --gh::rv_rreq(mk_fwd))
			, cast<query_unary<expr_type       >>(th<':'>::_char++ >> --gh::rv_rreq(mk_fwd))
			, ident
			, gh::quoted_string
			, gh::int_
			, gh::fp
			, (as<true>(gh::template lit<"true">)|as<false>(gh::template lit<"false">))
			, rv_result(th<'('>::_char >> gh::rv_req >> th<')'>::_char)
	);

	return
	(-gh::int_)++ >> (-as<false>(th<'!'>::_char))++ >>
	( cast<query_vertex<factory>>((th<'{'>::_char++ >> --th<'}'>::_char) | (th<'{'>::_char >> query_expr++ >> --th<'}'>::_char))
	| (as<0>(gh::template lit<"->">) | (th<'-'>::_char >> gh::int_ >> gh::template lit<"->">))
	| cast<query_edge<factory>>(th<'-'>::_char >> th<'['>::_char >> query_expr++ >> --th<']'>::_char >> gh::template lit<"->">)
	)++ >> -th<0>::req([]<typename type>(type& v){v.reset(new typename type::element_type{});return v.get();})
	;
}

template<typename factory, typename gh>
constexpr auto parse_query(const factory& f, auto&& src) {
	query<factory> result;
	auto parsed_sz = parse(make_query_parser<factory, gh>(f), +gh::space, gh::make_source(std::forward<decltype(src)>(src)), result);
	return result;
}

} // namespace details

constexpr auto query(const auto& f, const auto& source, auto&& q) {
	auto result = details::mk_empty_result(f, mk_children_types(f, source));
	details::fill_with_all_data(result, f, source, details::add_vertex(result, source, f));
	result.root = &result.vertexes[0];
	return result;
}

} // namespace ast_graph
