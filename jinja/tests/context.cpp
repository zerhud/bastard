/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "factory.hpp"

#include "absd/iostream_op.hpp"
#include <iostream>

using namespace std::literals;

static_assert( absd::details::as_object<jinja_ctx::out_info, data> );
static_assert( absd::details::as_array<jinja_ctx::output_frame, data> );

static_assert( [] {
	jinja_ctx c{ factory {} };
	c(data{"content"});
	auto out = c.extract_output();
	auto obj = data::mk(out);
	return (obj.size()==1) + 2*obj.is_array() + 4*obj[0].is_object();
}() == 7 );
static_assert( [] {
	jinja_ctx c{ factory {} };
	c(data{"content"});
	c(jinja_details::trim_info<factory>{true}, data{"trim"}, jinja_details::trim_info<factory>{false});
	c(jinja_details::shift_info{3, true});
	auto out = c.extract_output();
	auto obj = data::mk(out);
	return (obj.size()==3) + 2*(obj[0][data{"value"}]=="content") + 4*(obj[0][data{"trim_before"}]==false) +
		8*(obj[1][data{"value"}]=="trim") + 16*(obj[1][data{"trim_before"}]==true) +
		32*obj[2][data{"value"}].is_none() + 64*(obj[2][data{"shift"}]==3)
	;
}() == 127 );

int main(int,char**) {
	return 0;
}
