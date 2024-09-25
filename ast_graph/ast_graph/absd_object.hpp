/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <utility>
#include <tref.hpp>

#include "node.hpp"

namespace ast_graph {

//TODO: remove this class?
template<typename factory, typename origin>
struct absd_object {
	using data_type = typename factory::data_type;
	struct field_info {
		data_type name;
		data_type value;
	};
	using field_vec_type = decltype(factory{}.template mk_vec<field_info>());

	struct node<factory, origin> node;
	[[no_unique_address]] factory f;
	data_type empty;
	field_vec_type fields, subs;

	constexpr explicit absd_object(ast_graph::node<factory, origin> n)
			: node(std::move(n)) {
		extract_fields();
		extract_subs();
	}

	constexpr auto& at(const data_type& k) {
		for(auto& f:fields) if(k==f.name) return f.value;
		for(auto& f:subs) if(k==f.name) return f.value;
		return empty;
	}
	constexpr auto size() const { return node.children_count() + node.fields_count(); }
	constexpr bool contains(const data_type& k) const {
		for(auto& f:fields) if(f.name == k) return true;
		return false;
	}
	constexpr auto keys() const {
		auto ret = f.template mk_vec<data_type>();
		for(auto& f:fields) ret.emplace_back(f.name);
		return ret;
	}
private:
	constexpr void extract_fields() {
		node.for_each_field_value([this](auto&& n, const auto& v){
			fields.emplace_back(field_info{data_type{typename data_type::string_t{n}},data_type{v}});
		});
	}
	constexpr void extract_subs() {
		node.for_each_child_value([this]<typename vt>(auto&& n, const vt& v){
			add_to_subs(n, create_absd_from_object(v));
		});
	}
	template<tref::smart_ptr value_type>
	constexpr auto create_absd_from_object(const value_type& v) const {
		return v ? create_absd_from_object(*v) : empty;
	}
	template<tref::vector value_type>
	constexpr auto create_absd_from_object(const value_type& v) const {
		using val_t = typename value_type::value_type;
		data_type val;
		val.mk_empty_array();
		for(const auto& i:v) {
			absd_object<factory, val_t> obj{ ast_graph::node<factory, val_t>{f, &i} };
			val.push_back(data_type::mk(std::move(obj)));
		}
		return val;
	}
	template<typename value_type>
	constexpr auto create_absd_from_object(const value_type& v) const {
		ast_graph::node<factory, value_type> node{f, &v};
		absd_object<factory, value_type> obj{ std::move(node) };
		return data_type::mk(std::move(obj));
	}
	constexpr void add_to_subs(auto&& n, auto&& v) {
		subs.emplace_back(field_info{data_type{typename data_type::string_t{n}}, std::forward<decltype(v)>(v)});
	}
};

} // namespace ast_graph
