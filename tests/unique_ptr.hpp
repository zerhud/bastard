#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

namespace tests {

template<typename type>
struct unique_ptr {
	constexpr unique_ptr() = default ;
	constexpr unique_ptr(type* ptr) : ptr(ptr) {}
	constexpr unique_ptr(unique_ptr&& other) : ptr(other.ptr) { other.ptr = nullptr; }
	constexpr unique_ptr& operator=(unique_ptr&& other) { delete ptr; ptr = other.ptr; other.ptr = nullptr; }

	constexpr ~unique_ptr() noexcept ;

	constexpr type* get() { return ptr; }
	constexpr const type* get() const { return ptr; }

	constexpr type* operator*() { return ptr; }
	constexpr const type* operator*() const { return ptr; }

	constexpr type* operator->() { return ptr; }
	constexpr const type* operator->() const { return ptr; }
private:
	type* ptr = nullptr;
};

template<typename type>
constexpr auto make_unique(auto&&... args) {
	return unique_ptr<type>{new type{static_cast<decltype(args)&&>(args)...}};
}

} // namespace tests

#define DEFINE_UPTR_DTOR template<typename type> constexpr tests::unique_ptr<type>::~unique_ptr() noexcept { delete ptr; }
