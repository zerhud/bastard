/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of bastard.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <iostream>

#include <utility>
#include <cassert> //TODO: remove after gcc bug will be fixed

namespace absd {

namespace details {

template<typename type, template<typename...>class tmpl> constexpr const bool is_specialization_of = false;
template<template<typename...>class type, typename... args> constexpr const bool is_specialization_of<type<args...>, type> = true;

template<typename factory> constexpr auto mk_integer_type() {
	if constexpr(!requires{ typename factory::integer_t; }) return int{};
	else return typename factory::integer_t{};
}
template<typename factory> constexpr auto mk_float_point_type() {
	if constexpr(!requires{ typename factory::float_pint_t; }) return double{};
	else return typename factory::float_pint_t{};
}

struct inner_counter {
	unsigned long int ref_counter = 0;
	constexpr auto increase_counter() { return ++ref_counter; }
	constexpr auto decrease_counter() { return --ref_counter; }
};

struct counter_interface {
	virtual ~counter_interface() noexcept =default ;
	constexpr virtual decltype(inner_counter::ref_counter) increase_counter() =0 ;
	constexpr virtual decltype(inner_counter::ref_counter) decrease_counter() =0 ;
};

template<typename factory, typename data>
struct array : inner_counter {
	using vec_content = data;

	[[no_unique_address]] factory f;
	std::decay_t<decltype(std::declval<factory>().template mk_vec<vec_content>())> holder;


	constexpr array(factory _f) requires (requires{ _f.template mk_vec<vec_content>(); }) : f(std::move(_f)), holder(f.template mk_vec<vec_content>()) {}
	constexpr array(factory _f) requires (!requires{ _f.template mk_vec<vec_content>(); }) : f(std::move(_f)) {}

	constexpr data& emplace_back(data d) { return holder.emplace_back(std::move(d)); }
	constexpr data& at(std::integral auto ind) { return holder.at(ind); }
	constexpr auto size() const { return holder.size(); }
};
template<typename data, typename factory> constexpr auto mk_array_type(const factory& f) {
	if constexpr(!requires{ typename factory::array_t; }) return array<factory, data>(f);
	else return typename factory::array_t{};
}

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

template<typename factory, typename data>
struct type_erasure_callable1 : counter_interface {
	constexpr virtual ~type_erasure_callable1() noexcept =default ;
	constexpr virtual data call() =0 ;
};

template<typename factory, typename data>
struct type_erasure_callable : counter_interface {
	using factory_t = factory;
	using string_t = typename data::string_t;
	struct parameter_descriptor {
		string_t name;
		data default_value;
	};
	using params_t = decltype(std::declval<factory_t>().template mk_vec<parameter_descriptor>());

	constexpr virtual ~type_erasure_callable() noexcept =default ;
	constexpr virtual data call(data params) =0 ;
	constexpr virtual params_t params(const factory_t& f) const =0 ;
};

template<typename factory, typename data> // cannot to be derived from both: cannot to be constexpr and have virtual base class
struct type_erasure_callable_both : type_erasure_callable<factory, data> {
	constexpr virtual data call() =0 ;
};

template<typename factory, typename data>
struct type_erasure_object : counter_interface {
	using factory_t = factory;

	virtual ~type_erasure_object() noexcept =default ;
	constexpr virtual bool contains(const data& key) const =0 ;
	constexpr virtual data& at(const data& ind) =0 ;
	constexpr virtual data& put(data key, data value) =0 ;
	constexpr virtual data keys(const factory_t& f) const =0 ;
	constexpr virtual decltype(sizeof(data)) size() const =0 ;
};

template<typename factory, typename data>
struct type_erasure_array : counter_interface {
	constexpr virtual ~type_erasure_array() noexcept =default ;

