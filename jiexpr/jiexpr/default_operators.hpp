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

struct expr_operators_simple { };

template<typename data_type>
constexpr auto to_bool(const expr_operators_simple& op, auto&& val) {
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
constexpr auto negate(const expr_operators_simple& op, auto&& val) {
	return data_type{ !to_bool<data_type>( op, std::forward<decltype(val)>(val) ) };
}
template<typename data_type>
constexpr auto do_and(const expr_operators_simple& op, auto&& left, auto&& right) {
	return data_type{ to_bool<data_type>(op, left) && to_bool<data_type>(op, right) };
}
template<typename data_type>
constexpr auto do_or(const expr_operators_simple& op, auto&& left, auto&& right) {
	return data_type{ to_bool<data_type>(op, left) || to_bool<data_type>(op, right) };
}
template<typename data_type>
constexpr auto do_ceq(const expr_operators_simple& op, data_type&& left, data_type&& right) {
	if(left.is_none()||right.is_none()) return data_type{left.is_none() && right.is_none()};
	return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l==r;}{return l==r;});
}
template<typename data_type>
constexpr auto do_neq(const expr_operators_simple& op, data_type&& left, data_type&& right) {
	if(left.is_none()||right.is_none()) return data_type{!(left.is_none() && right.is_none())};
	return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l!=r;}{return l!=r;});
}

template<typename data_type>
constexpr auto do_lt(const expr_operators_simple& op, data_type&& left, data_type&& right) {
	return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l<r;}{return l<r;});
}
template<typename data_type>
constexpr auto do_gt(const expr_operators_simple& op, data_type&& left, data_type&& right) {
	return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l>r;}{return l>r;});
}
template<typename data_type>
constexpr auto do_let(const expr_operators_simple& op, data_type&& left, data_type&& right) {
	return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l<=r;}{return l<=r;});
}
template<typename data_type>
constexpr auto do_get(const expr_operators_simple& op, data_type&& left, data_type&& right) {
	return exec_operation(left, right, [](const auto& l, const auto& r)requires requires{l>=r;}{return l>=r;});
}

template<typename data_type>
constexpr auto do_in(const expr_operators_simple& op, data_type&& left, data_type&& right) {
	return right.contains(std::forward<decltype(left)>(left));
}

template<typename to_type>
constexpr auto div(const expr_operators_simple&, const auto& l, const auto& r) {
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
constexpr auto math_op(const expr_operators_simple&, auto&& l, auto&& r, auto&& op) {
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

template<typename data_type>
constexpr auto int_div(const expr_operators_simple& op, auto&& left, auto&& right) {
	return div<typename data_type::integer_t>( op, left, right );
}

template<typename data_type>
constexpr auto fp_div(const expr_operators_simple& op, auto&& left, auto&& right) {
	return div<typename data_type::float_point_t>( op, left, right );
}

template<typename data_type>
constexpr auto sub(const expr_operators_simple& op, auto&& l, auto&& r) {
	return math_op<data_type>(op, l,r, [](const auto& l, const auto& r){ return l - r; });
}

template<typename data_type>
constexpr auto pow(const expr_operators_simple&, auto&& l, auto&& r) {
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
template<typename data_type>
constexpr auto multiply(const expr_operators_simple& op, auto&& l, auto&& r) {
	return math_op<data_type>(op, l,r, [](const auto& l, const auto& r){ return l * r; });
}

template<typename data_type>
constexpr auto substruct(const expr_operators_simple& op, auto&& l, auto&& r) {
	return math_op<data_type>(op, l,r, [](const auto& l, const auto& r){ return l - r; });
}

template<typename data_type>
constexpr auto add(const expr_operators_simple& op, auto&& l, auto&& r) {
	return math_op<data_type>(op, l,r, [](const auto& l, const auto& r){ return l + r; });
}

constexpr auto do_concat(const expr_operators_simple&, auto&& left, auto&& right) {
	return std::forward<decltype(left)>(left) + std::forward<decltype(right)>(right);
}

} // namespace jiexpr_details
