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

template<typename...> struct test_type_list{};
template<typename... types> struct extra_data_types_factory : factory {
	using extra_types = test_type_list<types...>;
	constexpr extra_data_types_factory() : extra_data_types_factory(factory{}) {}
	constexpr explicit extra_data_types_factory(const factory& f) : factory(f) {}
};
template<typename... types, typename factory> constexpr auto mk_data_type(const factory& f, auto&&... args) {
	using ft = extra_data_types_factory<types...>;
	return typename factory::template data_type<ft>{ft(f), std::forward<decltype(args)>(args)...};
}

} // namespace tests