	constexpr virtual data& emplace_back(data d) =0 ;
	constexpr virtual data& at(typename data::integer_t ind) =0 ;
	constexpr virtual decltype(sizeof(data)) size() const =0 ;
};

template<typename factory, typename data> struct type_erasure_callable_object : type_erasure_callable<factory,data>, type_erasure_object<factory, data> {};

} // namespace details

// просто создать класс данных от фабрики, он должен содержать
//
// - вызываемый объект с двумя параметрами:
//   - список позиционных параметров и
//   - карта именованных параметров
//   - также должен возращать дефолтные параметы (получит в карте именнованых, если не указаны)
//   - дефолтный объект должен работать на подобии std::function - type erasure
// 
// - так как std::{,unordered_}map (похоже) не может быть использована за неимением constexpr
//   методов, то операции с ней нужно проверить в runtime

template<typename factory, typename crtp>
struct data {
	static_assert( noexcept( std::declval<factory>().deallocate((int*)nullptr) ), "for safity delete allocated objects the dealocate method must to be noexcept" );

	using self_type = crtp;//data<factory, crtp>;
	using base_data_type = data<factory, crtp>;
	using array_t = decltype(details::mk_array_type<crtp>(std::declval<factory>()));
	using object_t = decltype(details::mk_object_type<crtp>(std::declval<factory>()));
	using string_t = typename factory::string_t;
	using integer_t = decltype(details::mk_integer_type<factory>());
	using float_point_t = decltype(details::mk_float_point_type<factory>());

	using te_callable1 = details::type_erasure_callable1<factory, self_type>;
	using te_callable = details::type_erasure_callable<factory, self_type>;
	using te_callable_both = details::type_erasure_callable_both<factory, self_type>;
	using te_array = details::type_erasure_array<factory, self_type>;
	using te_object = details::type_erasure_object<factory, self_type>;

	using holder_t = typename factory::template variant<
		typename factory::empty_t, bool, integer_t, float_point_t,
		string_t, array_t*, object_t*,
		te_callable*, te_callable1*, te_callable_both*,
		details::type_erasure_object<factory, self_type>*, details::type_erasure_array<factory, self_type>*,
		details::type_erasure_callable_object<factory, self_type>*
	>;

	constexpr static auto mk_param(auto&& name) {
		return name;
	}
	constexpr static auto mk_param(auto&& _name, self_type _def_val) {
		struct desc {
			std::decay_t<decltype(_name)> name;
			std::decay_t<decltype(_def_val)> def_val;
		};
		return desc{std::forward<decltype(_name)>(_name), std::move(_def_val)};
	}
	template<typename functor>
	struct callable { ///< allow to specify default arguments, use it as function in code and so on...
		functor fnc;
		self_type params_info;
		constexpr explicit callable(functor f, auto&&... params) : fnc(std::move(f)) {
			params_info.mk_empty_array();
			(create_param(std::forward<decltype(params)>(params)),..., 1);
		}
		constexpr auto operator()(auto&&... args) const {
			return call_with_params(std::forward<decltype(args)>(args)...);
		}
		constexpr auto call(self_type params) const {
			return call_with_combined_params<0>(params);
		}
	private:
		constexpr void create_param(auto&& param) {
			self_type desk;
			if constexpr (!requires{param.name;param.def_val;}) desk.put(self_type{string_t{"name"}}, self_type{param});
			else {
				desk.put(self_type{string_t{"name"}}, self_type{param.name});
				desk.put(self_type{string_t{"val"}}, self_type{param.def_val});
			}
			params_info.push_back(std::move(desk));
		}
		constexpr auto call_with_params(auto&&... params) const {
			if constexpr (requires{fnc(std::forward<decltype(params)>(params)...);})
				return fnc(std::forward<decltype(params)>(params)...);
			else {
				auto& desk = params_info[(integer_t) sizeof...(params)];
				if(desk.size() == 2) return call_with_params(std::forward<decltype(params)>(params)..., desk[self_type{string_t{"val"}}]);
				else {
					factory::throw_wrong_interface_error("call with wrong parameter number");
					return call_with_params(std::forward<decltype(params)>(params)..., self_type{});
				}
			}
		}
		template<auto ind>
		constexpr auto call_with_combined_params(const auto& user_params, auto&&... params) const {
			if constexpr (requires{fnc(std::forward<decltype(params)>(params)...);}) return fnc(std::forward<decltype(params)>(params)...);
			else {
				if(user_params.contains(self_type{ind})) return call_with_combined_params<ind+1>(user_params, std::forward<decltype(params)>(params)..., user_params[self_type{ind}]);
				else if(user_params.contains(params_info[ind][self_type{string_t{"name"}}]))
					return call_with_combined_params<ind+1>(user_params, std::forward<decltype(params)>(params)..., user_params[params_info[ind][self_type{string_t{"name"}}]]);
				else return call_with_combined_params<ind+1>(user_params, std::forward<decltype(params)>(params)..., params_info[ind][self_type{string_t{"val"}}]);
			}
		}
	};

