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

struct factory : tests::factory {
	using parser = ascip<std::tuple>;
};
using tjinja = jinja<factory>;

using parser = factory::parser;

static_assert( (tjinja{factory{}}, 1), "can create" ) ;
static_assert( parse(jinja_details::content<factory>::mk_parser(), parser::make_source("ab%! ^(\\<<<%")) == 10 );
static_assert( []{
	jinja_details::content<factory> r;
	auto p = parse(r.mk_parser(), parser::make_source("\\%<<#"), r);
	return (p == 3) + 2*(r.value == "\\%<");
}() == 3 );
static_assert( []{
	jinja_details::trim_info<factory> r1, r2, r3;
	auto p = parse(r1.mk_parser(), parser::make_source("+3+"), r1);
	auto p2 = parse(r1.mk_parser(), parser::make_source("+"), r2);
	auto p3 = parse(r1.mk_parser(), parser::make_source(" "), r3);
	return
	(p==3) + 2*(r1.shift==3) + 4*r1.trim +
	8*(p2==1) + 16*(r2.shift==0) + 32*(r2.trim) +
	64*(p3==0) + 128*(r3.shift==0) + 256*(!r3.trim)
	;
}() == 511);
static_assert( []{
	jinja_details::comment_operator<factory> r1, r2;
	auto p1 = parse(r1.mk_parser(), parser::make_source("<# <% <= foo +#>"), r1);
	auto p2 = parse(r1.mk_parser(), parser::make_source("<#+ <% <= foo #>"), r2);
	return
	  (p1==16) + 2*(r1.value == " <% <= foo ") + 4*r1.end.trim +
	8*(p2==16) + 16*r2.begin.trim
	;
}() == 31 );

int main(int,char**) {
	return 0;
}