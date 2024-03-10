//
// Created by zerhud on 08/03/24.
//

#pragma once

namespace absd {

template<typename factory>
template<typename interface, typename ret_val_t>
constexpr ret_val_t data<factory>::throw_wrong_interface_error(ret_val_t ret_val) {
	factory::template throw_wrong_interface_error2<interface>();
	std::unreachable();
	return ret_val;
}

template<typename factory>
constexpr auto data<factory>::inner_mk(const factory& f, auto&& _v, auto&& ... args) {
	auto v = mk_ca_val(std::forward<decltype(_v)>(_v), std::forward<decltype(args)>(args)...);
	using val_type = std::decay_t<decltype(v)>;

	constexpr bool is_callable = details::is_specialization_of<val_type, details::callable2>;

	auto arr = details::mk_te_array<self_type>(f, counter_maker < details::origin<val_type> > {std::move(v)});
	auto arr_obj = details::mk_te_object<self_type>(f, std::move(arr));
	return details::mk_te_callable<is_callable, self_type>(f, std::move(arr_obj));
}

template<typename factory>
constexpr void data<factory>::mk_ptr_and_assign(data::self_type& ret, const auto& f, auto&& v) {
	auto tmp = f.mk_ptr(std::forward<decltype(v)>(v));
	auto* ptr = tmp.get();
	ret.assign(std::move(tmp));
	if constexpr (requires{ret.multi_callable = ptr;}) ret.multi_callable = ptr;
	if constexpr (requires{ret.multi_array = ptr;}) ret.multi_array = ptr;
	if constexpr (requires{ret.multi_object = ptr;}) ret.multi_object = ptr;
}

template<typename factory>
constexpr void data<factory>::mk_map_impl(const auto& f, data::self_type& result, auto&& key, auto&& val, auto&& ... tail) {
	result.put(self_type{std::forward<decltype(key)>(key)}, self_type{std::forward<decltype(val)>(val)});
	if constexpr (sizeof...(tail) != 0) mk_map_impl(f, result, std::forward<decltype(tail)>(tail)...);
}

template<typename factory>
constexpr auto data<factory>::mk_ca_val(auto&& v, auto&& ... args) {
	constexpr bool np = requires{ v(); };
	constexpr bool wp = sizeof...(args) > 0;
	if constexpr (np || wp)
		return mk_ca(std::forward<decltype(v)>(v), std::forward<decltype(args)>(args)...);
	else return std::move(v);
}

template<typename factory>
constexpr bool data<factory>::contains(const auto& val) const {
	return !is_none() && visit([this,&val](const auto& v){
		if(is_multiptr_obj(v)) return multi_object->contains(val);
		if constexpr(requires{v.contains(typename std::decay_t<decltype(v)>::key_type{});}) return v.contains(val);
		else if constexpr(requires{v==val;}) return v==val;
		else return throw_wrong_interface_error<details::interfaces::contains>(false);
	}, holder);
}

template<typename factory>
constexpr auto data<factory>::size() const {
	return visit( [this](const auto& v){
		if(is_multiptr_obj(v)) return multi_object->size();
		else if(is_multiptr_arr(v)) return multi_array->size();
		if constexpr(requires{ v.size(); }) return v.size();
		else if constexpr(requires{ v->size(); }) return v->size();
		else return sizeof(v); }, holder);
}

template<typename factory>
constexpr auto data<factory>::keys() const {
	return visit( [this](const auto& v){
		if(is_multiptr_obj(v)) return multi_object->keys(factory{});
		else return throw_wrong_interface_error<details::interfaces::keys>();
	}, holder);
}

template<typename factory>
constexpr data<factory>::self_type& data<factory>::push_back(data::self_type d) {
	if(is_none()) mk_empty_array();
	return visit([this,d=std::move(d)](auto& v) -> self_type& {
		if(is_multiptr_arr(v)) return multi_array->emplace_back(std::move(d));
		if constexpr(requires{ v.push_back(std::move(d)); }) { return v.push_back(std::move(d)); }
		else return throw_wrong_interface_error<details::interfaces::push_back,self_type&>(*this);
	}, holder);
}

template<typename factory>
constexpr data<factory>::self_type& data<factory>::put(data::self_type key, data::self_type value) {
	if(is_none()) mk_empty_object();
	return visit([this,&key,&value](auto& v) -> self_type& {
		if(is_multiptr_obj(v)) return multi_object->put(key, value);
		else return throw_wrong_interface_error<details::interfaces::put,self_type&>(*this);
	}, holder);
}

template<typename factory>
constexpr data<factory>::self_type& data<factory>::operator[](data::integer_t ind) {
	return visit([this,ind](auto& v)->self_type&{
		if(is_multiptr_arr(v)) return multi_array->at(ind);
		else return throw_wrong_interface_error<details::interfaces::at_ind, self_type&>(*this);
	}, holder);
}

template<typename factory>
constexpr data<factory>::self_type& data<factory>::operator[](const data::self_type &key) {
	return visit([this,&key](auto&v)->self_type& {
		if(is_multiptr_obj(v)) return multi_object->at(key);
		else return throw_wrong_interface_error<details::interfaces::at_key, self_type&>(*this);
	}, holder);
}

template<typename factory>
constexpr data<factory>::self_type data<factory>::call(auto &&params) {
	return visit([this,&params](auto& v) -> self_type {
		if(is_multiptr_cll(v)) return multi_callable->call(params);
		else throw_wrong_interface_error<details::interfaces::call>();
	}, holder);
}

} // namespace absd