	constexpr static self_type mk(auto&& v) { return mk(factory{}, std::forward<decltype(v)>(v)); }
	constexpr static self_type mk(const factory& f, auto&& v) requires (!details::is_specialization_of<std::decay_t<decltype(v)>, callable>) {
		using v_type = std::decay_t<decltype(v)>;
		constexpr const bool is_call_np = requires{ v(); };
		constexpr const bool is_array = requires{ v.at(integer_t{}); };
		constexpr const bool is_object = requires{ v.at(self_type{}); };
		self_type ret{};
		if constexpr( is_call_np && is_object ) {
		}
		else if constexpr( is_call_np ) {
			struct te : te_callable1 {
				v_type val;
				constexpr te(v_type v) : val(std::move(v)) {}
				constexpr self_type call() override {
					if constexpr(requires{ {val()}->std::same_as<self_type>; }) return val();
					else return (val(), self_type{});
				}
			};
			ret = mk_coutner_and_assign<te_callable1, te>(f, std::forward<decltype(v)>(v));
		}
		else if constexpr( is_array ) {
			struct te : te_array {
				v_type val;
				constexpr te(v_type v) : val(std::move(v)) {}

				constexpr self_type& emplace_back(self_type d) override { return val.emplace_back(std::move(d)); }
				constexpr self_type& at(integer_t ind) override { return val.at(ind); }
				constexpr decltype(sizeof(self_type)) size() const override { return val.size(); }
			};
			ret = mk_coutner_and_assign<te_array, te>(f, std::forward<decltype(v)>(v));
		}
		else if constexpr( is_object ) {
			struct te : te_object {
				v_type val;
				constexpr te(v_type v) : val(std::move(v)) {}

				constexpr bool contains(const self_type& key) const override { return val.contains(key); }
				constexpr self_type& at(const self_type& ind) override { return val.at(ind); }
				constexpr self_type& put(self_type key, self_type value) override {
					struct kv{ self_type k, v; };
					val.insert( kv{ key, value} );
					return val.at(key);
				}
				constexpr self_type keys(const factory& f) const override {
					self_type ret;
					ret.mk_empty_array();
					for(const auto& [k,v]:val) ret.push_back(k);
					return ret;
				}
				constexpr decltype(sizeof(self_type)) size() const override { return val.size(); }
			};
			ret = mk_coutner_and_assign<te_object, te>(f, std::forward<decltype(v)>(v));
		}
		return ret;
	}
	constexpr static self_type mk(auto&& v) requires( !std::is_same_v<std::decay_t<decltype(v)>, factory> && details::is_specialization_of<std::decay_t<decltype(v)>, callable> ){
		//TODO: do we realy need it? or it can to be replaced with first mk method?
		return mk( factory{}, std::forward<decltype(v)>(v) ); }
	constexpr static self_type mk(const factory& f, auto&& v) requires( details::is_specialization_of<std::decay_t<decltype(v)>, callable> ){
		using val_type = std::decay_t<decltype(v)>;
		constexpr const bool is_object = requires{ v.at(self_type{}); };

		struct te_base {
			using params_t = typename te_callable::params_t;
			val_type v;
			constexpr explicit te_base(val_type&& v) : v(std::forward<decltype(v)>(v)) {}
			constexpr self_type call(self_type params) {return self_type{3};}
			constexpr params_t params(const factory& f) const {return params_t{};}
			constexpr self_type call_without_params() {
				if constexpr(requires{ self_type{v()}; }) return self_type{v()};
				else if constexpr(requires{ {v()} -> std::same_as<void>; })  return (v(), self_type{});
				else {
					std::unreachable();
					return self_type{};
				}
			}
		};

		if constexpr(requires{ {v()} -> std::same_as<void>; } || requires{ self_type{v()}; }) {
			struct te : te_base, te_callable_both {
				using params_t = typename te_callable::params_t;
				constexpr explicit  te(val_type&& v) : te_base(std::forward<decltype(v)>(v)) {}
				constexpr self_type call() override {return te_base::call_without_params();}
				constexpr self_type call(self_type params) override {return te_base::call(std::move(params));}
				constexpr params_t params(const factory& f) const override {return te_base::params(f);}
			};
			return mk_coutner_and_assign<te_callable_both, te>(f, std::forward<decltype(v)>(v));
		}
		else {
			struct te : te_base, te_callable {
				using params_t = typename te_callable::params_t;
				constexpr self_type call(self_type params) override { return te_base::call(std::move(params)); }
				constexpr params_t params(const factory &f) const override { return te_base::params(f); }
			};
			return mk_coutner_and_assign<te_callable, te>(f, te(std::forward<decltype(v)>(v)));
		}
	}

private:
	template<typename type> constexpr static const bool is_inner_counter_exists = requires(std::remove_pointer_t<std::decay_t<type>>& v){ v.increase_counter(); };

