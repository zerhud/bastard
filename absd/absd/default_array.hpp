#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>
#include "type_erasure.hpp"
#include "exceptions.hpp"

namespace absd::details {

template<typename factory, typename data>
struct array : inner_counter {
	using vec_content = data;

	std::decay_t<decltype(std::declval<factory>().template mk_vec<vec_content>())> holder;


	constexpr array(factory f) requires (requires{ f.template mk_vec<vec_content>(); }) : holder(f.template mk_vec<vec_content>()) {}

	constexpr data& emplace_back(data d) { return holder.emplace_back(std::move(d)); }
	constexpr data& at(std::integral auto ind) { return holder.at(ind); }
	constexpr auto size() const { return holder.size(); }
};
template<typename data, typename factory> constexpr auto mk_array_type(const factory& f) {
	if constexpr(!requires{ typename factory::array_t; }) return array<factory, data>(f);
	else return typename factory::array_t{};
}

template<typename data>
struct type_erasure_array {
	constexpr virtual ~type_erasure_array() noexcept =default ;

	constexpr virtual data& emplace_back(data d) =0 ;
	constexpr virtual data& at(typename data::integer_t ind) =0 ;
	constexpr virtual decltype(sizeof(data)) size() const =0 ;
	constexpr virtual bool contains(const data& val) const =0 ;
};
template<typename data_type>
constexpr auto mk_te_array(const auto& f, auto&& src) {
	using int_t = data_type::integer_t;
	using src_type = std::decay_t<decltype(src)>;
	using te_array = type_erasure_array<data_type>;
	constexpr const bool is_array = requires{ src.orig_val().at(int_t{}); };
	if constexpr (!is_array) return std::move(src);
	else {
		struct te_ar2 : src_type, te_array {
			constexpr te_ar2(src_type v) : src_type(std::move(v)) {}
			constexpr ~te_ar2() noexcept override {}

			constexpr bool is_arr() const override { return true; }
			constexpr data_type& emplace_back(data_type d) override {
				if constexpr (requires{this->orig_val().emplace_back(std::move(d));})
					return this->orig_val().emplace_back(std::move(d));
				else data_type::factory_t::template throw_wrong_interface_error<interfaces::push_back>();
			}
			constexpr data_type& at(typename data_type::integer_t ind) override { return this->orig_val().at(ind); }
			constexpr decltype(sizeof(data_type)) size() const override { return this->orig_val().size(); }
			constexpr bool contains(const data_type& val) const override {
				if constexpr (requires{this->orig_val().contains(val);})
					return this->orig_val().contains(val);
				else {
					for(auto& i:this->orig_val()) if(i==val) return true;
					return false;
				}
			}
		};
		return te_ar2{std::move(src)};
	}
}

} // namespace absd::details
