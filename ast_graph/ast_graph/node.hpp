#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <tref.hpp>

namespace ast_graph {

template<typename factory, typename origin>
struct node {
	using self_type = node<factory, origin>;

	factory f;
	const origin* ptr=nullptr;

	constexpr auto name() const {
		if constexpr (requires{ node_name(f, ptr); })
			return node_name(f, ptr);
		else return tref::gs::type_name<factory, origin>();
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
		return list_fields(factory{});
	}
	constexpr static auto list_fields(const auto& f) {
		return keys_impl<0, struct_children_count()>(f, []<auto ind>{
			return factory::template is_field_type<field_type<ind>>();
		});
	}
	constexpr static auto list_children() {
		return list_children(factory{});
	}
	constexpr static auto list_children(const auto& f) {
		return keys_impl<0, struct_children_count()>(f, []<auto ind>{
			return !factory::template is_field_type<field_type<ind>>();
		});
	}

	template<auto ind> constexpr static auto key() {
		if constexpr(requires{node_leaf_name(factory{}, static_cast<const origin*>(nullptr), tref::object_c<ind>);})
			return node_leaf_name(factory{}, static_cast<const origin*>(nullptr), tref::object_c<ind>);
		else return tref::gs::field_name<factory, ind, origin>();
	}

	constexpr auto value(auto&& name) const {
		return value_impl<0, struct_children_count()>(name);
	}
	constexpr auto field_value(auto&& name) const {
		return field_value_impl<0, struct_children_count()>(name);
	}

	constexpr static auto value_types() {
		return key_types_impl<0, struct_children_count()>([]<typename>{return true;});
	}
	constexpr static auto field_value_types() {
		return key_types_impl<0, struct_children_count()>([]<typename t>{return factory::template is_field_type<t>();});
	}

	constexpr static auto children_types() {
		return key_types_impl<0, struct_children_count()>([]<typename type>{
			return !factory::template is_field_type<type>();
		});
	}

private:
	template<auto ind>
	constexpr static auto& st_value(const auto& f, const auto* ptr) {
		constexpr bool value_overloaded = requires{ node_value(f, ptr, tref::object_c<ind>); };
		if constexpr (value_overloaded) return node_value(f, ptr, tref::object_c<ind>);
		else return tref::gs::get<ind>(*ptr);
	}

	template<auto ind> using field_type = decltype(
			tref::decay(
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
		static_assert( !tref::vector<origin> );
		if constexpr (requires{ node_children_count(factory{}, static_cast<const origin*>(nullptr)); })
			return node_children_count(factory{}, static_cast<const origin*>(nullptr));
		else return tref::gs::size<origin>;
	}

	template<auto cur, auto size>
	constexpr static auto keys_impl(const auto& f, auto&& check, auto&&... args) {
		if constexpr (cur==size) return mk_vec
		    <decltype(tref::lref<self_type>().template key<0>())>
			(f, static_cast<decltype(args)&&>(args)...);
		else return check.template operator()<cur>()
			? keys_impl<cur+1,size>(f,
					static_cast<decltype(check)&&>(check),
					static_cast<decltype(args)&&>(args)...,
					key<cur>())
			: keys_impl<cur+1,size>(f,
					static_cast<decltype(check)&&>(check),
					static_cast<decltype(args)&&>(args)...)
			;
	}
	template<auto cur, auto size, typename... types, typename fnc>
	constexpr static auto key_types_impl(const fnc& _fnc) {
		if constexpr (cur==size) return transform_uniq(tref::type_list<types...>{});
		else {
			using next_type = tref::decay_t<decltype(node{}.value<cur>())>;
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
	constexpr auto value_impl(auto&& request) const {
		if constexpr (cur==size) return mk_graph_node_field_value(f, value_types());
		else return request == key<cur>()
		    ? mk_graph_node_field_value(f, value_types(), value<cur>())
		    : value_impl<cur+1, size>(request)
		    ;
	}
	template<auto cur, auto size>
	constexpr auto field_value_impl(auto&& request) const {
		if constexpr (cur==size) return mk_graph_node_field_value(f, field_value_types());
		else if constexpr (!contains<field_type<cur>>(field_value_types())) return field_value_impl<cur+1, size>(request);
		else return request == key<cur>()
		            ? mk_graph_node_field_value(f, field_value_types(), value<cur>())
		            : field_value_impl<cur+1, size>(request)
					;
	}
};

template<typename factory, typename origin>
constexpr auto mk_node(const factory& f, const origin& obj) {
	return node<factory, origin>{ f, &obj };
}

} // namespace ast_graph
