/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "node.hpp"
#include "mk_children_types.hpp"
#include "graph_calc_size.hpp"

namespace ast_graph {

template<typename factory>
struct ast_vertex {
	using string_view = typename factory::string_view;
	struct link {
		string_view name{};
		const ast_vertex* vertex=nullptr;
	};

	using names_type = decltype(mk_vec<string_view>(factory{}));
	using data_type  = typename factory::data_type;
	using link_holder = decltype(mk_vec<link>(factory{}));

	virtual constexpr ~ast_vertex() noexcept =default ;

	link_holder children ; //< all children. links will be added to children later
	const ast_vertex* parent = nullptr ;

	virtual string_view type_name() const =0 ;
	virtual names_type fields() const =0 ;
	virtual data_type field(string_view name) const =0 ;
	virtual bool is_array() const =0 ;
	virtual unsigned size() const =0 ; //TODO: do we really still need this method ?
	virtual unsigned fields_count() const =0 ;
	virtual const char* debug_info() const {return "";}
	virtual const void* origin() const =0 ;
};

template<typename factory, typename source>
struct ast_vertex_holder : ast_vertex<factory> {
	using base = ast_vertex<factory>;
	using data_type = base::data_type;
	using names_type = base::names_type;
	using string_view = base::string_view;
	using node_type = struct node<factory, source>;

	constexpr ast_vertex_holder(factory f, const source& val) : f(std::move(f)), src(&val) {}
	constexpr ~ast_vertex_holder() noexcept =default ;

	factory f;
	const source* src;

	constexpr const void* origin() const override { return src; }
	constexpr node_type create_node() const { return node_type{ f, src }; }
	constexpr string_view type_name() const override { return create_node().name(); }
	constexpr names_type fields() const override {
		names_type ret = mk_vec<string_view>(f);
		if constexpr(!tref::vector<source>)
			for(auto&& i:create_node().list_fields()) ret.emplace_back(i);
		return ret;
	}
	constexpr data_type field(string_view name) const override {
		if constexpr(tref::vector<source>) return data_type{};
		else return visit([](const auto& v){
			constexpr bool is_optional = requires{ static_cast<bool>(v);*v;v.value(); };
			if constexpr (is_optional && requires{ data_type{*v}; }) return data_type{v.value()};
			else if constexpr (requires{ data_type{v}; }) return data_type{v};
			else return data_type{};
		}, create_node().field_value(name));
	}
	constexpr bool is_array() const override {
		return tref::iterable<source>;
	}
	constexpr unsigned fields_count() const override {
		if constexpr (requires{ src->size(); }) return src->size();
		else {
			auto n = create_node();
			return n.fields_count();
		}
	}
	constexpr unsigned size() const override {
		if constexpr (requires{ src->size(); }) return src->size();
		else {
			auto n = create_node();
			return n.fields_count() + this->children.size();
		}
	}

	constexpr const char* debug_info() const override {return __PRETTY_FUNCTION__;}
};

template<typename factory, typename... types>
struct vertex_holder {
	template<typename t> using vertex = ast_vertex_holder<factory, t>;
	using variant = factory::template variant<vertex<types>...>;
	explicit constexpr vertex_holder(const factory& f, const auto& val) : holder(vertex<tref::decay_t<decltype(val)>>(f, val)) {
		base = &get<vertex<tref::decay_t<decltype(val)>>>(holder);
	}
	ast_vertex<factory>* base;
	variant holder;
};

template<typename factory, typename _node_type> struct graph_view ;
template<typename factory, typename _vertex_type>
struct graph_holder {
	using view_type = graph_view<factory, _vertex_type>;
	using vertex_type = _vertex_type;
	using vertex_interface = ast_vertex<factory>;
	using string_view = typename factory::string_view;
	struct link {
		string_view name{};
		const vertex_interface* parent = nullptr;
		const vertex_interface* child = nullptr;
	};

	using link_holder = decltype(mk_vec<link>(std::declval<factory>()));
	using vertex_holder = decltype(mk_vec<vertex_type>(std::declval<factory>()));
	using data_type  = typename factory::data_type;

	constexpr graph_holder(const factory& f)
	: f(f)
	, edges(mk_vec<link>(this->f))
	, vertices(mk_vec<vertex_type>(this->f))
	{}

	factory f;
	link_holder edges;
	vertex_holder vertices;

	constexpr auto size() const { return vertices.size(); }
	constexpr auto& root() { return vertices.front(); }
	constexpr auto create_view() const { return view_type( *this ); }
	constexpr friend void reserve_vertex_count(graph_holder& h, auto sz) { h.vertices.reserve(sz); }
	constexpr friend auto& create_vertex(graph_holder& h, auto&&... args) {
		return h.vertices.emplace_back(std::forward<decltype(args)>(args)...);
	}

	constexpr friend auto& create_ast_link( graph_holder& h, auto&& name, const auto* parent, const auto* child) {
		return h.edges.emplace_back( std::forward<decltype(name)>(name), parent, child );
	}
};

template<typename factory, typename _node_type>
struct graph_view {
	using holder = graph_holder<factory, _node_type>;
	using link = holder::link;
	using link_holder = holder::link_holder;
	using vertex_type = holder::vertex_type;
	using vertex_interface = holder::vertex_interface;
	using vertex_holder = decltype(mk_vec<const vertex_type*>(factory{}));

