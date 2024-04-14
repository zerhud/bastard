#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "field_names.hpp"

namespace ast_graph {

template<typename factory, typename origin>
struct node {
	factory f;
	const origin* ptr=nullptr;

	constexpr auto name() const {
		if constexpr (requires{ node_name(f, ptr); })
			return node_name(f, ptr);
		else return details::ref::type_name<factory, origin>();
	}

	constexpr static auto children_count() {
		if constexpr (requires{ node_children_count(factory{}, static_cast<const origin*>(nullptr)); })
			return node_children_count(factory{}, static_cast<const origin*>(nullptr));
		else return details::ref::size<origin>;
	}

	template<auto ind> constexpr auto key() const {
		if constexpr(requires{node_leaf_name(f, ptr, details::object_c<ind>);})
			return node_leaf_name(f, ptr, details::object_c<ind>);
		else return details::ref::field_name<factory, ind, origin>();
	}

	constexpr auto keys() const {
		return keys_impl<0, children_count()>();
	}

	constexpr auto value(auto&& name) const {
		return value_impl<0, children_count()>(name);
	}

	constexpr static auto value_types() {
		return key_types_impl<0, children_count()>();
	}
private:
	template<auto cur, auto size>
	constexpr auto keys_impl(auto&&... args) const {
		if constexpr (cur==size) return f.mk_vec(static_cast<decltype(args)&&>(args)...);
		else return keys_impl<cur+1,size>(static_cast<decltype(args)&&>(args)..., key<cur>());
	}
	template<auto cur, auto size, typename... types>
	constexpr static auto key_types_impl() {
		if constexpr (cur==size) return transform_uniq(details::type_list<types...>{});
		else return key_types_impl<
		        cur+1,size,types...,
				details::ref::decay_t<decltype(node{}.value<cur>())>
				>();
	}
	template<auto ind>
	constexpr auto value() const {
		constexpr bool value_overloaded = requires{ node_value(f, ptr, details::object_c<ind>); };
		if constexpr (value_overloaded) return node_value(f, ptr, details::object_c<ind>);
		else return details::ref::get<ind>(*ptr);
	}
	template<auto cur, auto size>
	constexpr auto value_impl(auto&& request) const {
		if constexpr (cur==size) return f.mk_val(value_types());
		else return request == key<cur>()
		    ? f.mk_val(value_types(), value<cur>())
			: value_impl<cur+1, size>(request)
			;
	}
};

} // namespace ast_graph
