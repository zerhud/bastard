#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

namespace absd::details {

template<typename data_type, typename functor>
struct callable2 {
	using factory_t = data_type::factory_t;

	functor fnc;
	data_type params_info;

	constexpr explicit callable2(factory_t f, functor fnc, auto&&... params)
	: fnc(std::move(fnc))
	, params_info(std::move(f))
	{
		params_info.mk_empty_array();
		(void)(create_param(std::forward<decltype(params)>(params)),..., 1);
	}

	constexpr explicit callable2(functor f, auto&&... params)
	: callable2(factory_t{}, std::move(f), std::forward<decltype(params)>(params)...) { }

	constexpr auto operator()(auto&&... args) const {
		return call_with_params<sizeof...(args)>(std::forward<decltype(args)>(args)...);
	}

	constexpr auto call(data_type params) const {
		using ret_type = decltype(call_with_combined_params<0>(params));
		if constexpr(!std::is_same_v<ret_type, void>) return call_with_combined_params<0>(params);
		else {
			call_with_combined_params<0>(params);
			return data_type{params_info.factory};
		}
	}
private:
	using integer_t = typename data_type::integer_t;
	constexpr static integer_t param_sign_name = 1;
	constexpr static integer_t param_sign_value = 2;
	constexpr void create_param(auto&& param) ;
	template<auto initial_count> constexpr auto call_with_params(auto&&... params) const ;
	template<auto ind> constexpr auto call_with_combined_params(const auto& user_params, auto&&... params) const ;
	template<auto ind> constexpr auto find_next_param(const auto& user_params) const ;
};

template<typename data_type, typename functor> template<auto ind>
constexpr auto callable2<data_type, functor>::call_with_combined_params(const auto& user_params, auto&&... params) const {
	static_assert( sizeof...(params) < 501, "too many parameters :)" );
	if constexpr (requires{fnc(std::forward<decltype(params)>(params)...);}) return fnc(std::forward<decltype(params)>(params)...);
	else {
		data_type next_param = find_next_param<ind>(user_params);
		if(next_param.is_none()) factory_t::template throw_wrong_parameters_count<ind>();
		return call_with_combined_params<ind+1>(
				user_params,
				std::forward<decltype(params)>(params)...,
				std::move(next_param));
	}
}

template<typename data_type, typename functor> template<auto ind>
constexpr auto callable2<data_type, functor>::find_next_param(const auto& user_params) const {
	if(user_params.contains(data_type{ind}))
		return user_params[data_type{ind}];
	if(user_params.contains(params_info[ind][data_type{param_sign_name}]))
		return user_params[params_info[ind][data_type{param_sign_name}]];
	return params_info[ind][data_type{param_sign_value}];
}

template<typename data_type, typename functor>
constexpr void callable2<data_type, functor>::create_param(auto&& param) {
	data_type desk;
	constexpr bool  parameter_is_only_name = requires{data_type{param};};
	if constexpr (parameter_is_only_name)
		desk.put(data_type{param_sign_name}, data_type{params_info.factory, std::forward<decltype(param)>(param)});
	else {
		//TODO: use std::forward_like<decltype(param)>(name) and same for def_val since clang can support
		auto&& [name, def_val] = param;
		desk.put(data_type{param_sign_name}, data_type{params_info.factory, name});
		desk.put(data_type{param_sign_value}, data_type{params_info.factory, def_val});
	}
	params_info.push_back(std::move(desk));
}

template<typename data_type, typename functor> template<auto initial_count>
constexpr auto callable2<data_type, functor>::call_with_params(auto&&... params) const {
	if constexpr (requires{fnc(std::forward<decltype(params)>(params)...);})
		return fnc(std::forward<decltype(params)>(params)...);
	else {
		auto& desk = params_info[(integer_t) sizeof...(params)];
		const bool is_name_and_value_param = desk.size() == 2;
		if(is_name_and_value_param)
			return call_with_params<initial_count>(std::forward<decltype(params)>(params)..., desk[data_type{param_sign_value}]);
		else {
			factory_t::template throw_wrong_parameters_count<initial_count>();
			return call_with_params<initial_count>(std::forward<decltype(params)>(params)..., data_type{params_info.factory});
		}
	}
}

} // namespace absd::details
