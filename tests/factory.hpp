#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "absd_factory.hpp"
#include "common_factory.hpp"
#include "jiexpr_factory.hpp"
#include "parser_factory.hpp"
#include "graph_factory.hpp"

namespace tests {

struct factory
		: common_factory
		, absd_factory
		, jiexpr_factory
		, parser_factory
		, graph_factory
{ };

} // namespace tests
