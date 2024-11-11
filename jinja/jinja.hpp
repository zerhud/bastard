#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

#include "jinja/common.hpp"
#include "jinja/operators.hpp"

namespace jinja_details {

template<typename factory>
struct content : base_jinja_element<factory> {
	using string_type = factory::string_t;
	using parser = factory::parser;
	using context_type = base_jinja_element<factory>::context_type;

	string_type value;

	constexpr void execute(context_type& ctx) const override {
		ctx.append_content(value);
	}

	constexpr static auto mk_parser() {
		using bp = base_parser<factory>;
		return lexeme( fnum<0>(parser::nop) >> +(parser::any - bp::mk_check_parser()) );
	}
};

template<typename factory>
struct named_block : base_jinja_element<factory> {
	using base = base_jinja_element<factory>;
	using context_type = base_jinja_element<factory>::context_type;
	using p = factory::parser;

	constexpr static auto mk_content_holder(const factory& f) {
		return mk_vec<const base*>(f);
	}

	constexpr void execute(context_type& ctx) const override { }

	constexpr static auto struct_fields_count() { return 4; }
	trim_info<factory> begin_left, begin_right;
	trim_info<factory> end_left, end_right;
};

} // namespace jinja_details

template<typename factory>
struct jinja {
	factory f;

	constexpr jinja() : jinja(factory{}) {}
	constexpr explicit jinja(factory f) : f(std::move(f)) {}
};
