/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include "ast_graph/field_names.hpp"

#include <string_view>
#include <source_location>

using namespace std::literals;

struct factory {
	using string_view = std::string_view;
	using source_location = std::source_location;
};

struct test_fields {
	int field_1=1;
	int field_2=2;
};

static_assert(2 == ast_graph::details::ref::size<test_fields>);
static_assert(1 == ast_graph::details::ref::get<0>(test_fields{}));
static_assert(2 == [] {
	const test_fields obj;
	return ast_graph::details::ref::get<1>(obj);
}());

static_assert( "field_1"sv == ast_graph::details::ref::field_name<factory, 0, test_fields>() );
static_assert( "field_2"sv == ast_graph::details::ref::field_name<factory, 1, test_fields>() );
static_assert( "test_fields"sv == ast_graph::details::ref::type_name<factory, test_fields>() );

int main(int,char**) {
	return 0;
}
