#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "common.hpp"
#include "content.hpp"

namespace jinja_details {

template<typename factory>
struct block_content : base_jinja_element<factory> {
	using base = base_jinja_element<factory>;
	using context_type = base_jinja_element<factory>::context_type;

	constexpr static auto mk_content_holder(const factory& f) {
		using ptr_t = decltype(mk_empty_ptr<const base>(f));
		return mk_vec<ptr_t>(f);
	}

	using holder_type = decltype(mk_content_holder(std::declval<factory>()));

	constexpr void execute(context_type& ctx) const override { }

	constexpr static auto struct_fields_count() { return 4; }

	trim_info<factory> left;
	holder_type holder;
	trim_info<factory> right;
	factory f;

	constexpr auto size() const { return holder.size(); }
	constexpr auto& operator[](auto ind) const { return holder[ind]; }

	constexpr explicit block_content(factory f)
		: holder(mk_content_holder(f))
		, f(std::move(f))
	{}

};

} // namespace jinja_details
