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
	field_vec_type fields;

	constexpr explicit absd_object(ast_graph::node<factory, origin> n)
			: node(std::move(n)) {
		extract_fields();
	}

	constexpr auto at(const data_type& k) {
		for(auto& f:fields) if(k==f.name) return f.value;
		return data_type{};
	}
	constexpr auto size() const { return node.fields_count(); }
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
		node.for_each_field_value([this](auto&& n, const auto& v) {
			constexpr bool is_optional = requires{ static_cast<bool>(v);*v; };
			if constexpr (!is_optional)
				fields.emplace_back(field_info{data_type{typename data_type::string_t{n}},data_type{v}});
			else {
				if(v) fields.emplace_back(field_info{data_type{typename data_type::string_t{n}},data_type{v.value()}});
				else fields.emplace_back(field_info{data_type{typename data_type::string_t{n}},data_type{}});
			}
		});
	}
};

} // namespace ast_graph
