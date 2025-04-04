#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>
#include "concepts.hpp"
#include "exceptions.hpp"
#include "type_erasure.hpp"

namespace absd::details {

//NOTE: no constexpr map in cpp for now
//TODO: use https://github.com/karel-burda/constexpr-hash-map ?
//NOTE: this class is only used inside absd, so the value is always absd::data
template<typename factory, typename key, typename value>
struct constexpr_kinda_map {
	static constexpr int absd_map_replacer=0;
	struct chunk {
		key k;
		value v;
	};
	std::decay_t<decltype(mk_vec<chunk>(std::declval<factory>()))> store;

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
		throw_key_not_found(fk.factory, fk);
	}

	constexpr auto size() const { return store.size(); }

	constexpr auto end() { return store.end(); }
	constexpr auto begin() { return store.begin(); }
	constexpr auto end() const { return store.end(); }
	constexpr auto begin() const { return store.begin(); }

	constexpr bool is_eq(const constexpr_kinda_map& other) const {
		if(store.size() != other.store.size()) return false;
		for(auto i=0;i<store.size();++i)
			if(store[i].k!=other.store[i].k || store[i].v!=other.store[i].v) return false;
		return true;
	}
};
template<typename key, typename value, typename factory> constexpr auto mk_map_type(const factory& f) {
	if constexpr(requires{ mk_map<key,value>(f); }) return mk_map<key,value>(f);
	else {
		using map_t = constexpr_kinda_map<factory, key, value>;
		return map_t{mk_vec<typename map_t::chunk>(f)};
	}
}

template<typename factory, typename data>
struct type_erasure_object  {
	using factory_t = factory;

	virtual ~type_erasure_object() noexcept =default ;
	constexpr virtual bool contains(const data& key) const =0 ;
	constexpr virtual data at(const data& ind) =0 ;
	constexpr virtual data& put(data key, data value) =0 ;
	constexpr virtual data keys(const factory_t& f) const =0 ;
	constexpr virtual decltype(sizeof(data)) size() const =0 ;
};

template<typename data, typename map_t>
struct object : inner_counter {
	map_t map;

	constexpr object(map_t map) : map(std::move(map)) {}
	constexpr bool contains(const data& key) const { return map.contains(key); }
	constexpr data at(const data& ind) { return map.at(ind); }
	constexpr data& put(data key, data value) {
		struct kv{ data k, v; };
		map.insert( kv{ key, value} );
		return map.at(key);
	}
	constexpr data keys(const auto& f) const {
		data ret{f};
		ret.mk_empty_array();
		for(const auto& [k,v]:map) ret.push_back(k);
		return ret;
	}
	constexpr auto size() const { return map.size(); }
};
template<typename data, typename factory> constexpr auto mk_object_type(const factory& f) {
	if constexpr(requires{ mk_object(f); }) return mk_object(f);
	else {
		using map_t = decltype(mk_map_type<data,data>(std::declval<factory>()));
		return object<data,map_t>(mk_map_type<data,data>(factory{}));
	}
}

template<typename data_type>
constexpr auto mk_te_object(const auto& f, auto&& src) {
	using src_type = std::decay_t<decltype(src)>;
	using te_object = type_erasure_object<typename data_type::factory_t, data_type>;
	if constexpr (!as_object<decltype(src.orig_val()), data_type>) return std::move(src);
	else {
		struct te : src_type, te_object {
			constexpr te(src_type s) : src_type(std::move(s)) {}

			constexpr decltype(sizeof(data_type)) size() const override { return this->orig_val().size(); }
			constexpr bool is_obj() const override {
				if constexpr (requires{this->orig_val().is_object();}) return this->orig_val().is_object();
				else return true;
			}
			constexpr bool contains(const data_type& key) const override {
				if constexpr(requires{this->orig_val().contains(key);}) return this->orig_val().contains(key);
				else return false;
			}
			constexpr data_type at(const data_type& ind) override {
				if constexpr (requires{ this->orig_val().absd_map_replacer; }) { if(!this->orig_val().contains(ind)) return data_type{}; }
				return this->orig_val().at(ind);
			}
			constexpr data_type& put(data_type key, data_type value) override {
				struct kv {
					data_type k, v;
				};
				if constexpr (requires{inert(this->orig_val(), key, value);}) {
					insert(this->orig_val(), key, value);
					return this->orig_val().at(key);
				}
				else if constexpr (requires{this->orig_val().insert({key, value});}) {
					this->orig_val().insert({key, value});
					return this->orig_val().at(key);
				}
				else if constexpr (requires{this->orig_val().insert(kv{key,value});}) {
					this->orig_val().insert(kv{key, value});
					return this->orig_val().at(key);
				}
				else if constexpr(requires{put(this->orig_val(), key, std::move(value));}) {
					put(this->orig_val(), key, std::move(value));
					return this->orig_val.at(key);
				}
				else if constexpr(requires{this->orig_val().put(key, std::move(value));}) {
					this->orig_val().put(key, std::move(value));
					return this->orig_val.at(key);
				}
				else throw_wrong_interface_error<interfaces::put>(key.factory);
			}

			constexpr data_type keys(const typename data_type::factory_t& f) const override {
				data_type ret{f};
				ret.mk_empty_array();
				if constexpr (iteratable<decltype(src.orig_val())>)
					for (const auto&[k,v]:this->orig_val()) ret.push_back(k);
				else if constexpr (has_keys<decltype(src.orig_val()), data_type>) {
					auto keys = [&] {
						if constexpr (requires{this->orig_val().keys(f);}) return this->orig_val().keys(f);
						else return this->orig_val().keys();
					}();
					for(auto& v:keys) ret.push_back(data_type{f, std::move(v)});
				}
				return ret;
			}
		};

		return te{std::forward<decltype(src)>(src)};
	}
}

} // namespace absd::details
