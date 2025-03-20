/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "factory.hpp"

static_assert( [] {
  jinja_details::file<factory> r1;
  auto src1 = "<% import 1 as 2 %><%import 2 as 3%><% template t%>cnt<%endtemplate%><%template t2%><% endtemplate %>"sv;
  const auto p1 = parse(r1.mk_parser(factory{}), +parser::space, parser::make_source(src1), r1);
  return (p1==src1.size()) + 2*(r1.imports.size()==2) + 4*(r1.templates.size()==2);
}() == 7 );
static_assert( [] {
  jinja_details::file<factory> r1;
  auto src1 = "<% import 1 as 2 %><%import 2 as 3%><%template%><% block name() %>cnt<% endblock %><%endtemplate%>"sv;
  const auto p1 = parse(r1.mk_parser(factory{}), +parser::space, parser::make_source(src1), r1);
  return (p1==src1.size()) + 2*(r1.imports.size()==2) + 4*(r1.templates.size()==1);
}() == 7 );

int main(int,char**){}