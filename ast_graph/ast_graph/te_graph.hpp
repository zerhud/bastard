/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include "concepts.hpp"
#include "mk_children_types.hpp"
#include <memory> // placement new works only in std namespace ))

namespace ast_graph {

template<typename factory>
struct te_graph {
	using data_type = typename factory::data_type;
	using name_view = typename factory::name_view;

	constexpr virtual ~te_graph() =default ;

	constexpr virtual data_type field(const name_view& name) =0 ;
	constexpr virtual data_type field_at(unsigned ind) =0 ;
	constexpr virtual unsigned size() const = 0;
	constexpr virtual unsigned children_size() const = 0;
	constexpr virtual te_graph* child(const name_view& name) =0 ;
	constexpr virtual te_graph* child_at(unsigned ind) =0 ;
	constexpr virtual bool is_array() const =0 ;
};

template<typename factory, vector origin>
constexpr auto make_te_graph(const factory& f, const origin* o) ;
template<typename factory, typename origin>
constexpr auto make_te_graph(const factory& f, const origin* o) ;

template<typename type, typename origin, typename factory>
auto make_te_graph_vec_value_type(const factory& f) requires (factory::template is_field_type<type>()) {
	using data_type = typename factory::data_type;
	struct holder {
		using vec_type = decltype(factory{}.template mk_vec<data_type>());
		vec_type children;
		constexpr holder(const factory& f, const origin* o) : children(f.template mk_vec<data_type>()) {
			children.reserve(o->size());
			for(auto&& i:*o) {
				if constexpr (requires{data_type{&i};})
					children.emplace_back(data_type{&i});
				else children.emplace_back(data_type{i});
			}
		}
	};
	return details::type_c<holder>;
}
template<typename type, typename origin, typename factory>
auto make_te_graph_vec_value_type(const factory& f) requires (!factory::template is_field_type<type>()) {
	using data_type = typename factory::data_type;
	struct holder {
		using base = te_graph<factory>;
		using child_type = decltype(factory{}.template mk_ptr<base>());
		using vec_type = decltype(factory{}.template mk_vec<child_type>());

		vec_type children;
		constexpr holder(const factory& f, const origin* o) : children(f.template mk_vec<child_type>()) {
			for(auto&& i:*o) {
				auto val = make_te_graph(f, &i);
				children.emplace_back( f.mk_ptr(std::move(val)) );
			}
		}
	};
	return details::type_c<holder>;
}

template<typename factory, vector origin>
constexpr auto make_te_graph(const factory& f, const origin* o) {
	using value_type = origin::value_type;
	using holder_type = typename decltype(make_te_graph_vec_value_type<value_type, origin>(factory{}))::type;
	struct te : te_graph<factory>, holder_type {
		using base = te_graph<factory>;
		using typename base::data_type;
		using typename base::name_view;

		factory f;

		constexpr te(const factory& f, const origin* o) : holder_type(f, o), f(f) { }
		constexpr bool is_array() const override { return true; }
		constexpr data_type field(const name_view& name) override {
			data_type ret;
			return ret;
		}
		constexpr unsigned size() const override { return this->children.size(); }
		constexpr unsigned children_size() const override { return this->children.size(); }
		constexpr base* child(const name_view& name) override { return nullptr; }
		constexpr base* child_at(unsigned ind) override {
			if constexpr (factory::template is_field_type<value_type>()) return nullptr;
			else return this->children.at(ind).get();
		}
		constexpr data_type field_at(unsigned ind) override {
			if constexpr (!factory::template is_field_type<value_type>()) return data_type{};
			else return this->children.at(ind);
		}
	};
	return te{f, o};
}
template<typename factory, typename origin>
constexpr auto make_te_graph(const factory& f, const origin* o) {
	//TODO: we can create the same type using void* and index in children types table
	//      and we cannot store it in a vector of children, only in heap. and after
	//      the std::start_lifetime_as will be supported we can store the children
	//      in vector of bytes (we know children size in compile time, excluding vec).
	struct te : te_graph<factory> {
		using base = te_graph<factory>;
		using typename base::data_type;
		using typename base::name_view;
		using child_type = decltype(factory{}.template mk_ptr<base>());

		struct child_info {
			name_view name;
			child_type child;
		};

		using children_type = decltype(factory{}.template mk_vec<child_info>());

		factory f;
		struct node<factory, origin> node;
		children_type children;
		constexpr te(const factory& _f, const origin* o)
			: f(_f)
			, node(f, o)
			, children(f.template mk_vec<child_info>())
		{
			node.for_each_child_value([&]<typename cur_type>(auto&& cur_name, const cur_type& cur_val)mutable{
				if constexpr (any_ptr<cur_type> || variant<cur_type>) return;
				else {
					auto val = make_te_graph(f, &cur_val);
					children.emplace_back(child_info{name_view{cur_name}, f.mk_ptr(std::move(val))});
				}
			});
		}

		constexpr bool is_array() const override { return false; }
		constexpr data_type field_at(unsigned) override { return data_type{}; }
		constexpr data_type field(const name_view& name) override {
			data_type ret;
			node.for_each_field_value([&](auto&& cur_name, const auto& val){
				if(cur_name==name) ret = data_type{val};
			});
			return ret;
		}
		constexpr unsigned size() const override { return node.fields_count() + node.children_count(); }
		constexpr unsigned children_size() const override { return node.children_count(); }
		constexpr base* child_at(unsigned) override { return nullptr; }
		constexpr base* child(const name_view& name) override {
			base* ret = nullptr;
			for(auto& i:children) if(i.name == name) ret = i.child.get();
			return ret;
		}
	};
	te ret{ f, o };
	return ret;
}

} // namespace ast_graph
