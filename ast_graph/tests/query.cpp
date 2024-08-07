/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "ast_graph/query.hpp"
#include "factory.hpp"
#include "ascip.hpp"

struct factory : ast_graph_tests::query_factory{ };

int main(int,char**) {

	using query_type = ast_graph::details::query<factory>;
	using ident_type = query_type::ident_type;
	using name_eq_type = query_type::name_eq_type;

	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "{a==1}");
		auto& q = get<0>(get<0>(result.data).data);
		auto& val = get<ident_type>(*get<0>(q).left).val;
		return (val.size()==1) + 2*(val[0]=='a') + 4*result.to_output + 8*(result.input_number==0);
	}() == 15);
	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "{a:41}");
		auto& q = get<0>(get<0>(result.data).data);
		auto& val = get<ident_type>(*get<0>(q).left).val;
		return (get<factory::integer_type>(*get<0>(q).right)==41) + 2*(val.size()==1) + 4*(result.next==nullptr);
	}() == 7);
	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "3!{a:41}-[a:b]->{}");
		auto& q = get<0>(get<0>(result.data).data);
		auto& val = get<ident_type>(*get<0>(q).left).val;
		return (get<factory::integer_type>(*get<0>(q).right)==41) + 2*(val.size()==1) + 4*(result.next!=nullptr);
	}() == 7);
	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "{:42}");
		auto& vertex = get<0>(result.data).data;
		auto& val = get<name_eq_type>(vertex).val;
		return get<factory::integer_type>(*val);
	}() == 42);
	static_assert(  []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "3{:ident}-[a==b]->{}");
		auto& vertex1 = get<0>(result.data).data;
		auto& edge = get<2>(result.next->data).data;
		auto& vertex2 = get<0>(result.next->next->data).data;
		return 
			(get<ident_type>(*get<name_eq_type>(vertex1).val).val=="ident")
			+ 2*(get<ident_type>(*get<0>(get<0>(edge)).left).val=="a")
			+ 4*(get<ident_type>(*get<0>(get<0>(edge)).right).val=="b")
			+ 8*(get<0>(get<0>(vertex2)).left==nullptr)
			+ 16*(get<0>(get<0>(vertex2)).right==nullptr)
			;
	}() == 31);

	static_assert( []{
		auto result = ast_graph::details::parse_query<factory, ascip<std::tuple, factory>>(factory{}, "{:42}");
		return 1;
	}() == 1 );

	return 0;
}
