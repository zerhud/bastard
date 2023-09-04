/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of modegen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

namespace absd {

namespace details {

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

template<typename factory, typename data>
struct array : inner_counter {
	using vec_content = data;

	[[no_unique_address]] factory f;
	factory::template vector<vec_content> holder;


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
	factory::template vector<chunk> store;

	using key_type = key;
	using value_type = chunk;

	constexpr void insert(auto&& _v) {
		auto&& [k,v] = _v;
		store.emplace_back( chunk{ std::move(k), std::move(v) } );
	}

	constexpr auto& at(const key& fk) {
		for(auto&[k,v]:store) if(k==fk) return v;
		throw __LINE__;
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
struct type_erasure_callable1 {
	constexpr virtual ~type_erasure_callable1() noexcept =default ;
	constexpr virtual data call() =0 ;
};

template<typename factory, typename data>
struct type_erasure_callable {
	using factory_t = factory;
	using string_t = data::string_t;
	struct parameter_descriptor {
		string_t name;
		data default_value;
	};
	using params_t = decltype(std::declval<factory_t>().template mk_vec<parameter_descriptor>());

	constexpr virtual ~type_erasure_callable() noexcept =default ;
	constexpr virtual data call(data params) =0 ;
	constexpr virtual params_t params(const factory_t& f) const =0 ;
};

template<typename factory, typename data>
struct type_erasure_object {
	using size_t = decltype(sizeof(data));
	using factory_t = factory;

	virtual ~type_erasure_object() noexcept =default ;
	constexpr virtual data& at(const data& ind) =0 ;
	constexpr virtual data& put(data key, data value) =0 ;
	constexpr virtual data keys(const factory_t& f) const =0 ;
	constexpr virtual size_t size() const =0 ;
};

template<typename factory, typename data>
struct type_erasure_array {
	using size_t = decltype(sizeof(data));

	constexpr virtual ~type_erasure_array() noexcept =default ;

	constexpr virtual data& emplace_back(data d) =0 ;
	constexpr virtual data& at(size_t ind) =0 ;
	constexpr virtual size_t size() const =0 ;
};

template<typename factory, typename data> struct type_erasure_callable_object : type_erasure_callable<factory,data>, type_erasure_object<factory, data> {};

} // namespace details

// просто создать класс данных от фабрики, он должен содержать
//
// - массив таких же типов
// - объект (карта) таких же типов и в качестве ключа и в качестве значения
// - вызываемый объект с двумя параметрами:
//   - список позиционных параметров и
//   - карта именованных параметров
//   - также должен возращать дефолтные параметы (получит в карте именнованых, если не указаны)
//   - дефолтный объект должен работать на подобии std::function - type erasure
// 
// - реализиющие объекты должны иметь также счетчик ссылок и должны очиститься при его обнулении
// - объект данных достает все из фабрики, если там нет, то использует дефолтные типы
// - если нет типа в фабрике, то может понадобится, чтобы там был тип вектора и карты
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
	using holder_t = factory::template variant<
		typename factory::empty_t, bool, integer_t, float_point_t,
		string_t, array_t*, object_t*,
		details::type_erasure_callable<factory, self_type>*, details::type_erasure_object<factory, self_type>*, details::type_erasure_array<factory, self_type>*,
		details::type_erasure_callable_object<factory, self_type>*
	>;

	constexpr static self_type mk(auto&& v) {
		using v_type = std::decay_t<decltype(v)>;
		return self_type{};
	}

private:
	template<typename type> constexpr static const bool is_inner_counter_exists = requires(std::remove_pointer_t<std::decay_t<type>>& v){ v.increase_counter(); };

	holder_t holder;

	constexpr void allocate() {
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
public:

	constexpr data() {}
	constexpr data(auto v) requires (std::is_same_v<decltype(v), bool>) : holder(v) {}

	constexpr data(string_t v) : holder(v) {} //NOTE: bug in libstdc++ string cannot be moved here (gcc bugs 111258 111284)
	constexpr data(const typename string_t::value_type* v) : holder(string_t(v)) {}

	constexpr data(integer_t v) : holder(v) {}
	constexpr data(float_point_t v) : holder(v) {}
	constexpr data(self_type&& v) : holder(std::move(v.holder)) { v.holder=typename factory::empty_t{}; }
	constexpr data(const self_type& v) : holder(v.holder) { allocate(); }
	constexpr data(const data& v) : holder(v.holder) { allocate(); }
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
		return visit([this,key=std::move(key),value=std::move(value)](auto& v) -> self_type& {
			if constexpr(requires{ v->put(key,value); }) return v->put(key, value);
			else {
				factory::throw_wrong_interface_error("put");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}
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

	constexpr self_type call() {
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
	//NOTE: it seems there is ub in string realization after move
	//	static_assert( []{ data_type d; d="hel"; auto ret = ((string_t)d)[2]; return ret; }() == 'l' );
		static_assert( data_type{ integer_t{} }.size() == sizeof(integer_t) );
		static_assert( data_type{ string_t{"hello"} }.size() == 5 );
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
		return true;
	}

	constexpr static bool test_callable_cases() {
		struct data_type : data<factory, data_type> { using data<factory, data_type>::operator=; };

		//static_assert( data_type{ []{return data_type{(integer_t)1};} }() == 1 );
		return true;
	}

	constexpr static bool test() {
		return test_simple_cases() && test_array_cases() && test_callable_cases() && test_object_cases();
	}
};


} // namespace absd
