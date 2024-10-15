#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

template<
		template<typename...>class variant,
		typename base_type, template<typename>class item_wrapper,
		typename... types>
struct virtual_variant : base_type {
	template<typename...> struct type_list {};
	template<typename t> struct type_c{using type=t;};

	using factory = base_type::factory;

	template<typename type> constexpr static auto mk_content_type() {
		if constexpr (std::is_base_of_v<base_type, type>) return type_c<type>{};
		else return type_c<item_wrapper<type>>{};
	}

	template<typename... result, typename first, typename... tail>
	constexpr static auto mk_holder(type_list<first, tail...>) {
		using next = decltype(mk_content_type<first>())::type;
		if constexpr (sizeof...(tail)==0) return variant<result..., next> {};
		else return mk_holder<result..., next>(type_list<tail...>{});
	}

	using holder_type = decltype(mk_holder<>(type_list<types...>{}));

	factory f;
	holder_type holder;

	template<typename type>
	friend constexpr type& create(virtual_variant& v) {
		using cur_type = decltype(mk_content_type<type>())::type;
		auto& ret = v.holder.template emplace<cur_type>(v.f);
		v.pointer = &ret;
		if constexpr (std::is_base_of_v<base_type, type>) return ret;
		else return ret.item;
	}

	template<auto ind>
	friend constexpr auto& create(virtual_variant& v) {
		return create<__type_pack_element<ind, types...>>(v);
	}

	constexpr virtual_variant() : virtual_variant(factory{}) {}
	constexpr explicit virtual_variant(factory f) : f(std::move(f)) {
		create<__type_pack_element<0, types...>>(*this);
	}
	constexpr virtual_variant(const virtual_variant& other)
	//noexcept(noexcept(holder_type(other.holder))) //TODO: won't compile if no copy ctor
	requires requires{holder_type(other.holder);}
	: holder(other.holder) {
		exec([this](auto& v){pointer = &v;});
	}
	constexpr virtual_variant& operator=(const virtual_variant& other)
	//noexcept(noexcept(holder = other.holder)) //TODO: won't compile if no copy operator=
	requires requires{holder = other.holder;} {
		holder = other.holder;
		exec([this](auto& v){pointer = &v;});
		return *this;
	}
	constexpr virtual_variant(virtual_variant&& other) noexcept(noexcept(holder_type{std::move(other.holder)}))
			: holder(std::move(other.holder)) {
		exec([this](auto& v){pointer = &v;});
	}
	constexpr virtual_variant& operator=(virtual_variant&& other) noexcept(noexcept(holder = std::move(other.holder))) {
		holder = std::move(other.holder);
		exec([this](auto& v){pointer = &v;});
		return *this;
	}

protected:
	base_type* pointer=nullptr;
private:
	template<auto ind=0>
	constexpr void exec(auto&& fnc) {
		if constexpr (ind==sizeof...(types)) return ;
		else {
			if(holder.index()==ind) fnc(get<ind>(holder));
			else exec<ind+1>(std::forward<decltype(fnc)>(fnc));
		}
	}
};
