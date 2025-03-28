/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include "tests/factory.hpp"

#include "ast_graph/graph.hpp"

#include <memory>

using namespace std::literals;
struct inner_factory : tests::factory {};

struct test_leaf{ int ff=3; };

namespace test_namespace {
struct in_namespace {};
}

constexpr auto node_name(const inner_factory&, const test_leaf*) {
	return inner_factory::string_view{"leaf"};
}
constexpr auto node_children_count(const inner_factory&, const test_leaf*) {
	return 1;
}
template<template<auto>class num_holder, auto num>
constexpr auto node_leaf_name(const inner_factory&, const test_leaf*, num_holder<num>)
requires (num==0) {
	return "leafs_value";
}
template<template<auto>class num_holder, auto num>
constexpr const auto& node_value(const inner_factory&, const test_leaf* ptr, num_holder<num>) {
	return ptr->ff;
}

struct test_fields {
	int field_1=1;
	int field_2=2;
	std::vector<test_leaf> leafs;
};
template<template<auto>class num_holder, auto num>
constexpr auto node_leaf_name(const inner_factory&, const test_fields*, num_holder<num>)
requires (num==2) {
	return "leafs_override";
}
template<template<auto>class num_holder, auto num>
constexpr auto node_leaf_value(const inner_factory&, const test_fields*, num_holder<num>)
requires (num==1) {
	return 100;
}

static_assert(3 == tref::gs::size<test_fields>);
static_assert(1 == tref::gs::get<0>(test_fields{}));
static_assert(2 == [] {
	const test_fields obj;
	return tref::gs::get<1>(obj);
}());

static_assert( contains<int>(tref::type_list<int,char>{}) );
static_assert( !contains<double>(tref::type_list<int,char>{}) );
static_assert( tref::type_list<int,char>{} == push_back<char>(tref::type_list<int>{}) );
static_assert( tref::type_list<int,char>{} == push_front_if_not_contains(tref::type_c<int>,tref::type_list<char>{}) );
static_assert( tref::type_list<int>{} == transform_uniq(tref::type_list<int>{}) );
static_assert( tref::type_list<int,char>{} == transform_uniq(tref::type_list<int,char>{}) );
static_assert( tref::type_list<int>{} == transform_uniq(tref::type_list<int,int>{})) ;
static_assert( tref::type_list<char,int>{} == transform_uniq(tref::type_list<char,int,int>{}) );

static_assert( "field_1"sv == tref::gs::field_name<inner_factory, 0, test_fields>() );
static_assert( "leafs"sv == tref::gs::field_name<inner_factory, 2, test_fields>() );
static_assert( "test_leaf"sv == tref::gs::type_name<inner_factory, test_leaf>() );

static_assert( "leaf"sv == ast_graph::node<inner_factory,test_leaf>{}.name() );
static_assert( "test_fields"sv == ast_graph::node<inner_factory,test_fields>{}.name() );
static_assert( "test_namespace::in_namespace"sv == ast_graph::node<inner_factory,test_namespace::in_namespace>{}.name() );

static_assert( 1 == ast_graph::node<inner_factory,test_leaf>{}.fields_count() );
static_assert( 2 == ast_graph::node<inner_factory,test_fields>{}.fields_count() );

static_assert( 0 == ast_graph::node<inner_factory,test_leaf>{}.children_count() );
static_assert( 1 == ast_graph::node<inner_factory,test_fields>{}.children_count() );

static_assert( "field_1"sv == ast_graph::node<inner_factory,test_fields>{}.key<0>() );
static_assert( "leafs_override"sv == ast_graph::node<inner_factory,test_fields>{}.key<2>() );

static_assert( 2 == ast_graph::node<inner_factory,test_fields>{}.list_fields().size() );
static_assert( "field_2" == ast_graph::node<inner_factory,test_fields>{}.list_fields()[1] );
static_assert( 1 == ast_graph::node<inner_factory,test_fields>{}.list_children().size() );
static_assert( "leafs_override"sv == ast_graph::node<inner_factory,test_fields>{}.list_children()[0] );

static_assert( 1 == ast_graph::node<inner_factory,test_leaf>{}.list_fields().size() );
static_assert( 0 == ast_graph::node<inner_factory,test_leaf>{}.list_children().size() );

static_assert( std::is_same_v<
        std::variant<std::monostate,int,std::vector<test_leaf>>,
		decltype( ast_graph::node<inner_factory,test_fields>{}.value("not_exists") )
		> );
static_assert( tref::type_list<int,std::vector<test_leaf>>{} ==
		ast_graph::node<inner_factory,test_fields>{}.value_types() ) ;

template<typename type>
constexpr auto extract_field(auto&& name) {
	type val;
	return ast_graph::node<inner_factory, type>{{}, &val}.value(std::forward<decltype(name)>(name));
}
static_assert( 1 == get<int>(extract_field<test_fields>("field_1"sv)) );
static_assert( 2 == get<int>(extract_field<test_fields>("field_2"sv)) );
static_assert( holds_alternative<std::monostate>(extract_field<test_fields>("not_exists"sv)) );

static_assert( 3 == get<int>(extract_field<test_leaf>("leafs_value"sv)) );
static_assert( holds_alternative<std::monostate>(extract_field<test_leaf>("not_exists"sv)) );

struct test_with_ptr {
	int f1=1;
	std::unique_ptr<test_with_ptr> f2;
};

static_assert( 2 == tref::gs::size<test_with_ptr> );
static_assert( 1 == ast_graph::node<inner_factory,test_with_ptr>{}.fields_count() );
static_assert( 1 == ast_graph::node<inner_factory,test_with_ptr>{}.children_count() );
static_assert( 1 == ast_graph::node<inner_factory,test_with_ptr>{}.list_children().size() );
static_assert( "f2"sv == ast_graph::node<inner_factory,test_with_ptr>{}.list_children()[0] );

int main(int,char**) {
}
