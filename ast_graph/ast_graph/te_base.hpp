/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

namespace ast_graph {

template<typename factory>
struct te_graph {
	using data_type = typename factory::data_type;
	using name_view = typename factory::name_view;

	constexpr virtual ~te_graph() = default;

	constexpr virtual data_type field(const name_view& name) = 0;

	constexpr virtual data_type field_at(unsigned ind) = 0;

	constexpr virtual unsigned size() const = 0;

	constexpr virtual unsigned children_size() const = 0;

	constexpr virtual te_graph* child(const name_view& name) = 0;

	constexpr virtual te_graph* child_at(unsigned ind) = 0;

	constexpr virtual bool is_array() const = 0;
};

} // namespace ast_graph