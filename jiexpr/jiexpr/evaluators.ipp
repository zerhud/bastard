#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

template< typename data_type, typename operators_factory, typename data_factory >
constexpr void bastard<data_type,operators_factory,data_factory>::mk_params(auto& params, const auto& op) const {
	typename data_type::integer_t ind=params.size();
	for(auto& param:op.params) {
		if(!jiexpr_details::variant_holds<op_eq_tag>(*param)) params.put(data_type{ind++}, visit(*this, *param));
		else {
			auto& [name,val] = jiexpr_details::get_by_tag<op_eq_tag>(*param);
			params.put(data_type{get<string_t>(*name.path.at(0))}, visit(*this, *val));
		}
	}
}

template< typename data_type, typename operators_factory, typename data_factory > template<typename... types>
constexpr data_type bastard<data_type,operators_factory,data_factory>::operator()(const op_concat<types...>& op) const {
	auto left_str = df.mk_str();
	auto right_str = df.mk_str();
	back_insert_format(df.back_inserter(left_str), visit(*this, *op.left));
	back_insert_format(df.back_inserter(right_str), visit(*this, *op.right));
	return data_type{ ops.template do_concat( std::move(left_str), std::move(right_str) ) };
}
template< typename data_type, typename operators_factory, typename data_factory > template<typename... types>
constexpr data_type bastard<data_type,operators_factory,data_factory>::operator()(const op_eq<types...>& op) const {
	auto cur = *env;
	auto& left = op.name.path;
	for(auto i=0;i<left.size()-1;++i) cur = cur[data_type{get<string_t>(*left[i])}];
	data_type key{get<string_t>(*left[left.size()-1])};
	cur.put(key, visit(*this, *op.value));
	return cur[key];
}
template< typename data_type, typename operators_factory, typename data_factory > template<typename... types>
constexpr data_type bastard<data_type,operators_factory,data_factory>::operator()(const list_expr<types...>& op) const {
	data_type ret;
	ret.mk_empty_array();
	for(auto&& item:op.list) ret.push_back(visit(*this, *item));
	return ret;
}
template< typename data_type, typename operators_factory, typename data_factory > template<typename... types>
constexpr data_type bastard<data_type,operators_factory,data_factory>::operator()(const dict_expr<types...>& op) const {
	data_type ret;
	ret.mk_empty_object();
	for(auto i=0;i<op.names.size();++i)
		ret.put(visit(*this, *op.names[i]), visit(*this, *op.values.at(i)));
	return ret;
}
template< typename data_type, typename operators_factory, typename data_factory > template<typename... types>
constexpr data_type bastard<data_type,operators_factory,data_factory>::operator()(const var_expr<types...>& op) const {
	auto cur = (*env)[data_type{get<string_t>(*op.path.at(0))}];
	for(auto pos = ++op.path.begin();pos!=op.path.end();++pos) {
		auto& item = **pos;
		if(holds_alternative<string_t>(item)) cur = cur[data_type{get<string_t>(item)}];
		else {
			auto key = visit(*this, item);
			if(key.is_int()) cur = cur[(integer_t)key];
			else cur = cur[key];
		}
	}
	return cur;
}
template< typename data_type, typename operators_factory, typename data_factory > template<typename... types>
constexpr data_type bastard<data_type,operators_factory,data_factory>::operator()(const fnc_call_expr<types...>& op) const {
	auto fnc =(*this)(op.name);
	data_type params;
	params.mk_empty_object();
	mk_params(params, op);
	return fnc.call(std::move(params));
}
template< typename data_type, typename operators_factory, typename data_factory > template<typename... types>
constexpr data_type bastard<data_type,operators_factory,data_factory>::operator()(const ternary_op<types...>& op) const {
	if(ops.template to_bool<data_type>(visit(*this,*op.cond))) return visit(*this,*op.left);
	if(op.right) return visit(*this, *op.right);
	return data_type{};
}
template< typename data_type, typename operators_factory, typename data_factory >
constexpr data_type bastard<data_type,operators_factory,data_factory>::operator()(const auto& op) const {
	if constexpr (requires{!op.left;}) if(!op.left) std::unreachable();
	if constexpr (requires{!op.right;}) if(!op.right) std::unreachable();
	if constexpr (requires{visit([](const auto&){}, op);}) {
		return visit(*this, op);
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_division> ) {
		return ops.template int_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_fp_div> ) {
		return ops.template fp_div<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_multiply> ) {
		return ops.template mul<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_subtract > ) {
		return ops.template sub<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_addition> ) {
		return ops.template add<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_power> ) {
		return ops.template pow<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_not> ) {
		return ops.template negate<data_type>( visit(*this,*op.expr) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_and> ) {
		return ops.template do_and<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_or> ) {
		return ops.template do_or<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_ceq> ) {
		return ops.template do_ceq<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_neq> ) {
		return ops.template do_neq<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_lt> ) {
		return ops.template do_lt<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_gt> ) {
		return ops.template do_gt<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_let> ) {
		return ops.template do_let<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_get> ) {
		return ops.template do_get<data_type>( visit(*this,*op.left), visit(*this,*op.right) );
	}
	else if constexpr ( jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, op_in> ) {
		return data_type{ ops.template do_in<data_type>( visit(*this,*op.left), visit(*this,*op.right) ) };
	}
	else if constexpr (
			jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, apply_filter_expr>
			|| 	jiexpr_details::is_specialization_of<std::decay_t<decltype(op)>, is_test_expr>
			) {
		data_type params;
		params.put(data_type{0}, visit(*this, *op.object));
		auto ret = visit([this,&params](const auto& op){
			if constexpr (requires{op.path;}) {
				return (*this)(op).call(params);
			} else {
				auto fnc = (*this)(op.name);
				mk_params(params, op);
				return fnc.call(std::move(params));
			}
		}, [&op]->auto& {
			if constexpr(requires{op.filter;}) return op.filter;
			else return op.test;
		}());
		if constexpr (requires{op.test;}) return ops.template to_bool<data_type>(ret);
		else return ret;
	}
	else {
		op.wasnt_specialized();
		//std::unreachable(); // your specialization doesn't work :(
		return data_type{(integer_t) __LINE__};
	}
}
