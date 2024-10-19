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
	using self_type = virtual_variant<variant, base_type, item_wrapper, types...>;

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
	constexpr explicit virtual_variant(factory f)
	: f(std::move(f))
	, pointer(&get<0>(holder))
	{
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

	template<typename type, typename cur_self_type>
	friend constexpr auto&& get(cur_self_type&& v) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		using inner_type = decltype(mk_content_type<type>())::type;
		return get<inner_type>(v.holder);
	}
	template<auto ind, typename cur_self_type>
	friend constexpr auto&& get(cur_self_type&& v) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		return get<__type_pack_element<ind, types...>>(v);
	}

	template<typename type, typename cur_self_type>
	friend constexpr bool holds_alternative(cur_self_type&& v) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		using inner_type = decltype(mk_content_type<type>())::type;
		return holds_alternative<inner_type>(v.holder);
	}
	template<auto ind, typename cur_self_type>
	friend constexpr bool holds_alternative(cur_self_type&& v) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		return holds_alternative<__type_pack_element<ind, types...>>(v);
	}

	template<typename type, typename cur_self_type>
	friend constexpr auto first_index_of(cur_self_type&&) requires std::is_base_of_v<self_type, std::decay_t<cur_self_type>> {
		static_assert( ((std::is_base_of_v<type, types>||std::is_same_v<type,types>) + ... ) > 0, "there is no such type" );
		decltype(sizeof...(types)) ind=0;
		(void)( ((std::is_base_of_v<type, types>||std::is_same_v<type,types>)||(++ind,false)) || ... );
		return ind;
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
