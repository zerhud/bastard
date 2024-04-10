#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "type_erasure.hpp"
#include "callable.hpp"

namespace absd::details {

template<typename factory, typename data>
struct type_erasure_callable {
	using factory_t = factory;
	using string_t = typename data::string_t;
	struct parameter_descriptor {
		string_t name;
		data default_value;
	};
	using params_t = decltype(std::declval<factory_t>().template mk_vec<parameter_descriptor>());

	constexpr virtual ~type_erasure_callable() noexcept =default ;
	constexpr virtual data call(data params) =0 ;
	//TODO: params never used
	constexpr virtual params_t params(const factory_t& f) const {return {};}
};
template<bool is_callable, typename data_type, typename factory>
constexpr auto mk_te_callable(const factory& f, auto&& src, auto&&... args) {
	using src_type = std::decay_t<decltype(src)>;
	using te_callable = type_erasure_callable<factory, data_type>;
	if constexpr (!is_callable) return std::move(src);
	else {
		struct te : src_type, te_callable {
			constexpr te(src_type v) : src_type(std::move(v)) {}
			constexpr bool is_cll() const override { return true; }
			constexpr data_type call(data_type params) override {
				return data_type{this->call_val().call(std::move(params))};
			}
		};
		return te{std::move(src)};
	}
}

template<typename data_type>
constexpr bool test_callable() {
	using int_t = typename data_type::integer_t;
	static_assert( !data_type::mk([]{}).is_array() );
	static_assert( !data_type::mk([]{}).is_object() );
	static_assert( data_type::mk([]{}).is_callable() );
	static_assert( data_type::mk([]{}).call(data_type::mk_map()).is_none() );
	static_assert( (int_t)data_type::mk([]{return 1;}).call(data_type::mk_map()) == 1);
	static_assert( (int_t)data_type::mk([](int a){return a-1;}, data_type::mk_param("a")).call(data_type::mk_map(data_type{0}, data_type{3})) == 2 );

	static_assert( (int_t)data_type::mk([](int a, int b){return a-b;}, data_type::mk_param("a"), data_type::mk_param("b", data_type{1})).call(data_type::mk_map(0, 3)) == 2);

	constexpr auto amb = []{return data_type::mk([](int a, int b){return a-b;}, data_type::mk_param("a", data_type{3}), data_type::mk_param("b")); };
	static_assert( (int_t)amb().call(data_type::mk_map("b", 3)) == 0);
	static_assert( (int_t)amb().call(data_type::mk_map(0, 7, "b", 3)) == 4);
	static_assert( (int_t)amb().call(data_type::mk_map("b", 3, 0, 7)) == 4);

	struct callable_obj_arr : constexpr_kinda_map<typename data_type::factory_t, data_type, data_type> {
		mutable data_type fake_data;
		mutable data_type data_7{7}, data_3{3}, data_5{5};
		constexpr data_type operator()() const {return data_type{88};}
		constexpr data_type operator()(int_t a, int_t b) const {return data_type{a-b};}
		constexpr data_type& at(int_t ind) const { if(ind==0) return data_3; if(ind==1) return data_5; return fake_data; }
		constexpr data_type& emplace_back(data_type d) { return fake_data; }
		//constexpr int_t size() const { return 2; }
		//constexpr bool contains(const data_type&) const { return false; }
	};

	static_assert( data_type::mk(callable_obj_arr{}).is_array() );
	static_assert( data_type::mk(callable_obj_arr{}).is_object() );
	static_assert( data_type::mk(callable_obj_arr{}, data_type::mk_param("a"), data_type::mk_param("b")).is_callable() );

	return true;
}

} // namespace absd::details
