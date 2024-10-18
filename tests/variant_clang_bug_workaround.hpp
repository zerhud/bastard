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
	using self_type = variant_clang_bug_workaround<types...>;
	using tuple_type = std::tuple<std::decay_t<types>...>;
	decltype(sizeof...(types)) cur{0};
	tuple_type holder;

	constexpr auto index() const { return cur; }

	template<typename _type>
	constexpr auto& emplace(auto&&... args) {
		using type = std::decay_t<_type>;
		holder = tuple_type{std::decay_t<types>{std::forward<decltype(args)>(args)...}...};
		set_index_to_type<type>();
		return std::get<type>(holder);
	}

	template<typename type, typename self>
	friend constexpr bool holds_alternative(self&& v) requires std::is_same_v<std::decay_t<self>, self_type> {
		return v.index() == index_of_type<type>();
	}
	template<typename type, typename self>
	friend constexpr auto& get(self&& v) requires std::is_same_v<std::decay_t<self>, self_type> {
		return std::get<index_of_type<type>()>(v.holder);
	}

	template<auto ind, typename self>
	friend constexpr auto& get(self&& v) requires std::is_same_v<std::decay_t<self>, self_type> {
		return std::get<ind>(v.holder);
	}

private:
	template<typename type>
	constexpr static auto index_of_type() {
		unsigned ret=0;
		(void)((std::is_same_v<type, std::decay_t<types>> || (++ret,false)) || ...);
		return ret;
	}
	template<typename type>
	constexpr void set_index_to_type() {
		cur = 0;
		(void)((std::is_same_v<type, std::decay_t<types>> || (++cur,false))||...);
	}
};

} // namespace tests
