//
// Created by zerhud on 01/03/24.
//

#pragma once

#include <utility>
#include "type_erasure.hpp"

namespace absd::details {

//NOTE: no constexpr map in cpp for now
//TODO: use https://github.com/karel-burda/constexpr-hash-map ?
template<typename factory, typename key, typename value>
struct constexpr_kinda_map {
	struct chunk {
		key k;
		value v;
	};
	std::decay_t<decltype(std::declval<factory>().template mk_vec<chunk>())> store;

	using key_type = key;
	using value_type = chunk;

	constexpr bool contains(const auto& v) const {
		for(const auto&[k,_]:store) if(k==v) return true;
		return false;
	}
	constexpr void insert(auto&& _v) {
		auto&& [k,v] = _v;
		store.emplace_back( chunk{ std::move(k), std::move(v) } );
	}

	constexpr auto& at(const key& fk) {
		for(auto&[k,v]:store) if(k==fk) return v;
		throw __LINE__; //TODO: what to do if key not found
	}

	constexpr auto size() const { return store.size(); }

	constexpr auto end() { return store.end(); }
	constexpr auto begin() { return store.begin(); }
	constexpr auto end() const { return store.end(); }
	constexpr auto begin() const { return store.begin(); }
};
template<typename key, typename value, typename factory> constexpr auto mk_map_type(const factory& f) {
	if constexpr(requires{ f.template mk_map<key,value>(); }) return f.template mk_map<key,value>();
	else {
		using map_t = constexpr_kinda_map<factory, key, value>;
		return map_t{f.template mk_vec<typename map_t::chunk>()};
	}
}

template<typename factory, typename data>
struct type_erasure_object  {
	using factory_t = factory;

	virtual ~type_erasure_object() noexcept =default ;
	constexpr virtual bool contains(const data& key) const =0 ;
	constexpr virtual data& at(const data& ind) =0 ;
	constexpr virtual data& put(data key, data value) =0 ;
	constexpr virtual data keys(const factory_t& f) const =0 ;
	constexpr virtual decltype(sizeof(data)) size() const =0 ;
};

template<typename data, typename map_t>
struct object : inner_counter {
	map_t map;

	constexpr object(map_t map) : map(std::move(map)) {}
	constexpr bool contains(const data& key) const { return map.contains(key); }
	constexpr data& at(const data& ind) { return map.at(ind); }
	constexpr data& put(data key, data value) {
		struct kv{ data k, v; };
		map.insert( kv{ key, value} );
		return map.at(key);
	}
	constexpr data keys(const auto& f) const {
		data ret;
		ret.mk_empty_array();
		for(const auto& [k,v]:map) ret.push_back(k);
		return ret;
	}
	constexpr data cmpget_workaround(const auto& f) const {
		for(auto& [k,v]:map) if((decltype(f))k==f) return v;
		return data{};
	}

	constexpr auto size() const { return map.size(); }
};
template<typename data, typename factory> constexpr auto mk_object_type(const factory& f) {
	if constexpr(requires{ typename factory::object_t; }) return typename factory::object_t{};
	else {
		using map_t = decltype(mk_map_type<data,data>(std::declval<factory>()));
		return object<data,map_t>(mk_map_type<data,data>(factory{}));
	}
}

template<typename data_type>
constexpr auto mk_te_object(const auto& f, auto&& src) {
	using src_type = std::decay_t<decltype(src)>;
	using te_object = type_erasure_object<typename data_type::factory_t, data_type>;
	constexpr const bool is_object = requires{ src.orig_val().at(data_type{}); };
	if constexpr (!is_object) return std::move(src);
	else {
		struct te : src_type, te_object {
			constexpr te(src_type s) : src_type(std::move(s)) {}

			constexpr decltype(sizeof(data_type)) size() const override { return this->orig_val().size(); }
			constexpr bool is_obj() const override { return true; }
			constexpr bool contains(const data_type &key) const override { return this->orig_val().contains(key); }
			constexpr data_type& at(const data_type &ind) override { return this->orig_val().at(ind); }
			constexpr data_type& put(data_type key, data_type value) override {
				struct kv {
					data_type k, v;
				};
				this->orig_val().insert(kv{key, value});
				return this->orig_val().at(key);
			}

			constexpr data_type keys(const typename data_type::factory_t &f) const override {
				data_type ret;
				ret.mk_empty_array();
				for (const auto &[k, v]: this->orig_val()) ret.push_back(k);
				return ret;
			}
		};

		return te{std::forward<decltype(src)>(src)};
	}
}

} // namespace absd::details