	holder_t holder;

	constexpr void allocate() noexcept {
		visit([](auto& v){
			if constexpr(is_inner_counter_exists<decltype(v)>) {
				v->increase_counter();
			}
		}, holder);
	}
	constexpr void deallocate() {
		visit([this](auto& v){
			if constexpr(is_inner_counter_exists<decltype(v)>) {
				if(v->decrease_counter()==0) {
					factory::deallocate(v);
					holder = typename factory::empty_t{};
				}
			}
			}, holder);
	}

	template<typename type>
	struct counter_maker : details::inner_counter, type {
		constexpr counter_maker(auto&&... args) : type(std::forward<decltype(args)>(args)...) {}
		constexpr decltype(inner_counter::ref_counter) increase_counter() override { return inner_counter::increase_counter(); }
		constexpr decltype(inner_counter::ref_counter) decrease_counter() override { return inner_counter::decrease_counter(); }
	};
	template<typename from, typename type>
	constexpr static self_type mk_coutner_and_assign(const factory& f, auto&& v) {
		self_type ret;
		auto tmp = f.mk_ptr(counter_maker<type>(std::forward<decltype(v)>(v)));
		ret.assign(static_cast<from*>(tmp.get()));
		tmp.release();
		return ret;
	}
public:

	constexpr data() {}
	constexpr data(auto v) requires (std::is_same_v<decltype(v), bool>) : holder(v) {}

	constexpr data(string_t v) : holder(v) {} //NOTE: bug in gcc https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111284
	constexpr data(const typename string_t::value_type* v) : holder(string_t(v)) {}

	constexpr data(integer_t v) : holder(v) {}
	constexpr data(float_point_t v) : holder(v) {}
	constexpr data(self_type&& v) : holder(std::move(v.holder)) { v.holder=typename factory::empty_t{}; }
	constexpr data(const self_type& v) : holder(v.holder) { allocate(); }
	constexpr data(const data& v) : holder(v.holder) { allocate(); }
	//NOTE: bug in gcc workaround: use holder = declytpe(holder){v.holder} ?
	constexpr data(data&& v) : holder(std::move(v.holder)) { v.holder=typename factory::empty_t{}; }
	constexpr ~data() { deallocate(); }

	constexpr self_type& assign() {
		deallocate();
		holder = typename factory::empty_t{};
		return static_cast<self_type&>(*this);
	}
	constexpr self_type& assign(auto&& v) {
		deallocate();
		if constexpr(requires{ holder = std::forward<decltype(v)>(v); })
			holder = std::forward<decltype(v)>(v);
		else {
			holder = v.get();
			v.release();
		}
		allocate();
		return static_cast<self_type&>(*this);
	}

