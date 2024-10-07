/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

namespace ast_graph {

template<typename factory, typename graph> struct graph_absd ;

template<typename factory, typename graph>
struct graph_absd {
	using data_type = typename factory::data_type;
	using int_t = typename data_type::integer_t;
	using str_t = typename data_type::string_t;
	using vertex_type = typename graph::vertex_interface;

	factory f;
	graph g;
	const vertex_type* cur_root;

	constexpr graph_absd(factory f, graph g, const vertex_type* root)
	: f(std::move(f))
	, g(std::move(g))
	, cur_root(root)
	{}

	constexpr bool is_eq(const graph_absd& other) const {
		//TODO: make view to subgraph and compare with subgraph from other
		//      (subgraph is a graph started from current root)
		return g == other.g && cur_root->origin() == other.cur_root->origin();
	}
	[[nodiscard]] constexpr bool is_array() const {
		return cur_root->is_array();
	}
	constexpr auto at(int_t k) {
		if(!cur_root->is_array()) return data_type{};
		auto children = children_of(g, cur_root);
		if(k > children.size()) return data_type{};
		return data_type::mk(graph_absd(f, g, children[k]));
	}
	constexpr auto at(const data_type& k) {
		if(k.is_string()) {
			auto children = ast_links_of(g, cur_root);
			auto&& str_k = (str_t)k;
			for(auto& i:children) if(i.name == str_k) return data_type::mk(graph_absd(f, g, i.child));
			if(str_k=="__parent") return cur_root->parent ? data_type::mk(graph_absd(f, g, cur_root->parent)) : data_type{};
			return cur_root->field(str_k);
		}
		if(k.is_int()) return at((int_t)k);
		return data_type{};
	}
	constexpr auto size() const {
		auto children = children_of(g, cur_root);
		return children.size() + cur_root->fields_count();
	}
	constexpr bool contains(const data_type& k) const {
		for(auto&& field:cur_root->fields()) if((str_t)k==field) return true;
		auto children = ast_links_of(g, cur_root);
		auto&& str_k = (str_t)k;
		for(const auto& child:children) if(str_k == child.name) return true;
		return false;
	}
	constexpr auto keys() const {
		auto ret = mk_vec<data_type>(f);
		for(auto&& field:cur_root->fields()) ret.emplace_back(mk_data(f, std::move(field)));
		auto children = ast_links_of(g, cur_root);
		for(const auto& child:children) ret.emplace_back(mk_data(f, child.name));
		return ret;
	}
};

} // namespace ast_graph
