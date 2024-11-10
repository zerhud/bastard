/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"
#include "ascip.hpp"

#include "jinja.hpp"

struct test_expr : std::variant<int, bool> {
	constexpr static auto mk_parser() {
		using p = ascip<std::tuple>;
		return p::int_ | (as<true>(p::template lit<"true">)|as<false>(p::template lit<"false">));
	}
};

struct factory : tests::factory {
	using parser = ascip<std::tuple>;
	using jinja_expression = test_expr;
};

using parser = factory::parser;

int main(int,char**) {
	return 0;
}