	constexpr auto& operator=(const data& v) { return assign(v.holder); }
	constexpr auto& operator=(const self_type& v) { return assign(v.holder); }
	constexpr auto& operator=(data&& v) { return assign(std::move(v.holder)); }
	constexpr auto& operator=(self_type&& v) { return assign(std::move(v.holder)); }

	constexpr auto& operator=(string_t v){ return assign(std::move(v)); }
	constexpr auto& operator=(integer_t v){ return assign(v); }
	constexpr auto& operator=(float_point_t v){ return assign(v); }

	constexpr explicit operator bool() const { return get<bool>(holder); }
	constexpr operator string_t() const { return get<string_t>(holder); }
	constexpr operator integer_t() const { return get<integer_t>(holder); }
	constexpr operator float_point_t() const { return get<float_point_t>(holder); }

	constexpr auto cur_type_index() const { return holder.index(); }
	constexpr bool is_int() const { return holder.index() == 2; }
	constexpr bool is_none() const { return holder.index() == 0; }
	constexpr bool is_bool() const { return holder.index() == 1; }
	constexpr bool is_string() const { return holder.index() == 4; }
	constexpr bool is_float_point() const { return holder.index() == 3; }
	constexpr bool is_array() const { return visit( [](const auto& v){ return requires(std::decay_t<decltype(v)> vv){ vv->at( integer_t{} ); }; }, holder); }
	constexpr bool is_object() const { return visit( [](const auto& v){ return requires(std::decay_t<decltype(v)> vv){ vv->at( crtp{} ); }; }, holder); }
	constexpr bool is_callable() const { return visit( [](const auto& v) {
			return requires{ v->call(); } || requires{ v->call({}); }; }, holder); }

	constexpr int contains(const auto& val) const {
		return visit([this,&val](const auto& v){
			if constexpr(requires{v->contains(val);}) return v->contains(val);
			else if constexpr(requires{v.contains(typename std::decay_t<decltype(v)>::key_type{});}) return v.contains(val);
			else if constexpr(requires{v==val;}) return v==val;
			else {
				factory::throw_wrong_interface_error("contains");
				std::unreachable();
				return false;
			}
		}, holder);
	}
	constexpr auto size() const {
		return visit( [](const auto& v){
			if constexpr(requires{ v.size(); }) return v.size();
			else if constexpr(requires{ v->size(); }) return v->size();
			else return sizeof(v); }, holder);
	}
	constexpr auto keys() const {
		return visit( [](const auto& v){
			if constexpr(requires{ v->keys(factory{}); }) return v->keys(factory{});
			else {
				factory::throw_wrong_interface_error("keys");
				std::unreachable();
				return self_type{};
			}
			}, holder);
	}

