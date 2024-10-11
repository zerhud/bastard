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
	using params_t = decltype(mk_vec<parameter_descriptor>(std::declval<factory_t>()));

	constexpr virtual ~type_erasure_callable() noexcept =default ;
	constexpr virtual data call(data params) =0 ;
	//TODO: params never used
	constexpr virtual params_t params(const factory_t& f) const {return mk_vec<parameter_descriptor>(f);}
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

} // namespace absd::details
