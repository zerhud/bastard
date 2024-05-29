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
	using self_type = node<factory, origin>;

	factory f;
	const origin* ptr=nullptr;

	constexpr auto name() const {
		if constexpr (requires{ node_name(f, ptr); })
			return node_name(f, ptr);
		else return details::ref::type_name<factory, origin>();
	}

	constexpr void for_each_field_value(auto&& fnc) const {
		for_each_field_v<struct_children_count()>([&fnc]<typename cur>(auto&& name, const cur& v){
			if constexpr (factory::template is_field_type<cur>()) fnc(static_cast<decltype(name)>(name), v);
		});
	}
	constexpr void for_each_child_value(auto&& fnc) const {
		for_each_field_v<struct_children_count()>([&fnc]<typename cur>(auto&& name, const cur& v){
			if constexpr (!factory::template is_field_type<cur>()) fnc(static_cast<decltype(name)>(name), v);
		});
	}

	constexpr static auto fields_count() {
		return for_each_field<struct_children_count()>([]<typename... types>(){
			return (factory::template is_field_type<types>() + ...);
		});
	}
	constexpr static auto children_count() {
		static_assert( struct_children_count() - fields_count() >= 0, "internal error" );
		return struct_children_count() - fields_count();
	}

	constexpr static auto list_fields() {
		return keys_impl<0, struct_children_count()>([]<auto ind>{
			return factory::template is_field_type<field_type<ind>>();
		});
	}
	constexpr static auto list_children() {
		return keys_impl<0, struct_children_count()>([]<auto ind>{
			return !factory::template is_field_type<field_type<ind>>();
		});
	}

	template<auto ind> constexpr static auto key() {
		if constexpr(requires{node_leaf_name(factory{}, static_cast<const origin*>(nullptr), details::object_c<ind>);})
			return node_leaf_name(factory{}, static_cast<const origin*>(nullptr), details::object_c<ind>);
		else return details::ref::field_name<factory, ind, origin>();
	}

	constexpr auto* value_ptr(auto&& name) const {
		return value_ptr_impl<0, struct_children_count()>(name);
	}
	constexpr auto value(auto&& name) const {
		return value_impl<0, struct_children_count()>(name);
	}

	constexpr static auto value_types() {
		return key_types_impl<0, struct_children_count()>([]<typename>{return true;});
	}

	constexpr static auto children_types() {
		return key_types_impl<0, struct_children_count()>([]<typename type>{
			return !factory::template is_field_type<type>();
		});
	}

private:
	template<auto ind>
	constexpr static auto& st_value(const auto& f, const auto* ptr) {
		constexpr bool value_overloaded = requires{ node_value(f, ptr, details::object_c<ind>); };
		if constexpr (value_overloaded) return node_value(f, ptr, details::object_c<ind>);
		else return details::ref::get<ind>(*ptr);
	}

	template<auto ind> using field_type = decltype(
			details::decay(
					st_value<ind>(
							factory{},
							static_cast<const origin*>(nullptr)
					)
				)
			)::type;

	template<auto sz, auto cur=0, typename... types>
	constexpr void for_each_field_v(auto&& fnc) const {
		if constexpr (cur<sz) {
			fnc(key<cur>(), st_value<cur>(f, ptr));
			for_each_field_v<sz, cur+1>(static_cast<decltype(fnc)&&>(fnc));
		}
	}

	template<auto sz, auto cur=0, typename... types>
	constexpr static auto for_each_field(auto&& fnc) {
		if constexpr (cur==sz) return fnc.template operator()<types...>();
		else return for_each_field<sz, cur+1, types..., field_type<cur>>(static_cast<decltype(fnc)&&>(fnc));
	}
	constexpr static auto struct_children_count() {
		if constexpr (requires{ node_children_count(factory{}, static_cast<const origin*>(nullptr)); })
			return node_children_count(factory{}, static_cast<const origin*>(nullptr));
		else return details::ref::size<origin>;
	}

	template<auto cur, auto size>
	constexpr static auto keys_impl(auto&& check, auto&&... args) {
		if constexpr (cur==size) return factory{}.template mk_vec
		    <decltype(details::lref<self_type>().template key<0>())>
			(static_cast<decltype(args)&&>(args)...);
		else return check.template operator()<cur>()
			? keys_impl<cur+1,size>(
					static_cast<decltype(check)&&>(check),
					static_cast<decltype(args)&&>(args)...,
					key<cur>())
			: keys_impl<cur+1,size>(
					static_cast<decltype(check)&&>(check),
					static_cast<decltype(args)&&>(args)...)
			;
	}
	template<auto cur, auto size, typename... types, typename fnc>
	constexpr static auto key_types_impl(const fnc& _fnc) {
		if constexpr (cur==size) return transform_uniq(details::type_list<types...>{});
		else {
			using next_type = details::ref::decay_t<decltype(node{}.value<cur>())>;
			if constexpr (fnc{}.template operator()<next_type>())
				return key_types_impl< cur+1, size, types..., next_type>(_fnc);
			else return key_types_impl<cur+1, size, types...>(_fnc);
		}

	}
	template<auto ind>
	constexpr auto& value() const {
		return st_value<ind>(f, ptr);
	}
	template<auto cur, auto size>
	constexpr auto* value_ptr_impl(auto&& request) const {
		if constexpr (cur==size) return static_cast<void*>(nullptr);
		else return request == key<cur>()
		            ? &value<cur>()
		            : value_ptr_impl<cur+1, size>(request)
		;
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

template<typename factory, typename origin>
constexpr auto mk_node(const factory& f, const origin& obj) {
	return node<factory, origin>{ f, &obj };
}

} // namespace ast_graph
