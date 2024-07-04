/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "ast_graph/mk_children_types.hpp"
#include "factory.hpp"

#include <memory>
#include <vector>

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

static_assert( tref::vector<std::vector<std::vector<entity_variant>>> );

struct file {
	std::string name;
	std::vector<sub_entity> singles;
	single_entity child;
	file* rptr=nullptr;
	std::unique_ptr<file> rsptr;
};

static_assert( tref::vector<std::vector<sub_entity>> );

static_assert(
		ast_graph::mk_children_types(ast_graph_tests::factory{}, file{}) ==
		tref::type_list<
				file,
				std::vector<sub_entity>,
				sub_entity,
				single_entity2,
				single_entity4,
				std::vector<std::vector<entity_variant>>,
				std::vector<entity_variant>,
				entity_variant,
				int,
				single_entity3,
				single_entity
		>{} );

static_assert( tref::any_ptr<file*> );
static_assert( tref::any_ptr<std::unique_ptr<file>> );
static_assert( tref::any_ptr_to<std::unique_ptr<file>, file> );
static_assert( tref::any_ptr_to<file*, file> );

static_assert( fold(tref::type_list<int,char>{}, tref::type_list<>{}, [](auto r, auto t){
	return push_back(push_back(r, t), tref::type_c<int>);
}) == tref::type_list<int,int,char,int>{} );

int main(int,char**) {
	return 0;
}
