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

#include "node.hpp"
#include "graph.hpp"

namespace ast_graph {

template<typename factory>
struct absd_object {
	using data_type = typename factory::data_type;
	struct field_info {
		data_type name;
		data_type value;
	};
	using field_vec_type = decltype(factory{}.template mk_vec<field_info>());

	const ast_vertex<factory>* v;
	factory f;

	constexpr explicit absd_object(factory f, const ast_vertex<factory>* v)
	: f(std::move(f))
	, v(v)
	{
	}

	constexpr auto at(const data_type& k) {
		auto str_k = (typename data_type::string_t)k;
		return v->field(str_k);
	}
	constexpr auto size() const { return v->size(); }
	constexpr bool contains(const data_type& k) const {
		auto str_k = (typename data_type::string_t)k;
		for(auto& f:v->fields()) if(f == str_k) return true;
		return false;
	}
	constexpr auto keys() const {
		auto ret = mk_vec<data_type>(f);
		for(auto& f:v->fields()) ret.emplace_back(mk_data(this->f, f));
		return ret;
	}
};

} // namespace ast_graph
