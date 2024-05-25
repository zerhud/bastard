/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <utility>

#include "node.hpp"
#include "concepts.hpp"
#include "graph.hpp"

namespace ast_graph {

template<typename factory, typename origin>
struct absd_object2 {
	using data_type = typename factory::data_type;
	using keys_type = decltype(details::lref<factory>().template mk_vec<data_type>());
	struct base {
		constexpr virtual ~base() =default ;
		constexpr virtual data_type at(const data_type& key) const =0 ;
		constexpr virtual unsigned size() const =0 ;
		constexpr virtual bool contains(const data_type& k) const =0 ;
		constexpr virtual keys_type keys() const =0 ;
	};

	template<typename content>
	struct holder : base {
		node<factory, content> info;
		constexpr holder(node<factory, content> info) : info(info) {};
		constexpr ~holder() =default ;
		constexpr virtual data_type at(const data_type& key) const override { return data_type{}; }
		constexpr virtual unsigned size() const override { return info.fields_count(); }
		constexpr virtual bool contains(const data_type& k) const override { return false; }
		constexpr virtual keys_type keys() const override {
			return {};
		}
	};

	struct field_info {
		data_type name;
		data_type value;
	};
	using field_vec_type = decltype(factory{}.template mk_vec<field_info>());

	factory f;
	graph<factory, origin> g;
	field_vec_type store;

	base* holder_storage=nullptr;

	//struct node<factory, origin> node;

	constexpr void mk_storage() {
		exec_for_ptr(g, [this]<typename type>(const type* v){
			//TODO: all holders will be the same size, we can alloc a buffer and use placement new
			//      and move ctor and eq operator will need to call the placement new again
			holder_storage = new holder<type>{node<factory,type>{f,v}};
		});
	}

	constexpr absd_object2(absd_object2&& other) : f(other.f), g(std::move(other.g)), holder_storage(other.holder_storage) { other.holder_storage = nullptr; }
	constexpr absd_object2& operator=(absd_object2&& other) { g = std::move(other.g); holder_storage = other.holder_storage; other.holder_storage=nullptr; return *this; }
	constexpr explicit absd_object2(auto&& f, graph<factory, origin> g)
		: f(std::forward<decltype(f)>(f))
		, g(std::move(g))
		, store(f.template mk_vec<field_info>())
	{
		mk_storage();
	}

	constexpr ~absd_object2() noexcept {
		delete holder_storage;
	}

	constexpr auto at(const data_type& k) {
		//for(auto& c:g.children) if(c.name==(typename data_type::string_t)k) data_type::mk(absd_object2( f, *c.value ));
		return data_type{};
	}
	constexpr auto size() const {
		return holder_storage ? holder_storage->size() : g.children.size() + fields_count(g);
	}
	constexpr bool contains(const data_type& k) const { return false; }
	constexpr auto keys() const {
		auto ret = f.template mk_vec<data_type>();
		for(auto& c:g.children) ret.emplace_back(data_type{ typename data_type::string_t{ c.name } });
		return ret;
	}
};

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
	template<smart_ptr value_type>
	constexpr auto create_absd_from_object(const value_type& v) const {
		return v ? create_absd_from_object(*v) : empty;
	}
	template<vector value_type>
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
