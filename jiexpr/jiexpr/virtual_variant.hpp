#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

namespace jiexpr_details {

template<typename _factory, typename _eval_info>
struct expression_base {
	using factory = _factory;
	using data_type = factory::data_type;
	using eval_info = _eval_info;

	virtual ~expression_base() noexcept =default ;
	constexpr virtual data_type eval(const eval_info&) const =0 ;
	constexpr virtual bool is_eq_operator() const {return false;}
};

template<typename factory, typename eval_info, typename item_type>
struct expression_item_wrapper : expression_base<factory, eval_info> {
	using base_type = expression_base<factory, eval_info>;
	using data_type = base_type::data_type;
	factory f;
	item_type item{};
	constexpr expression_item_wrapper() =default ;
	constexpr explicit expression_item_wrapper(factory f) : f(std::move(f)) {}
	constexpr data_type eval(const eval_info&) const override {
		return data_type{f, item};
	}
};

template<typename base_type>
struct expression_variant : base_type {
	using factory = base_type::factory;

	constexpr expression_variant() =default ;
	constexpr explicit expression_variant(factory f) : base_type(std::move(f)) {}

	using data_type = base_type::data_type;
	using eval_info = base_type::eval_info;
	constexpr data_type eval(const eval_info& i) const override { return this->pointer->eval(i); }
	constexpr bool is_eq_operator() const override { return this->pointer->is_eq_operator(); }
};

} // namespace jiexpr_details
