/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "node.hpp"

namespace ast_graph {

template<typename factory>
struct ast_vertex {
	using string_view = typename factory::string_view;
	struct link {
		string_view name;
		ast_vertex* vertex;
	};

	using names_type = typename factory::template vector<string_view>;
	using data_type  = typename factory::data_type;
	using link_holder = typename factory::template vector<link>;

	virtual constexpr ~ast_vertex() noexcept =default ;

	link_holder links = nullptr ;
	link_holder children = nullptr ;
	virtual const ast_vertex* parent = nullptr ;

	virtual names_type fields() const =0 ;
	virtual data_type field(string_view name) const =0 ;
};

template<typename factory, typename source>
struct ast_vertex_holder : ast_vertex<factory> {
	virtual constexpr ~ast_vertex_holder() noexcept =default ;

};

} // namespace ast_graph
