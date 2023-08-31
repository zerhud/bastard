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
		[[no_unique_address]] factory f;
		factory::template vector<data*> holder;


		constexpr array(factory _f) requires (requires{ _f.template mk_vec<data*>(); }) : f(std::move(_f)), holder(f.template mk_vec<data*>()) {}
		constexpr array(factory _f) requires (!requires{ _f.template mk_vec<data*>(); }) : f(std::move(_f)) {}

		//constexpr data& emplace_back(data d) { return holder.emplace_back(std::move(d)); }
		constexpr data& emplace_back(data d) { return *holder.emplace_back(f.mk_ptr(std::move(d))); }
		constexpr data& at(auto ind) { return *holder.at(ind); }
		constexpr auto size() const { return holder.size(); }
		constexpr auto deallocate() {
			for(auto& v:holder) f.deallocate(v);
		}
	};
	template<typename factory, typename data> constexpr auto mk_array_type() {
		if constexpr(!requires{ typename factory::array_t; }) return array<factory, data>(factory{});
		else return typename factory::array_t{};
	}

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
	using self_type = crtp;//data<factory, crtp>;
	using base_data_type = data<factory, crtp>;
	using array_t = decltype(details::mk_array_type<factory,crtp>());
	using string_t = typename factory::string_t;
	using integer_t = decltype(details::mk_integer_type<factory>());
	using float_point_t = decltype(details::mk_float_point_type<factory>());
	using holder_t = factory::template variant<typename factory::empty_t, bool, integer_t, float_point_t, string_t, array_t>;

private:
	holder_t holder;

	constexpr void allocate() {
		visit([](auto& v){
			if constexpr(requires{ v.increase_counter(); }) v.increase_counter();
		}, holder);
	}
	constexpr void deallocate() {
		visit([](auto& v){
			if constexpr(requires{ v.decrease_counter(); }) {
				if(v.decrease_counter()==0) v.deallocate();
			}
			}, holder);
	}
public:

	constexpr data() {}
	constexpr data(bool v) : holder(v) {}
	constexpr data(string_t v) : holder(v) {} //NOTE: bug in libstdc++ string cannot be moved here
	constexpr data(integer_t v) : holder(v) {}
	constexpr data(float_point_t v) : holder(v) {}
	constexpr data(self_type&& v) : holder(std::move(v.holder)) { v.holder=typename factory::empty_t{}; }
	constexpr data(const self_type& v) : holder(v.holder) { allocate(); }
	constexpr ~data() { deallocate(); }

	constexpr self_type& assign() { deallocate(); holder = typename factory::empty_t{}; return static_cast<self_type&>(*this);}
	constexpr self_type& assign(auto&& v) { deallocate(); holder = std::forward<decltype(v)>(v); allocate(); return static_cast<self_type&>(*this); }

	constexpr auto& operator=(string_t v){ return assign(std::move(v)); }
	constexpr auto& operator=(integer_t v){ return assign(v); }
	constexpr auto& operator=(float_point_t v){ return assign(v); }

	constexpr explicit operator bool() const { return get<bool>(holder); }
	constexpr operator string_t() const { return get<string_t>(holder); }
	constexpr operator integer_t() const { return get<integer_t>(holder); }
	constexpr operator float_point_t() const { return get<float_point_t>(holder); }

	constexpr bool is_int() const { return holder.index() == 2; }
	constexpr bool is_none() const { return holder.index() == 0; }
	constexpr bool is_bool() const { return holder.index() == 1; }
	constexpr bool is_string() const { return holder.index() == 4; }
	constexpr bool is_float_point() const { return holder.index() == 3; }
	constexpr bool is_array() const { return visit( [](const auto& v){ return requires(std::decay_t<decltype(v)>& vv){ vv.emplace_back( crtp{} ); }; }, holder); }

	constexpr auto size() const {
		return visit( [](const auto& v){
			if constexpr(requires{ v.size(); }) return v.size();
			else return sizeof(v); }, holder);
	}

	constexpr self_type& mk_empty_array() { return assign(array_t(factory{})); }
	constexpr self_type& push_back(self_type d) {
		return visit([this,d=std::move(d)](auto& v) -> self_type& { 
			if constexpr(requires{ v.emplace_back(std::move(d)); }) { return v.emplace_back(std::move(d)); }
			else {
				factory::throw_wrong_interface_error("push_back");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
	}
	constexpr self_type& operator[](integer_t ind){
		return visit([this,ind](auto& v)->self_type&{ 
			if constexpr(requires{ v.emplace_back(self_type{}); v.at(ind); }) { return v.at(ind); }
			else {
				factory::throw_wrong_interface_error("operator[]");
				std::unreachable();
				return static_cast<self_type&>(*this);
			}
		}, holder);
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
		static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); return (integer_t)d[0]; }() == 10 );
//		static_assert( []{ data_type d; return get<array_t>(d.holder).ref_counter; }() == 10 );
//		static_assert( []{ data_type d; d.mk_empty_array(); d.push_back(data_type{(integer_t)10}); auto dd = d; return (integer_t)dd[0]; }() == 10 );
		return true;
	}

	constexpr static bool test() {
		return test_simple_cases() && test_array_cases();
	}
};

} // namespace absd
