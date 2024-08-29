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

	factory f;
	const graph* g;

	constexpr graph_absd(factory f, const graph* g)
	: f(std::move(f))
	, g(g)
	{}

	constexpr bool is_eq(const graph_absd& other) const {
		if(g==other.g) return true;
		if(g->size() != other.g->size() || g->children.size() != other.g->children.size()) return false;
		for(auto i=0;i<g->children.size();++i) {
			if(g->children[i].name != other.g->children[i].name) return false;
			graph_absd l(f, g->children[i].vertex);
			graph_absd r(f, other.g->children[i].vertex);
			if(!l.is_eq(r)) return false;
		}
		for(auto& name:g->fields()) if(g->field(name)!=other.g->field(name)) return false;
		return true;
	}
	constexpr bool is_array() const {
		return g->is_array();
	}
	constexpr auto at(int_t k) {
		if(!g->is_array() || k > g->children.size()) return data_type{};
		return data_type::mk(graph_absd(f, g->children[k].vertex));
	}
	constexpr auto at(const data_type& k) {
		if(k.is_string()) {
			for(auto& i:g->children) if(i.name == (str_t) k) return data_type::mk(graph_absd(f, i.vertex));
			if((str_t)k=="__parent") return g->parent ? data_type::mk(graph_absd(f, g->parent)) : data_type{};
			return g->field((str_t)k);
		}
		if(k.is_int()) return at((int_t)k);
		return data_type{};
	}
	constexpr auto size() const {
		return g->size();
	}
	constexpr bool contains(const data_type& k) const {
		for(auto&& field:g->fields()) if((str_t)k==field) return true;
		for(const auto& child:g->children) if((str_t)k == child.name) return true;
		return false;
	}
	constexpr auto keys() const {
		auto ret = f.template mk_vec<data_type>();
		for(auto&& field:g->fields()) ret.emplace_back(mk_data(f, std::move(field)));
		for(const auto& child:g->children) ret.emplace_back(mk_data(f, child.name));
		return ret;
	}
};

} // namespace ast_graph
