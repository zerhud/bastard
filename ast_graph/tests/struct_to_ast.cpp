/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include "ast_graph/field_names.hpp"
#include "ast_graph/node.hpp"

#include <variant>
#include <vector>
#include <string_view>
#include <source_location>

using namespace std::literals;

struct factory {
	using string_view = std::string_view;
	using source_location = std::source_location;

	constexpr static auto mk_vec(auto&& first, auto&&... items) {
		std::vector<std::decay_t<decltype(first)>> ret;
		ret.emplace_back(std::forward<decltype(first)>(first));
		(void)(ret.emplace_back(std::forward<decltype(items)>(items)), ...);
		return ret;
	}

	template<template<typename...>typename list, typename... types>
	constexpr auto mk_val(const list<types...>&, auto&& val) const {
		return std::variant<std::monostate, types...>{val};
	}
	template<template<typename...>typename list, typename... types>
	constexpr auto mk_val(const list<types...>&) const {
		return std::variant<std::monostate, types...>{};
	}
};

struct test_leaf{};

constexpr auto node_name(const factory&, const test_leaf*) {
	return factory::string_view{"leaf"};
}
constexpr auto node_children_count(const factory&, const test_leaf*) {
	return 1;
}
template<template<auto>class num_holder, auto num>
constexpr auto node_leaf_name(const factory&, const test_leaf*, num_holder<num>)
requires (num==0) {
	return "leafs_value";
}
template<template<auto>class num_holder, auto num>
constexpr auto node_value(const factory&, const test_leaf*, num_holder<num>) {
	return 3;
}

struct test_fields {
	int field_1=1;
	int field_2=2;
	std::vector<test_leaf> leafs;
};
template<template<auto>class num_holder, auto num>
constexpr auto node_leaf_name(const factory&, const test_fields*, num_holder<num>)
requires (num==2) {
	return "leafs_override";
}
template<template<auto>class num_holder, auto num>
constexpr auto node_leaf_value(const factory&, const test_fields*, num_holder<num>)
requires (num==1) {
	return 100;
}

static_assert(3 == ast_graph::details::ref::size<test_fields>);
static_assert(1 == ast_graph::details::ref::get<0>(test_fields{}));
static_assert(2 == [] {
	const test_fields obj;
	return ast_graph::details::ref::get<1>(obj);
}());

static_assert( contains<int>(ast_graph::details::type_list<int,char>{}) );
static_assert( !contains<double>(ast_graph::details::type_list<int,char>{}) );
static_assert( ast_graph::details::type_list<int,char>{} == push_back<char>(ast_graph::details::type_list<int>{}) );
static_assert( ast_graph::details::type_list<int,char>{} == push_front_if_not_contains(ast_graph::details::type_c<int>,ast_graph::details::type_list<char>{}) );
static_assert( ast_graph::details::type_list<int>{} == transform_uniq(ast_graph::details::type_list<int>{}) );
static_assert( ast_graph::details::type_list<int,char>{} == transform_uniq(ast_graph::details::type_list<int,char>{}) );
static_assert( ast_graph::details::type_list<int>{} == transform_uniq(ast_graph::details::type_list<int,int>{})) ;
static_assert( ast_graph::details::type_list<char,int>{} == transform_uniq(ast_graph::details::type_list<char,int,int>{}) );

static_assert( "field_1"sv == ast_graph::details::ref::field_name<factory, 0, test_fields>() );
static_assert( "leafs"sv == ast_graph::details::ref::field_name<factory, 2, test_fields>() );
static_assert( "test_leaf"sv == ast_graph::details::ref::type_name<factory, test_leaf>() );

static_assert( "leaf"sv == ast_graph::node<factory,test_leaf>{}.name() );
static_assert( "test_fields"sv == ast_graph::node<factory,test_fields>{}.name() );

static_assert( 1 == ast_graph::node<factory,test_leaf>{}.children_count() );
static_assert( 3 == ast_graph::node<factory,test_fields>{}.children_count() );

static_assert( "field_1"sv == ast_graph::node<factory,test_fields>{}.key<0>() );
static_assert( "leafs_override"sv == ast_graph::node<factory,test_fields>{}.key<2>() );

static_assert( 3 == ast_graph::node<factory,test_fields>{}.keys().size() );
static_assert( "field_1"sv == ast_graph::node<factory,test_fields>{}.keys()[0] );
static_assert( "field_2"sv == ast_graph::node<factory,test_fields>{}.keys()[1] );
static_assert( "leafs_override"sv == ast_graph::node<factory,test_fields>{}.keys()[2] );

static_assert( 1 == ast_graph::node<factory,test_leaf>{}.keys().size() );
static_assert( "leafs_value"sv == ast_graph::node<factory,test_leaf>{}.keys()[0] );

static_assert( std::is_same_v<
        std::variant<std::monostate,int,std::vector<test_leaf>>,
		decltype( ast_graph::node<factory,test_fields>{}.value("not_exists") )
		> );
static_assert( ast_graph::details::type_list<int,std::vector<test_leaf>>{} ==
		ast_graph::node<factory,test_fields>{}.value_types() ) ;

template<typename type>
constexpr auto extract_field(auto&& name) {
	type val;
	return ast_graph::node<factory, type>{{}, &val}.value(std::forward<decltype(name)>(name));
}
static_assert( 1 == get<int>(extract_field<test_fields>("field_1"sv)) );
static_assert( 2 == get<int>(extract_field<test_fields>("field_2"sv)) );
static_assert( holds_alternative<std::monostate>(extract_field<test_fields>("not_exists"sv)) );

static_assert( 3 == get<int>(extract_field<test_leaf>("leafs_value"sv)) );
static_assert( holds_alternative<std::monostate>(extract_field<test_leaf>("not_exists"sv)) );

int main(int,char**) {
	return 0;
}
