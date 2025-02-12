/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "factory.hpp"

using jinja_content = jinja_details::content<factory>;
using jinja_block = jinja_details::named_block<factory>;
using jinja_macro = jinja_details::macro_block<factory>;
using jinja_set_b = jinja_details::set_block<factory>;

static_assert( parse(jinja_details::content<factory>::mk_parser(), parser::make_source("ab%! ^(\\<<<%")) == 10 );
static_assert( []{
	jinja_details::content<factory> r{factory{}};
	const auto p = parse(r.mk_parser(), parser::make_source("\\%<<#"), r);
	return (p == 3) + 2*(r.value == "\\%<");
}() == 3, "content parser test" );
static_assert( []{
	jinja_details::trim_info<factory> r2, r3;
	const auto p2 = parse(r2.mk_parser(), parser::make_source("+"), r2);
	const auto p3 = parse(r3.mk_parser(), parser::make_source(" "), r3);
	return (p2==1) + 2*(r2.trim) + 4*(p3==0) + 8*(!r3.trim) ;
}() == 15, "trim info parser test");
static_assert( [] {
	jinja_details::shift_info r1, r2, r3;
	const auto p1 = parse(r1.mk_parser<parser>(), +parser::space, parser::make_source("+3"), r1);
	const auto p2 = parse(r1.mk_parser<parser>(), +parser::space, parser::make_source("3"), r2);
	const auto p3 = parse(r1.mk_parser<parser>(), +parser::space, parser::make_source("-3"), r3);
	return (p1==2) + 2*(p2==1) + 4*(p3==2)
	+ 8*(!r1.absolute) + 16*r2.absolute + 32*(!r3.absolute)
	+ 64*(r1.shift==3) + 128*(r2.shift==3) + 256*(r3.shift==-3)
	;
}() == 511, "shift_info parser test" );
static_assert( []{
	jinja_details::comment_operator<factory> r1{factory{}}, r2{factory{}};
	const auto p1 = parse(r1.mk_parser(), parser::make_source("<# <% <= foo +#>"), r1);
	const auto p2 = parse(r1.mk_parser(), parser::make_source("<#+ <% <= foo #>"), r2);
	return
	  (p1==16) + 2*(r1.value == " <% <= foo ") + 4*r1.end.trim +
	8*(p2==16) + 16*r2.begin.trim
	;
}() == 31, "comment operator parser teset" );
static_assert( [] {
	test_expr r1;
	const auto p1 = parse(r1.mk_parser(1,1,1), +parser::space, parser::make_source("3"), r1);
	return (p1==1) + 2*(r1.index()==1);
}() == 3);
static_assert( []{
	jinja_details::expression_operator<factory> r1{ factory{} };
	const auto p1 = parse(r1.mk_parser(factory{}), +parser::space, parser::make_source("<= 3 =>"), r1);
	return (p1 == 7) + 2*(get<1>(r1.expr)==3);
}() == 3, "expression operator parser test" );


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
static_assert( [] {
	jinja_details::template_block<factory> r1, r2;
	const auto p1 = parse( r1.mk_parser(factory{}), +parser::space,
	parser::make_source("<% template tmpl %> <% block name(p=1,r=2) %> <% endblock %> <%endtemplate%>"), r1);
	const auto p2 = parse( r1.mk_parser(factory{}), +parser::space,
	parser::make_source("<% template tmpl %> <%+ +3 block name() %> <% endblock %> <%endtemplate%>"), r2);
	return (p1==76)
	+ 2*(r1.holder.size() == 3)
	+ 4*(static_cast<const jinja_block*>(r1.holder[1].get())->parameters.size() == 2)
	+ 8*(static_cast<const jinja_block*>(r1.holder[1].get())->parameters[0].name == "p")
	+ 16*(p2==73)
	+ 32*(static_cast<const jinja_block*>(r2.holder[1].get())->parameters.size() == 0)
	+ 64*(static_cast<const jinja_block*>(r2.holder[1].get())->shift_inside.shift == 3)
	+ 128*static_cast<const jinja_block*>(r2.holder[1].get())->begin_left.trim
	;
}() == 255, "can parse blocks with parameters" );
static_assert( []{
	jinja_details::template_block<factory> r1;
	const auto p1 = parse( r1.mk_parser(factory{}), +parser::space,
	parser::make_source("<% template tmpl %><% macro name %> <% endmacro %><%endtemplate%>"), r1);
	return (p1==65)
	+ 2*(r1.holder.size() == 1)
	+ 4*(static_cast<const jinja_macro*>(r1.holder[0].get())->name() == "name")
	+ 8*(static_cast<const jinja_macro*>(r1.holder[0].get())->size() == 1)
	;
}() == 15, "can parser template with macros" );

static_assert( [] {
	jinja_details::template_block<factory> r1, r2;
	auto src = "<%template t%><%set name%>cnt<%endset%><%endtemplate%>"sv;
	auto src2 = "<%template t%><%set(name)expression%>cnt<%endset%><%endtemplate%>"sv;
	const auto p1 = parse(r1.mk_parser(factory{}), +parser::space, parser::make_source(src), r1);
	const auto p2 = parse(r2.mk_parser(factory{}), +parser::space, parser::make_source(src2), r2);
	return (p1==src.size())
	+ 2*(static_cast<const jinja_set_b&>(*r1[0]).name() == "name")
	+ 4*(get<0>(static_cast<const jinja_set_b&>(*r1[0]).handler) == "name")
	+ 8*(p2==src2.size())
	+ 16*(static_cast<const jinja_set_b&>(*r2[0]).name() == "name")
	+ 32*(get<0>(static_cast<const jinja_set_b&>(*r2[0]).handler) == "expression")
	;
}() == 63, "template with set block" );

/*
static_assert( [] {
	jinja_details::template_block<factory> r1, r2;
	auto src1 = "<%template t%><%if 1%>cnt<%else%>e<%endif%><%endtemplate%>"sv;
	const auto p1 = parse(r1.mk_parser(factory{}), +parser::space, parser::make_source(src1), r1);
	return (p1==src1.size()) + 2*(r1.size()==1);
}() == 3, "other blocks" );
*/

int main(int,char**) { }
