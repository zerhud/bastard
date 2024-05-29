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
#include "ast_graph/graph.hpp"
#include "ast_graph/te_graph.hpp"

#include "absd.hpp"

#include "factory.hpp"

#include <string>
#include <vector>

static_assert( ast_graph::variant< std::variant<int,char> > );
static_assert( !ast_graph::variant< std::vector<int> > );
static_assert( !ast_graph::variant< std::tuple<int> > );

namespace test_namespace {

struct factory : ast_graph_tests::factory {
	using string_t = std::string;
	using empty_t = std::monostate;
	constexpr static auto mk_node(const auto& obj) {
		return ast_graph::mk_node(factory{}, obj);
	}
	constexpr static auto mk_ptr(auto d) { return std::make_unique<decltype(d)>( std::move(d) ); }
	constexpr static void deallocate(auto* ptr) noexcept { delete ptr; }
};

struct te_factory {
	using data_type = absd::data<factory>;
	using name_view = std::string_view;
	using string_view = std::string_view;
	using source_location = std::source_location;

	template<typename type>
	constexpr static bool is_field_type() {
		return std::is_integral_v<type> || std::is_same_v<type, std::string>;
	}

	constexpr auto mk_data_vec(auto sz) const {
		std::vector<std::byte> ret{};
		ret.resize(sz);
		return ret;
	}

	template<typename type>
	constexpr std::unique_ptr<type> mk_ptr() const { return std::make_unique<type>(); }
	constexpr auto mk_ptr(auto&& v) const {
		using type = std::decay_t<decltype(v)>;
		return std::make_unique<type>(std::forward<decltype(v)>(v));
	}

	template<typename type>
	constexpr auto mk_vec() const { return std::vector<type>{}; }
};

struct single_entity2 {
	int f0 = 0;
};

struct single_entity3 {
	int f0 = 0;
};
struct single_entity4 {
	int f0 = 0;
};

struct single_entity {
	int f0 = 0;
	int f1 = 1;
};

struct entity_variant {
	std::variant<int, single_entity3> evar_f0;
};

struct sub_entity {
	int f0 = 0;
	single_entity2 f1;
	std::unique_ptr<single_entity4> f2;
	std::vector<std::vector<entity_variant>> vec_vec_evar;
};

struct file {
	std::string name;
	std::vector<sub_entity> singles;
	single_entity child;
	file* rptr=nullptr;
	std::unique_ptr<file> rsptr;
};

static_assert( ast_graph::any_ptr<file*> );
static_assert( ast_graph::any_ptr<std::unique_ptr<file>> );
static_assert( ast_graph::any_ptr_to<std::unique_ptr<file>, file> );
static_assert( ast_graph::any_ptr_to<file*, file> );

static_assert( fold(ast_graph::details::type_list<int,char>{}, ast_graph::details::type_list<>{}, [](auto r, auto t){
	return push_back(push_back(r, t), ast_graph::details::type_c<int>);
}) == ast_graph::details::type_list<int,int,char,int>{} );

static_assert( []{
	file top;
	auto g = ast_graph::make_te_graph(te_factory{}, &top);
	return g.size() * !g.is_array();
}() == 5 );
static_assert( []{
	file top;
	return ast_graph::make_te_graph(te_factory{}, &top).child("child")->size();
}() == 2 );
static_assert( []{
	file top;
	top.singles.resize(3);
	auto g = ast_graph::make_te_graph(te_factory{}, &top);
	auto child = g.child("singles");
	return child->is_array() * child->children_size();
}() == 3 );
#ifndef __clang__
static_assert( []{
	std::vector<int> top;
	top.emplace_back(1); top.emplace_back(2); top.emplace_back(3);
	auto g = ast_graph::make_te_graph(te_factory{}, &top);
	return g.is_array() + (te_factory::data_type::integer_t)g.field_at(2);
}() == 4 );
static_assert( []{
	std::vector<std::string> top;
	top.emplace_back("a"); top.emplace_back("b"); top.emplace_back("c");
	auto g = ast_graph::make_te_graph(te_factory{}, &top);
	return (te_factory::data_type::string_t)g.field_at(2);
}() == "c" );
#endif

static_assert(
	ast_graph::mk_children_types(factory{}, file{}) ==
	ast_graph::details::type_list<
	        file,
			sub_entity,
			single_entity2,
			single_entity4,
			std::vector<entity_variant>,
			entity_variant,
			int,
			single_entity3,
			single_entity
			>{} );

static_assert( []{
	file top;
	auto g = ast_graph::mk_graph<file>(factory{}, top);
	return g.children.size();
}() == 4 );
static_assert( []{
	file top;
	top.singles.emplace_back();
	auto g = ast_graph::mk_graph<file>(factory{}, top);
	bool root_ok = visit([&top](auto* v){
		return &top.child == (void*)v;
	}, g.children.at(1).value->root);
	return root_ok
		* g.children.at(0).value->children.size()
		* std::holds_alternative<const ast_graph::no_value*>(g.children.at(0).value->root)
		;
}() == 1 );
static_assert( []{
	file top;
	top.singles.emplace_back();
	top.singles.back().vec_vec_evar.emplace_back().emplace_back().evar_f0.emplace<int>(42);
	auto g = ast_graph::mk_graph<file>(factory{}, top);
	auto* var_val = g.children.at(0).value->children.at(0).value->children.at(2).value->children.at(0).value->children.at(0).value.get();
	return *get<const int*>(var_val->children.at(0).value->root);
}() == 42 );

} // namespace test_namespace

void main_test() {
	using namespace test_namespace;
	CTRT( []{
		file top;
		top.name = "test";
		auto g = ast_graph::make_te_graph(te_factory{}, &top);
		auto d = g.field("name");
		return (std::string)d;
	}() == "test" );
}

int main(int,char**) {
	main_test();

	test_namespace::file top;
	top.singles.emplace_back();
	top.singles.back().vec_vec_evar.emplace_back().emplace_back().evar_f0.emplace<1>();
	auto g = ast_graph::mk_graph<decltype(top)>(test_namespace::factory{}, top);
	std::cout << g.children.at(0).value->children.at(0).value->children.at(2).value->children.at(0).value->children.at(0).value->children.at(0).name << std::endl;
	std::cout << g.children.at(0).value->children.at(0).value->children.at(2).value->children.at(0).value->children.at(0).value->children.at(0).value->children.size() << std::endl;

	auto n = ast_graph::mk_node(test_namespace::factory{}, test_namespace::file{});
	std::cout << "--" << std::endl;
	for(auto& c:n.list_fields()) std::cout << c << std::endl;
	return 0;
}
