/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "concepts.hpp"
#include "mk_children_types.hpp"
#include "te_base.hpp"

namespace ast_graph::details {

template<typename factory, typename... children>
struct te_with_index : te_graph<factory> {
};

} // namespace ast_graph::details

namespace ast_graph {

} // namespace ast_graph
