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

template<typename factory, tref::vector source> constexpr auto mk_graph_calc_size(const factory& f, const source& src) ;
template<typename factory, tref::variant source> constexpr auto mk_graph_calc_size(const factory& f, const source& src) ;
template<typename factory, tref::any_ptr source> constexpr auto mk_graph_calc_size(const factory& f, const source& src) ;
template<typename factory, typename source_1> constexpr auto mk_graph_calc_size(const factory& f, const source_1& src) ;
template<typename factory, tref::vector source>
constexpr auto mk_graph_calc_size(const factory& f, const source& src) {
	unsigned inner_cnt = 0;
	for(const auto& i:src) inner_cnt += mk_graph_calc_size(f, i);
	return src.size() + inner_cnt;
}
template<typename factory, tref::variant source>
constexpr auto mk_graph_calc_size(const factory& f, const source& src) {
	return visit([&f](const auto& i){ return mk_graph_calc_size(f, i); }, src);
}
template<typename factory, tref::any_ptr source>
constexpr auto mk_graph_calc_size(const factory& f, const source& src) {
	return src ? mk_graph_calc_size(f, *src) : 0;
}
template<typename factory, typename source_1>
constexpr auto mk_graph_calc_size(const factory& f, const source_1& src) {
	struct node<factory, source_1> node{ f, &src };
	unsigned inner_cnt = 0;
	node.for_each_child_value([&inner_cnt,&f](auto&&, const auto& val){
		inner_cnt += mk_graph_calc_size(f, val);
	});
	return node.children_count() + inner_cnt;
}

} // namespace ast_graph