	factory f;
	link_holder edges;
	vertex_holder vertices;

	constexpr graph_view(const holder& h) : graph_view(h.f) {
		for(auto& e:h.edges) edges.emplace_back(e);
		for(auto& v:h.vertices) vertices.emplace_back(&v);
	}

	constexpr explicit graph_view(const factory& f)
	: f(f)
	, edges(mk_vec<link>(f))
	, vertices(mk_vec<const vertex_type*>(f))
	{}

	constexpr auto size() const { return vertices.size(); }
	constexpr auto* root() { return vertices.front()->base; }

	constexpr friend auto& create_vertex(graph_view& h, const vertex_type* v) {
		return h.vertices.emplace_back(v);
	}

	constexpr friend auto ast_links_of(const graph_view& view, const vertex_interface* root) {
		link_holder ret = mk_vec<link>(view.f);
		for(auto& e:view.edges) if(e.parent == root) ret.emplace_back(e);
		return ret;
	}
	constexpr friend auto children_of(const graph_view& view, const vertex_interface* root) {
		auto ret = mk_vec<const vertex_interface*>(view.f);
		for(auto& e:view.edges) if(e.parent == root) ret.emplace_back(e.child);
		return ret;
	}

	constexpr friend bool operator==(const graph_view& left, const graph_view& right) {
		if(left.vertices.size()!=right.vertices.size()) return false;
		for(auto i=0;i<left.vertices.size();++i) {
			if(left.vertices[i]->base->origin() != right.vertices[i]->base->origin()) return false;
		}
		return true;
	}
};

template<typename factory, template<typename...>class list, typename... types>
constexpr auto mk_vertex_holder_type(list<types...>) {
	return tref::type_c< vertex_holder<factory, types...> >;
}

template<typename factory, tref::vector source>
constexpr auto mk_graph(const factory& f, auto& storage, auto&& name, auto& parent, const source& src) ;
template<typename factory, tref::variant source>
constexpr auto mk_graph(const factory& f, auto& storage, auto&& name, auto& parent, const source& src) ;
template<typename factory, tref::any_ptr source>
constexpr auto mk_graph(const factory& f, auto& storage, auto&& name, auto& parent, const source& src) ;
template<typename factory, typename source>
constexpr auto mk_graph(const factory& f, auto& storage, auto&& name, auto& parent, const source& src) ;

template<typename factory, tref::vector source>
constexpr auto mk_graph(const factory& f, auto& storage, auto&& name, auto& parent, const source& src) {
	auto& cur_parent = *create_vertex(storage, f, src).base;
	create_ast_link( storage, name, &parent, &cur_parent );
	cur_parent.parent = &parent;
	parent.children.emplace_back( typename ast_vertex<factory>::link{ std::forward<decltype(name)>(name), &cur_parent} );
	parent.children.back().vertex = &cur_parent;
	for(const auto& i:src) mk_graph(f, storage, "", cur_parent, i);
}
template<typename factory, tref::variant source>
constexpr auto mk_graph(const factory& f, auto& storage, auto&& name, auto& parent, const source& src) {
	return visit( [&](const auto& src){ return mk_graph(f, storage, std::forward<decltype(name)>(name), parent, src); }, src );
}
template<typename factory, tref::any_ptr source>
constexpr auto mk_graph(const factory& f, auto& storage, auto&& name, auto& parent, const source& src) {
	if(src) mk_graph(f, storage, std::forward<decltype(name)>(name), parent, *src);
}
template<typename factory, typename source>
constexpr auto mk_graph(const factory& f, auto& storage, auto&& name, auto& parent, const source& src) {
	auto& cur_parent = *create_vertex( storage, f, src ).base;
	create_ast_link( storage, name, &parent, &cur_parent );
	cur_parent.parent = &parent;
	parent.children.emplace_back( typename ast_vertex<factory>::link{std::forward<decltype(name)>(name), &cur_parent} );
	parent.children.back().vertex = &cur_parent;
	node<factory, source>{f, &src}.for_each_child_value([&](auto&& name, const auto& v){
		mk_graph(f, storage, std::forward<decltype(name)>(name), cur_parent, v);
	});
}

template<typename factory>
constexpr auto mk_empty_graph(const factory& f, const auto& src) {
	using children_type = decltype(mk_children_types(factory{}, src));
	using node_type = typename decltype(mk_vertex_holder_type<factory>(children_type{}))::type;
	return graph_holder<factory, node_type>{f};
}

template<typename factory>
constexpr auto mk_graph(const factory& f, const auto& src) {
	auto graph_nodes = mk_empty_graph(f, src);
	reserve_vertex_count( graph_nodes, mk_graph_calc_size(f, src) );
	auto& parent = *create_vertex(graph_nodes, f, src ).base;
	node<factory, tref::decay_t<decltype(src)>>{f, &src}.for_each_child_value([&](auto&& name, const auto& value) {
		mk_graph(f, graph_nodes, std::forward<decltype(name)>(name), parent, value);
	});
	return graph_nodes;
}

} // namespace ast_graph
