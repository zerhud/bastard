#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

namespace jinja_details {

struct shift_info {
	short shift{0};
	bool absolute{false};

	template<typename p, template<auto>class th=p::template tmpl>
	constexpr static auto mk_parser() {
		constexpr auto abs_parser = -fnum<0>(as<-1>(th<'-'>::char_)|as<1>(th<'+'>::char_));
		return add_to_ctx<shift_info>(0,
			   abs_parser
			>> use_seq_result(p::nop([](auto& r){r.absolute=(r.shift==0); r.shift+=(r.shift==0); return &r;}))
			>> result_from_ctx<shift_info>([](auto& r, auto& tmp){r *= tmp;}, th<10>::uint_)
		);
	}
};

template<typename factory>
struct trim_info {
	using p = factory::parser;
	bool trim{false};
	constexpr static auto mk_parser() {
		return p::nop++ >> ---as<true>(p::template char_<'+'>);
	}
};

} // namespace jinja_details
