/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of bastard.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

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
		using ret_type = decltype(call_with_combined_params<0>(params));
		if constexpr(!std::is_same_v<ret_type, void>) return call_with_combined_params<0>(params);
		else {
			call_with_combined_params<0>(params);
			return self_type{};
		}
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

constexpr static self_type mk(const factory& f, auto&& v) requires( details::is_specialization_of<std::decay_t<decltype(v)>, callable> ){
	using val_type = std::decay_t<decltype(v)>;
	constexpr const bool is_object = requires{ v.at(self_type{}); };

	struct te_base {
		using params_t = typename te_callable::params_t;
		val_type v;
		constexpr explicit te_base(val_type&& v) : v(std::forward<decltype(v)>(v)) {}
		constexpr self_type call(self_type params) {
			return self_type{v.call(params)};
		}
		constexpr params_t params(const factory& f) const {return params_t{};} // TODO: implement parameters search
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
/*
	assert( [&wd_lpr]{
		self_type params; params.put(self_type{0}, self_type{4}); params.put(self_type{"l"}, self_type{40});
		auto d = self_type::mk(std::move(wd_lpr));
		std::cout << "here " << d.is_callable() << ' ' << std::flush << (integer_t)(d.call(params)) << std::endl;
		return (integer_t)(d.call(params));
	}() == 6 );
*/
	return true;
}
