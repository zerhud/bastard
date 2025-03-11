/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "factory.hpp"

static_assert( [] {
  jinja_details::template_block<factory> r1, r2;
  auto src1 = "<%template t%><%if 1%>cnt<%elif 1 %>a<%else%>e<%endif%><%endtemplate%>"sv;
  auto src2 = "<%template t%><%if 1%>cnt<%endif%><%endtemplate%>"sv;
  const auto p1 = parse(r1.mk_parser(factory{}), +parser::space, parser::make_source(src1), r1);
  const auto p2 = parse(r2.mk_parser(factory{}), +parser::space, parser::make_source(src2), r2);
  auto& ir1 = static_cast<const jinja_details::if_block<factory>&>(*r1[0]);
  auto& ir2 = static_cast<const jinja_details::if_block<factory>&>(*r2[0]);
  return (p1==src1.size() && p2==src2.size())
  + 2*(ir1.holder.size()==1)
  + 4*(ir1.else_blocks.size()==2)
  + 8*(ir1.else_blocks[0].holder.size()==1)
  + 16*(ir1.else_blocks[1].holder.size()==1)
  + 32*(ir2.holder.size()==1)
  + 64*ir2.else_blocks.empty()
  ;
}() == 127, "if and else blocks" );

static_assert( [] {
  jinja_details::template_block<factory> r2, r3;
  auto src2 = "<%template t%><%for a,b in arr; c,d,e in bar %>cnt<%endfor%><%endtemplate%>"sv;
  auto src3 = "<%template t%><%for a in bar %>cnt<%else%>else<%endfor%><%endtemplate%>"sv;
  const auto p2 = parse(r2.mk_parser(factory{}), +parser::space, parser::make_source(src2), r2);
  const auto p3 = parse(r3.mk_parser(factory{}), +parser::space, parser::make_source(src3), r3);
  auto& ir2 = static_cast<const jinja_details::for_block<factory>&>(*r2[0]);
  auto& ir3 = static_cast<const jinja_details::for_block<factory>&>(*r3[0]);
  return (p2==src2.size())
  + 2*(ir2.holder.size()==1) + 4*(ir2.for_exprs.size()==2)
  + 8*(ir2.for_exprs[0].variables.size()==2) + 16*(ir2.for_exprs[1].variables.size()==3)
  + 32*(p3==src3.size()) + 64*(ir3.holder.size()==1) + 128*(ir3.else_holder.size()==1)
  ;
}() == 255 );

static_assert( [] {
  jinja_details::template_block<factory> r1, r2, r3;
  auto src1 = "<%template t%><%call a %>cnt<%endcall%><%endtemplate%>"sv;
  auto src2 = "<%template t%><%call(a=1,b=2) a %>cnt<%endcall%><%endtemplate%>"sv;
  const auto p1 = parse(r1.mk_parser(factory{}), +parser::space, parser::make_source(src1), r1);
  const auto p2 = parse(r2.mk_parser(factory{}), +parser::space, parser::make_source(src2), r2);
  return (p1==src1.size()) + 2*(p2==src2.size());
}() == 3 );

int main(int,char**) { }
