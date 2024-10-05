/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include "ast_graph/node.hpp"
#include "ast_graph/absd_object.hpp"

#include "absd.hpp"

#include "factory.hpp"

#include <vector>
#include <memory>
#include <string>
#include <variant>
#include <string_view>
#include <source_location>

using namespace std::literals;

struct absd_factory {
	template<typename... types> using variant = std::variant<types...>;
	template<typename type> using vector = std::vector<type>;
	using float_point_t = double;
	using string_t = std::string;
	using empty_t = std::monostate;

	template<typename type> constexpr static auto mk_vec(){ return std::vector<type>{}; }
//	template<typename key, typename value>
//	constexpr static auto mk_map(){ return map_t<key,value>{}; }
	constexpr static auto mk_ptr(auto d) { return std::make_unique<decltype(d)>( std::move(d) ); }
	constexpr static void deallocate(auto* ptr) noexcept { delete ptr; }
	template<typename interface>
	[[noreturn]] constexpr static void throw_wrong_interface_error() {
		throw std::runtime_error("cannot perform operation "s + interface::describe_with_chars());
	}
	template<auto cnt>
	[[noreturn]] constexpr static void throw_wrong_parameters_count() {
		throw std::runtime_error("wrong arguments count: " + std::to_string(cnt));
	}
};
using absd_data = absd::data<absd_factory>;

struct graph_factory : ast_graph_tests::factory {
	using data_type = absd::data<absd_factory>;
};

constexpr auto mk_data(const graph_factory& f, std::string_view d) {
	using data_type = graph_factory::data_type;
	return data_type{ data_type::string_t{d} };
}


constexpr auto to_str(const absd_data& d) {
	std::string ret;
	back_insert_format(std::back_inserter(ret), d);
	return ret;
}

struct simple_node {
	int f1 = 4;
	std::string f2 = "str";
};

struct with_subnode {
	int f1 = 3;
	std::optional<int> f2;
	simple_node sub1;
	std::vector<simple_node> sub2{ {}, {} };
	std::unique_ptr<with_subnode> sub3;
	constexpr static auto struct_fields_count() { return 5; }
};

static_assert( tref::smart_ptr<std::unique_ptr<simple_node>> );

template<typename type>
constexpr auto test(auto&& fnc, auto&& tune) {
	type obj;
	tune(obj);
	graph_factory f;
	ast_graph::ast_vertex_holder<graph_factory, type> v( f, obj );
	ast_graph::node<graph_factory, type> node{{}, &obj};
	return fnc(absd_data::mk(ast_graph::absd_object{ f, &v }));
}
template<typename type>
constexpr auto test(auto&& fnc) {
	return test<type>(std::forward<decltype(fnc)>(fnc), [](auto&){});
}

using data_type = graph_factory::data_type;

constexpr void test_objects() {
	CTRT( test<simple_node>([](auto obj){
		return obj.is_object() + obj.size();
	}) == 3);
	CTRT( test<simple_node>([](auto obj){
		return (data_type::integer_t)obj[data_type{"f1"}];
	}) == 4);
	CTRT( test<simple_node>([](auto obj){
		return (data_type::string_t)obj[data_type{"f2"}];
	}) == "str"sv);
	CTRT( test<simple_node>([](auto obj){
		return obj[data_type{"not_exists"}].is_none();
	}) == true);
	CTRT( test<simple_node>([](auto obj){
		return obj.keys().size();
	}) == 2);
	CTRT( test<simple_node>([](auto obj){
		return obj.contains(data_type{"f1"});
	}) == true);
	CTRT( test<with_subnode>([](auto obj){
		return obj.size();
	}) == 2);
}

struct opt_test {
	int f1=3;
	std::optional<int> f2=7;
	constexpr static auto struct_fields_count() { return 2; }
};

int main(int,char**) {
	using namespace std::literals;

	test_objects();

	CTRT(( []{
		opt_test obj;
		ast_graph::ast_vertex_holder v(graph_factory{}, obj);
		ast_graph::absd_object absd_obj{ graph_factory{}, &v };
		auto data = absd_data::mk(absd_obj);
		return to_str( data );
	}() == "{'f1':3,'f2':7}"sv ));

	opt_test obj;

	ast_graph::node<graph_factory, opt_test> node{{}, &obj};
	std::cout << "node: ";
	visit([](const auto& v){ if constexpr(requires{ std::cout << v;}) std::cout << v; }, node.value("f1"));
	std::cout << std::endl;

	ast_graph::ast_vertex_holder v(graph_factory{}, obj);
	ast_graph::absd_object absd_obj{ graph_factory{}, &v };
	auto data = absd_data::mk(absd_obj);
	std::cout << "we have ("sv << data.is_array() << ") "sv << to_str( data ) << std::endl;

	return 0;
}
