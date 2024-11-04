#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <memory>
#include <vector>
#include <unordered_set>

namespace tests {

struct common_factory {
	template<typename _t> struct _type_c { using type = _t; };
	template<typename type> constexpr static auto mk_set_type() {
		// can't use unordered_set due it has no constexpr ctor
		// should to be return _type_c<std::unordered_set<type>>{};
		//
		// can't check by consteval: an inconsistent deduction return
		// type if call in static_assert and in rt test
		return _type_c<std::vector<type>>{};
	}
};

template<typename type>
constexpr auto* allocate(const common_factory& f, auto&&... args) {
	return new type{ std::forward<decltype(args)>(args)... };
}

constexpr void deallocate(const common_factory& f, auto* ptr) {
	delete ptr;
}

constexpr auto mk_ptr(const common_factory&, auto d) {
	return std::make_unique<decltype(d)>( std::move(d) );
}

template<typename type> constexpr auto mk_vec(const common_factory&) {
	return std::vector<type>{};
}

template<typename type> constexpr auto mk_vec(const common_factory&, auto&&... items) {
	std::vector<type> ret{};
	if constexpr(sizeof...(items) > 0)
		(void)(ret.emplace_back(std::forward<decltype(items)>(items)), ...);
	return ret;
}

template<typename type> constexpr auto mk_set(const common_factory& f, auto&&... items) {
	using set_type = decltype(f.mk_set_type<type>())::type;
	set_type ret{};
	if constexpr (sizeof...(items) > 0) {
		auto first = [](auto&& first, auto&&...){return std::forward<decltype(first)>(first);};
		if constexpr( requires{ret.emplace_back(first(std::forward<decltype(items)>(items)...));} )
			(void)(ret.emplace_back(std::forward<decltype(items)>(items)),...);
		else if constexpr ( requires{ret.empalce(first(std::forward<decltype(items)>(items)...));})
			(void)(ret.emplace(std::forward<decltype(items)>(items)), ...);
		else static_assert( false, "emplace items: operation not supported");
	}
	return ret;
}

} // namespace tests
