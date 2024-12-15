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

struct test_context ;

struct factory : tests::factory {
	using parser = ascip<std::tuple>;
	using jinja_expression = test_expr;
	using jinja_context = test_context;
};

using jinja_content = jinja_details::content<factory>;
using jinja_block = jinja_details::named_block<factory>;

using parser = factory::parser;
struct test_context {
	factory f;
	constexpr void append_content(const auto& ){}
	constexpr void append_output(const auto&, auto&&, const auto&){}
};

constexpr auto jinja_to_string(const factory&, const test_expr&) { return std::string{}; }

static_assert( parse(jinja_details::content<factory>::mk_parser(), parser::make_source("ab%! ^(\\<<<%")) == 10 );
static_assert( []{
	jinja_details::content<factory> r;
	auto p = parse(r.mk_parser(), parser::make_source("\\%<<#"), r);
	return (p == 3) + 2*(r.value == "\\%<");
}() == 3 );
static_assert( []{
	jinja_details::trim_info<factory> r1, r2, r3;
	const auto p = parse(r1.mk_parser(), parser::make_source("+3+"), r1);
	const auto p2 = parse(r1.mk_parser(), parser::make_source("+"), r2);
	const auto p3 = parse(r1.mk_parser(), parser::make_source(" "), r3);
	return
	(p==3) + 2*(r1.shift==3) + 4*r1.trim +
	8*(p2==1) + 16*(r2.shift==0) + 32*(r2.trim) +
	64*(p3==0) + 128*(r3.shift==0) + 256*(!r3.trim)
	;
}() == 511);
static_assert( []{
	jinja_details::comment_operator<factory> r1, r2;
	const auto p1 = parse(r1.mk_parser(), parser::make_source("<# <% <= foo +#>"), r1);
	const auto p2 = parse(r1.mk_parser(), parser::make_source("<#+ <% <= foo #>"), r2);
	return
	  (p1==16) + 2*(r1.value == " <% <= foo ") + 4*r1.end.trim +
	8*(p2==16) + 16*r2.begin.trim
	;
}() == 31 );
static_assert( []{
	jinja_details::expression_operator<factory> r1;
	const auto p1 = parse(r1.mk_parser(), +parser::space, parser::make_source("<= 3 =>"), r1);
	return (p1 == 7) + 2*(get<0>(r1.expr)==3);
}() == 3 );


static_assert( []{
	jinja_details::template_block<factory> r1;
	auto p1 = parse(r1.mk_parser(factory{}), +parser::space, parser::make_source("<% template foo %><% endtemplate %>"), r1);
	return p1;
}() == 35, "can parse empty template");
static_assert( []{
	jinja_details::template_block<factory> r1;
	const auto p1 = parse( r1.mk_parser(factory{}), +parser::space,
	parser::make_source("<% template tmpl %> <% block name %> <% endblock %> <%endtemplate%>"), r1);
	return (p1==67)
	+ 2*(r1.holder.size() == 3)
	+ 4*(static_cast<const jinja_content*>(r1.holder[0].get())->value == " ")
	+ 8*(static_cast<const jinja_content*>(r1.holder[2].get())->value == " ")
	+ 16*(static_cast<const jinja_block*>(r1.holder[1].get())->name() == "name")
	+ 32*(static_cast<const jinja_block*>(r1.holder[1].get())->size() == 1)
	+ 64*(static_cast<const jinja_content&>(*static_cast<const jinja_block&>(*r1.holder[1])[0]).value == " ")
	;
}() == 127, "can parser template with blocks" );

int main(int,char**) {
	return 0;
}
