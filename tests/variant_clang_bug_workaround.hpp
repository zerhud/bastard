#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <tuple>

namespace tests {

//NOTE: there is a set of bugs in clang with variant
//      https://github.com/llvm/llvm-project/issues/107593
//TODO: check and remove this class if can compile without it
//      (the test in file tests/utils.cpp on moment when this todo was created)
template<typename... types>
struct variant_clang_bug_workaround {
	using tuple_type = std::tuple<std::decay_t<types>...>;
	decltype(sizeof...(types)) cur{0};
	tuple_type holder;

	constexpr auto index() const { return cur; }

	template<typename _type>
	constexpr auto& emplace(auto&&... args) {
		using type = std::decay_t<_type>;
		holder = tuple_type{std::decay_t<types>{std::forward<decltype(args)>(args)...}...};
		set_index_to_type<type>();
		return get<type>(holder);
	}

private:
	template<typename type>
	constexpr void set_index_to_type() {
		cur = 0;
		(void)((std::is_same_v<type, std::decay_t<types>> || (++cur,false))||...);
	}
};

template<auto ind, typename... types>
constexpr auto& get(variant_clang_bug_workaround<types...>& v) {
	return std::get<ind>(v.holder);
}

} // namespace tests