	constexpr self_type& mk_empty_array() { return assign(factory::mk_ptr(details::mk_array_type<crtp>(factory{}))); }
	constexpr self_type& mk_empty_object() { return assign(factory::mk_ptr(details::mk_object_type<crtp>(factory{}))); }
	constexpr self_type& push_back(self_type d) {
		return visit([this,d=std::move(d)](auto& v) -> self_type& { 
			if constexpr(requires{ v->emplace_back(std::move(d)); }) { return v->emplace_back(std::move(d)); }
			else {
				factory::throw_wrong_interface_error("push_back");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}
	constexpr self_type& put(self_type key, self_type value) {
		if(is_none()) mk_empty_object();
		return visit([this,&key,&value](auto& v) -> self_type& {
			if constexpr(requires{ v->put(key,value); }) return v->put(key, value);
			else {
				factory::throw_wrong_interface_error("put");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}
	constexpr const self_type& operator[](integer_t ind) const {return const_cast<base_data_type&>(*this)[ind];}
	constexpr self_type& operator[](integer_t ind){
		return visit([this,ind](auto& v)->self_type&{ 
			if constexpr(requires{ v->emplace_back(self_type{}); v->at(ind); }) { return v->at(ind); }
			else {
				factory::throw_wrong_interface_error("operator[ind]");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}
	constexpr const self_type& operator[](self_type key) const { return const_cast<base_data_type&>(*this)[std::move(key)]; }
	constexpr self_type& operator[](self_type key){
		return visit([this,key=std::move(key)](auto&v)->self_type& {
			if constexpr(requires{ v->at(key); }) return v->at(key);
			else {
				factory::throw_wrong_interface_error("operator[key]");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}

	constexpr self_type cmpget_workaround(const string_t& key) {
		return visit([this,&key](auto&v)->self_type {
			if constexpr(requires{ v->cmpget_workaround(key); }) return v->cmpget_workaround(key);
			else {
				factory::throw_wrong_interface_error("cmpget_workaround");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}

	constexpr self_type call() {
		return visit([this](auto& v) -> self_type {
			if constexpr(requires{ {v->call()}->std::same_as<self_type>; }) return v->call();
			else if constexpr(requires{ v->call(); }) return (v->call(), self_type{});
			else {
				factory::throw_wrong_interface_error("operator()");
				std::unreachable();
				return self_type{};
			}
		}, holder);
	}
	constexpr self_type call(const auto& params) {
	}

	friend constexpr bool operator==(const self_type& left, const self_type& right) {
		return left.holder.index() == right.holder.index() && visit([](const auto& l, const auto& r){
			if constexpr(requires{ l==r; }) return l==r;
			else return false;
			}, left.holder, right.holder);
	}

	constexpr static bool test_simple_cases() {
		struct data_type : data<factory,data_type> {using data<factory,data_type>::operator=;};

		static_assert( data_type{}.is_none(), "data is empty by defualt" );
		static_assert( data_type{ (integer_t)10 }.is_none() == false );
		static_assert( data_type{ (integer_t)10 }.assign().is_none() );
		static_assert( data_type{ (float_point_t).5 }.is_int() == false );
		static_assert( data_type{ (float_point_t).5 }.assign( (integer_t)3 ).is_int() );
		static_assert( data_type{ string_t{} }.is_string() == true );
		static_assert( data_type{ string_t{} }.is_array() == false );
		static_assert( []{ data_type d; d=10; return (integer_t)d; }() == 10 );
	//NOTE: string cannot to be tested in compile time due gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111284
	//	static_assert( []{ data_type d; d="hel"; auto ret = ((string_t)d)[2]; return ret; }() == 'l' );
		static_assert( data_type{ integer_t{} }.size() == sizeof(integer_t) );
		static_assert( data_type{ string_t{"hello"} }.size() == 5 );
		static_assert( data_type{3} == data_type{3});
		return true;
	}

	constexpr static bool test_array_cases() {
		struct data_type : data<factory,data_type> {using data<factory,data_type>::operator=;};

		static_assert( data_type{}.mk_empty_array().is_array() == true );
		static_assert( data_type{}.mk_empty_array().is_object() == false );
		static_assert( data_type{(integer_t)10}.mk_empty_array().is_array() == true );
		static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); return (integer_t)d[0]; }() == 10 );
		static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); auto dd = d; return (integer_t)dd[0]; }() == 10 );
		static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); auto dd = std::move(d); return (integer_t)dd[0]; }() == 10 );
		static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); auto dd = std::move(d); return dd.size(); }() == 1 );

		static_assert( []{
			auto vec = factory{}.template mk_vec<data_type>();
			vec.emplace_back(data_type{(integer_t)1});
			vec.emplace_back(data_type{(integer_t)3});
			auto d = data_type::mk(std::move(vec));
			return (integer_t)d[1]; }() == 3);

		return true;
	}

	constexpr static bool test_object_cases() {
		struct data_type : data<factory,data_type> {using data<factory,data_type>::operator=;};

		static_assert( data_type{}.mk_empty_object().is_object() == true );
		static_assert( data_type{}.mk_empty_object().is_array() == false );
		static_assert( []{data_type d{}; d.mk_empty_object(); d.put(data_type{1}, data_type{7}); return (integer_t)d[data_type{1}];}() == 7 );
		static_assert( []{ data_type d; d.put(data_type{1}, data_type{7}); d.put(data_type{2}, data_type{8}); return d.size(); }() == 2 );
		static_assert( []{ data_type d;
			d.put(data_type{1}, data_type{7});
			d.put(data_type{2}, data_type{8});
			auto keys = d.keys();
			return (keys.size() == 2) + (2*((integer_t)keys[0] == 1)) + (4*((integer_t)keys[1] == 2)); }() == 7 );

		static_assert( []{
			details::constexpr_kinda_map<factory,data_type,data_type> v;
			v.insert(std::make_pair(data_type{1}, data_type{2}));
			return v.contains(data_type{1});
		}() );
		static_assert( (integer_t)[]{
			details::constexpr_kinda_map<factory,data_type,data_type> v;
			v.insert(std::make_pair(data_type{1}, data_type{3}));
			return data_type::mk(std::move(v))[data_type{1}];}() == 3 );

		static_assert( []{
			data_type d;d.mk_empty_object();
			d.put(data_type{1}, data_type{7});
			return d.contains(data_type{1}) + (2*!d.contains(data_type{7}));
		}() == 3);
		return true;
	}

	constexpr static bool test_callable_cases() {
		struct data_type : data<factory, data_type> { using data<factory, data_type>::operator=; };

		static_assert( (integer_t)data_type::mk( []{return data_type{1};} ).call() == 1 );
		static_assert( (integer_t)data_type::mk( []{return data_type{2};} ).call() == 2 );
		static_assert( data_type::mk([]{}).is_callable() );
		static_assert( data_type::mk([]{}).call().is_none() );

		static_assert( data_type::mk(typename data_type::callable([](){})).is_callable() );
		static_assert( (integer_t)data_type::mk(typename data_type::callable([](){return data_type{2};})).call() == 2 );
		static_assert( (integer_t)data_type::mk(typename data_type::callable([](){return 2;})).call() == 2 );

		static_assert( (integer_t)callable([](int v){return v+1;})(2) == 3 );
		static_assert( (integer_t)callable([](int l, int r){return l+r;})(7, 13) == 20 );

		return true;
	}

	static bool test_callable_cases_rt() {
		//NOTE: cannot test in ct due gcc bug with move
		assert( []{
            callable wp([](int v) { return v + 1; }, mk_param("v", self_type{2}));
            return (integer_t)wp();}() == 3);
		callable w_lpr([](integer_t l, integer_t r) { return l + r; }, mk_param("l"), mk_param("r", self_type{2}));
		assert( (integer_t)w_lpr(1) == 3 );
		assert( (integer_t)w_lpr(1, 3) == 4 );
		callable wd_lpr([](integer_t l, integer_t r) { return l + r; }, mk_param("l", self_type{1}), mk_param("r", self_type{2}));
		assert( [&wd_lpr]{
			self_type params; params.put(self_type{0}, self_type{4}); params.put(self_type{1}, self_type{7});
			return (integer_t)wd_lpr.call(params);
		}() == 11 );
		assert( [&wd_lpr]{
			self_type params; params.put(self_type{0}, self_type{4});
			return (integer_t)wd_lpr.call(params);
		}() == 6 );
		assert( [&wd_lpr]{
			self_type params; params.put(self_type{1}, self_type{4});
			return (integer_t)wd_lpr.call(params);
		}() == 5 );
		assert( [&wd_lpr]{
			self_type params; params.put(self_type{"r"}, self_type{4});
			return (integer_t)wd_lpr.call(params);
		}() == 5 );
		assert( [&wd_lpr]{
			self_type params; params.put(self_type{"l"}, self_type{4});
			return (integer_t)wd_lpr.call(params);
		}() == 6 );
		assert( [&wd_lpr]{
			self_type params; params.put(self_type{0}, self_type{4}); params.put(self_type{"r"}, self_type{7});
			return (integer_t)wd_lpr.call(params);
		}() == 11 );
		assert( [&wd_lpr]{
			self_type params; params.put(self_type{0}, self_type{4}); params.put(self_type{"l"}, self_type{40});
			return (integer_t)wd_lpr.call(params);
		}() == 6 );
		return true;
	}

	constexpr static bool test() {
		return test_simple_cases() && test_array_cases() && test_callable_cases() && test_object_cases();
	}
};


} // namespace absd
