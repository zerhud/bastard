#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>
#include "trim_info.hpp"

namespace jinja_details {

template<typename factory>
struct environment {
    using data_type = decltype(mk_data_type<environment>(std::declval<factory>()));

	struct variable {
		data_type name;
		data_type value;
	};

	struct area_holder {
		environment& env;
		constexpr explicit area_holder(environment& e) : env(e) {
			env.stack.at(env.stack.size()-1).emplace_back(mk_vec<variable>(env.f));
		}
		constexpr ~area_holder() {
			env.stack.at(env.stack.size()-1).pop_back();
		}
	};
	struct frame_holder {
		environment* env;
		constexpr explicit frame_holder(environment& e) : env(&e) {
			env->stack
			.emplace_back(mk_vec<frame_type>(env->f))
			.emplace_back(mk_vec<variable>(env->f))
			;
		}
		constexpr ~frame_holder() {
			if (env) env->stack.pop_back();
		}
	};

	using frame_type = decltype(mk_vec<variable>(std::declval<factory>()));
	using area_type = decltype(mk_vec<frame_type>(std::declval<factory>()));
	using stack_type = decltype(mk_vec<area_type>(std::declval<factory>()));

    constexpr environment() : environment(factory{}) {}
    constexpr explicit environment(factory f)
	: f(std::move(f))
	, globals(mk_vec<variable>(this->f))
	, stack(mk_vec<area_type>(this->f)) {
	    frame_holder tmp{*this};
    	tmp.env = nullptr;
    }

	constexpr frame_type& cur_frame() {
    	auto& cur_area = stack.at(stack.size()-1);
	    return cur_area.at(cur_area.size()-1);
    }
    constexpr void add_global(data_type name, data_type d) {
    	for (auto& v:globals) if (v.name == name) {
    		v.value = std::move(d);
    		return;
    	}
	    globals.emplace_back(variable{std::move(name), std::move(d)});
    }
    constexpr void add_local(data_type name, data_type d) {
    	for (auto& v:cur_frame()) if (v.name == name) {
    		v.value = std::move(d);
    		return;
    	}
    	cur_frame().emplace_back( variable{std::move(name), std::move(d)} );
    }

    constexpr frame_holder push_frame() { return frame_holder{*this}; }
    constexpr area_holder push_area() { return area_holder{*this}; }

	constexpr data_type mk_context_data() {
	    return mk_data(this);
    }
    constexpr data_type mk_data(auto&&... args) const {
        return mk_data_type<environment>(f, std::forward<decltype(args)>(args)...);
    }

	constexpr data_type at(data_type key) {
    	for (auto& v:const_cast<environment*>(this)->cur_frame()) if (v.name == key) return v.value;
    	for (auto& v:globals) if (v.name == key) return v.value;
	    return mk_data();
    }
	constexpr bool contains(const typename data_type::string_t& key) const {
    	for (auto& v:globals) if (v.name == key) return true;
		return false;
	}
	constexpr std::size_t size() const {
    	auto ret =  globals.size();
    	for (auto& s:stack) for (auto& a:s) ret += a.size();
	    return ret;
    }
	constexpr data_type keys(const auto& f) const {
    	data_type ret{f};
    	ret.mk_empty_array();
    	for (auto& v:globals) ret.push_back(v.name);
    	return ret;
    }

private:
    factory f;
	frame_type globals;
	stack_type stack;
};

template<typename factory>
struct context {
	using data_type = typename environment<factory>::data_type;

	struct out_info {
		trim_info<factory> before;
		trim_info<factory> after;
		data_type value;
	};

	constexpr static auto mk_output(const factory& f) {
		return mk_vec<out_info>(f);
	}

	using output_type = decltype(mk_output(std::declval<factory>()));
	constexpr explicit context(factory f) : f(std::move(f)), out(mk_output(this->f)) {}

	constexpr context& operator()(data_type content) {
		out.emplace_back(out_info{.value=std::move(content)});
		return *this;
	}
	constexpr context& operator()(trim_info<factory> before, data_type content, trim_info<factory> after) {
		out.emplace_back(out_info{std::move(before), std::move(after), std::move(content)});
		return *this;
	}

	factory f;
	environment<factory> env;
	output_type out;
};

} // namespace jinja_details
