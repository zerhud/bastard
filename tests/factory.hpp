#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <variant>

#include "absd_factory.hpp"
#include "common_factory.hpp"
#include "jiexpr_factory.hpp"
#include "parser_factory.hpp"
#include "graph_factory.hpp"

#ifdef __clang__
#include "tests/variant_clang_bug_workaround.hpp"
#endif

#include "virtual_variant.hpp"

namespace tests {

template<typename...> struct test_type_list{};

struct factory
		: common_factory
		, absd_factory
		, jiexpr_factory
		, parser_factory
		, graph_factory
{
	template<
			typename base_type,
			template<typename>class wrapper, typename... types
	> using virtual_variant_t = virtual_variant<variant_t, base_type, wrapper, types...>;
};

struct variant_workaround_factory : factory {
#ifdef __clang__
	template<typename... types> using variant_t = tests::variant_clang_bug_workaround<types...>;
	template<
			typename base_type,
			template<typename>class wrapper, typename... types
	> using virtual_variant_t = virtual_variant<variant_t, base_type, wrapper, types...>;
#endif
};

} // namespace tests
