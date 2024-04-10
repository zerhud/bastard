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

struct expr_operators_simple {
	template<typename data_type>
	constexpr static auto math_op(auto&& l, auto&& r, auto&& op) {
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		if(l.is_int()) {
			if(r.is_float_point()) return data_type{ float_point_t( op((integer_t)l,(float_point_t)r) ) };
			return data_type{ integer_t( op((integer_t)l,(integer_t)r) ) };
		}
		else {
			if(r.is_float_point()) return data_type{ float_point_t( op((float_point_t)l,(float_point_t)r) ) };
			return data_type{ float_point_t( op((float_point_t)l,(integer_t)r) ) };
		}
	}

	template<typename to_type>
	constexpr static auto div(const auto& l, const auto& r) {
		using data_type = std::decay_t<decltype(l)>;
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		if(l.is_int()) {
			if(r.is_float_point()) return data_type{ to_type((integer_t)l / (float_point_t)r) };
			return data_type{ to_type((integer_t)l / (integer_t)r) };
		}
		else {
			if(r.is_float_point()) return data_type{ to_type((float_point_t)l / (float_point_t)r) };
			return data_type{ to_type((float_point_t)l / (integer_t)r) };
		}
	}

	template<typename data_type>
	constexpr static auto to_bool(auto&& val) {
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		using string_t = typename data_type::string_t;
		if( val.is_bool() ) return val;
		else if( val.is_int() ) return data_type{ !!((integer_t)val) };
		else if( val.is_float_point() ) return data_type{ !!((float_point_t)val) };
		else if( val.is_string() ) return data_type{ !((string_t)val).empty() };
		else return data_type{ false };
	}

	template<typename data_type>
	constexpr static auto int_div(auto&& left, auto&& right) {
		return div<typename data_type::integer_t>( left, right );
	}

	template<typename data_type>
	constexpr static auto fp_div(auto&& left, auto&& right) {
		return div<typename data_type::float_point_t>( left, right );
	}

	template<typename data_type>
	constexpr static auto mul(auto&& l, auto&& r) {
		return math_op<data_type>(l,r, [](const auto& l, const auto& r){ return l * r; });
	}

	template<typename data_type>
	constexpr static auto sub(auto&& l, auto&& r) {
		return math_op<data_type>(l,r, [](const auto& l, const auto& r){ return l - r; });
	}

	template<typename data_type>
	constexpr static auto add(auto&& l, auto&& r) {
		return math_op<data_type>(l,r, [](const auto& l, const auto& r){ return l + r; });
	}

	constexpr static auto do_concat(auto&& left, auto&& right) {
		return std::forward<decltype(left)>(left) + std::forward<decltype(right)>(right);
	}

	template<typename data_type>
	constexpr static auto negate(auto&& val) {
		return data_type{ !to_bool<data_type>( std::forward<decltype(val)>(val) ) };
	}
	template<typename data_type>
	constexpr static auto do_and(auto&& left, auto&& right) {
		return data_type{ to_bool<data_type>(left) && to_bool<data_type>(right) };
	}
	template<typename data_type>
	constexpr static auto do_or(auto&& left, auto&& right) {
		return data_type{ to_bool<data_type>(left) || to_bool<data_type>(right) };
	}
	template<typename data_type>
	constexpr static auto do_ceq(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l==r;}{return l==r;});
	}
	template<typename data_type>
	constexpr static auto do_neq(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l!=r;}{return l!=r;});
	}
	template<typename data_type>
	constexpr static auto do_lt(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l<r;}{return l<r;});
	}
	template<typename data_type>
	constexpr static auto do_gt(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l>r;}{return l>r;});
	}
	template<typename data_type>
	constexpr static auto do_let(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l<=r;}{return l<=r;});
	}
	template<typename data_type>
	constexpr static auto do_get(data_type&& left, data_type&& right) {
		return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l>=r;}{return l>=r;});
	}
	template<typename data_type>
	constexpr static auto do_in(data_type&& left, data_type&& right) {
		return right.contains(std::forward<decltype(left)>(left));
	}

	template<typename data_type>
	constexpr static auto pow(auto&& l, auto&& r) {
		using integer_t = typename data_type::integer_t;
		using float_point_t = typename data_type::float_point_t;
		auto right = (integer_t)r;
		if(l.is_int()) {
			auto left = (integer_t)l;
			while(--right > 0) left *= left;
			return data_type{ left };
		}
		else {
			auto left = (float_point_t)l;
			while(--right > 0) left *= left;
			return data_type{ left };
		}
	}
};

} // namespace jiexpr_details
