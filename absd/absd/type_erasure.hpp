#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

namespace absd::details {

template<typename type, template<typename...>class tmpl> constexpr const bool is_specialization_of = false;
template<template<typename...>class type, typename... args> constexpr const bool is_specialization_of<type<args...>, type> = true;

struct inner_counter {
	unsigned long int ref_counter = 0;
	constexpr auto increase_counter() { return ++ref_counter; }
	constexpr auto decrease_counter() { return --ref_counter; }
};

struct counter_interface {
	constexpr virtual ~counter_interface() noexcept {}
	constexpr virtual decltype(inner_counter::ref_counter) increase_counter() =0 ;
	constexpr virtual decltype(inner_counter::ref_counter) decrease_counter() =0 ;
};

struct multiobject_tag : counter_interface {
	constexpr virtual bool is_arr() const { return false; }
	constexpr virtual bool is_obj() const { return false; }
	constexpr virtual bool is_cll() const { return false; }
	constexpr virtual bool is_eq(const multiobject_tag& other) const {
		return this == &other;
	}
	constexpr virtual const char* dynamic_cast_workaround() const {
		return "";
	}
};

template<typename type>
struct counter_maker : inner_counter, type {
	constexpr counter_maker(auto&&... args) : type(std::forward<decltype(args)>(args)...) {}
	constexpr decltype(inner_counter::ref_counter) increase_counter() override { return inner_counter::increase_counter(); }
	constexpr decltype(inner_counter::ref_counter) decrease_counter() override { return inner_counter::decrease_counter(); }
};
template<typename data_type, typename from, typename type>
constexpr static data_type mk_coutner_and_assign(const auto& f, auto&& v) {
	data_type ret;
	auto tmp = f.mk_ptr(counter_maker<type>(std::forward<decltype(v)>(v)));
	ret.assign(static_cast<from*>(tmp.get()));
	tmp.release();
	return ret;
}

template<typename val_type>
struct origin : details::multiobject_tag {
	val_type val;
	constexpr origin(val_type val) : val(std::move(val)) {}

	constexpr auto& orig_val() {
		if constexpr (is_specialization_of<val_type, details::callable2>) return val.fnc;
		else return val;
	}
	constexpr const auto& orig_val() const {
		//TODO: use deducing this when gcc14
		return const_cast<origin&>(*this).orig_val();
	}

	constexpr auto& call_val() { return val; }

	constexpr const char* dynamic_cast_workaround() const override {
		// same strings and same pointers for same types,
		// so we can compare only pointers instead of whole string
		return __PRETTY_FUNCTION__ ;
	}
	constexpr bool is_eq(const multiobject_tag& other) const override {
		if(static_cast<const multiobject_tag*>(this) == &other) return true;
		if constexpr(requires{this->orig_val()==this->orig_val();}) {
#if (defined(__GNUC__) && !defined(__clang__))
			if(other.dynamic_cast_workaround() == dynamic_cast_workaround())
				return this->orig_val() == static_cast<const origin&>(other).orig_val();
#else
			if(auto* o=dynamic_cast<const origin*>(&other);o) return this->orig_val() == o->orig_val();
#endif
			return false;
		}
		else if constexpr(requires{this->orig_val().is_eq(std::declval<val_type>());}) {
#if (defined(__GNUC__) && !defined(__clang__))
			if(other.dynamic_cast_workaround() == dynamic_cast_workaround())
				return this->orig_val().is_eq(static_cast<const origin&>(other).orig_val());
#else
			if(auto* o=dynamic_cast<const origin*>(&other);o) return this->orig_val().is_eq(o->orig_val());
#endif
			return false;
		}
		else return multiobject_tag::is_eq(other);
	}
};

} // namespace absd::details
