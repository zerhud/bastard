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
template <typename factory>
struct environment {
	using data_type = typename factory::data_type;
	static_assert(
		requires{ data_type{std::declval<factory>(), std::declval<environment*>()}; },
		"the environment should to be present as data type" );

	struct variable {
		data_type name;
		data_type value;
	};

	struct area_holder {
		environment& env;

		constexpr explicit area_holder(environment& e) : env(e) {
			env.stack.at(env.stack.size() - 1).emplace_back(mk_vec<variable>(env.f));
		}
		constexpr ~area_holder() {
			env.stack.at(env.stack.size() - 1).pop_back();
		}
	};

	struct frame_holder {
		environment* env;

		constexpr explicit frame_holder(environment& e) : env(&e) {
			env->stack
			   .emplace_back(mk_vec<frame_type>(env->f))
			   .emplace_back(mk_vec<variable>(env->f));
		}
		constexpr ~frame_holder() {
			if (env) env->stack.pop_back();
		}
	};

	using frame_type = decltype(mk_vec<variable>(std::declval<factory>()));
	using area_type = decltype(mk_vec<frame_type>(std::declval<factory>()));
	using stack_type = decltype(mk_vec<area_type>(std::declval<factory>()));

	constexpr environment() : environment(factory{}) { }
	constexpr explicit environment(factory f)
		: f(std::move(f))
		  , globals(mk_vec<variable>(this->f))
		  , stack(mk_vec<area_type>(this->f)) {
		frame_holder tmp{*this};
		tmp.env = nullptr;
	}

	constexpr const frame_type& cur_frame() const { return const_cast<environment&>(*this).cur_frame(); }
	constexpr frame_type& cur_frame() {
		auto& cur_area = stack.at(stack.size() - 1);
		return cur_area.at(cur_area.size() - 1);
	}

	constexpr void add_global(data_type name, data_type d) {
		for (auto& v : globals) if (v.name == name) {
			v.value = std::move(d);
			return;
		}
		globals.emplace_back(variable{std::move(name), std::move(d)});
	}
	constexpr void add_local(data_type name, data_type d) {
		for (auto& v : cur_frame()) if (v.name == name) {
			v.value = std::move(d);
			return;
		}
		cur_frame().emplace_back(variable{std::move(name), std::move(d)});
	}

	constexpr frame_holder push_frame() { return frame_holder{*this}; }
	constexpr area_holder push_area() { return area_holder{*this}; }

	constexpr data_type mk_context_data() {
		return mk_data_inner(this);
	}
	constexpr data_type mk_data_inner(auto&&... args) const {
		return mk_data(f, std::forward<decltype(args)>(args)...);
	}

	constexpr data_type at(data_type key) const {
		for (auto& v : const_cast<environment*>(this)->cur_frame()) if (v.name == key) return v.value;
		for (auto& v : globals) if (v.name == key) return v.value;
		return mk_data_inner();
	}
	constexpr bool contains(const typename data_type::string_t& key) const {
		for (auto& v : globals) if (v.name == key) return true;
		return false;
	}
	constexpr std::size_t size() const {
		auto ret = globals.size();
		for (auto& s : stack) for (auto& a : s) ret += a.size();
		return ret;
	}
	constexpr data_type keys(const auto& f) const {
		data_type ret{f};
		ret.mk_empty_array();
		for (auto& v : globals) ret.push_back(v.name);
		for (auto& v : cur_frame()) ret.push_back(v.name);
		return ret;
	}

	factory f;
	frame_type globals;
	stack_type stack;
};

template <typename factory>
struct context {
	using data_type = typename environment<factory>::data_type;

	struct out_holder {
		context* ctx;
		constexpr explicit out_holder(context& c) : ctx(&c) {
			ctx->out.emplace_back(output_frame(ctx->f, &ctx->env));
		}
		constexpr ~out_holder() {
			if (ctx) ctx->out.pop_back();
		}
	};
	struct out_info {
		struct _value_info {
			bool trim_before:4{false};
			bool trim:4{false};
			data_type value;
		};
		constexpr static auto mk_value_type() {
			return typename factory::template variant_t<shift_info, _value_info>{};
		}

		constexpr auto at(const data_type& key) {
			if (key=="value") return i.index() == 1 ? get<1>(i).value : data_type{};
			if (key=="trim_before") return i.index() == 1 ? data_type{get<1>(i).trim_before} : data_type{};
			if (key=="trim_after") return i.index() == 1 ? data_type{get<1>(i).trim} : data_type{};
			if (key=="shift") return i.index() == 0 ? data_type{(typename data_type::integer_t)get<0>(i).shift} : data_type{};
			if (key=="absolute") return i.index() == 0 ? data_type{get<0>(i).absolute} : data_type{};
			return data_type{};
		}
		constexpr auto size() const { return 5; }
		constexpr auto keys(const auto& f) const {
			auto ret = mk_vec<data_type>(f);
			ret.emplace_back(data_type{"value"});
			ret.emplace_back(data_type{"trim_before"});
			ret.emplace_back(data_type{"trim_after"});
			ret.emplace_back(data_type{"shift"});
			ret.emplace_back(data_type{"absolute"});
			return ret;
		}

		constexpr explicit out_info(shift_info i) : i(std::move(i)) {}
		constexpr explicit out_info(data_type v) : i(_value_info{.value=std::move(v)}) {}
		constexpr explicit out_info(bool b, bool t, data_type v) : i(_value_info{b, t, std::move(v)}) { }

		constexpr auto value() const {
			return get<1>(i);
		}
		constexpr auto* shift() const {
			if (i.index()!=0) return (const shift_info*)nullptr;
			return &get<0>(i);
		}

		decltype(mk_value_type()) i;
	};

	struct output_frame {
		using records_type = decltype(mk_vec<out_info>(std::declval<factory>()));
		records_type records;
		const environment<factory>* env;
		mutable int shift = 0;

		constexpr explicit output_frame(const factory& f, const environment<factory>* env) : records(mk_vec<out_info>(f)), env(env) {}

		constexpr data_type at(records_type::size_type ind) { return data_type::mk(records.at(ind)); }
		constexpr const data_type at(records_type::size_type ind) const { return data_type::mk(records.at(ind)); }
		constexpr auto size() const { return records.size(); }
		constexpr bool contains(const data_type& key)  const {
			return false;
		}

		constexpr data_type stringify() const {
			shift = 0;
			auto ret = env->mk_data_inner("");
			for (auto& item:records) modify(ret, [&](auto&, typename data_type::string_t& v) {
				if (auto* shift = item.shift())
					this->shift = shift->shift + !shift->absolute * this->shift;
				else trim_value(item, v);
			});
			return ret;
		}

		constexpr data_type operator()() const {
			return stringify();
		}
	private:
		constexpr void trim_value(auto& item, typename data_type::string_t& v) const {
			auto [before,after,value] = item.value();
			if (before) trim_right(env->f, v);
			if (value.is_string()) v+= shift_value((typename data_type::string_t)value);
			else v += shift_value(jinja_to_string(env->f, value));
			if (after) trim_right(env->f, v);
		}
		constexpr auto shift_value(typename data_type::string_t&& v) const {
			if (shift<1) return v;
			//TODO: move algo to factory?
			auto rep = mk_str(env->f);
			rep.resize(shift+1, '\t');
			rep[0] = '\n';
			for (auto pos = v.find('\n', 0);pos!=v.npos;pos = v.find('\n', pos+rep.size())) {
				v.replace(pos, 1, rep);
			}
			return v;
		}
	};

	constexpr static auto mk_output(const factory& f) { return mk_vec<out_info>(f); }
	using output_type = decltype(mk_output(std::declval<factory>()));
	constexpr static auto mk_out(const factory& f) { return mk_vec<output_type>(f); }

	constexpr explicit context(factory f) : f(std::move(f)), out(mk_vec<output_frame>(this->f)) {
		auto h = catch_output();
		h.ctx = nullptr;
	}

	[[nodiscard]] constexpr auto catch_output() { return out_holder(*this); }
	constexpr const auto& cur_output() const { return out.back(); }
	constexpr auto& cur_output() { return out.back(); }

	constexpr auto extract_output() {
		auto ret = std::move(out.back());
		return ret;
	}
	constexpr auto extract_output_to_data() {
		return env.mk_data_inner(extract_output());
	}

	constexpr context& operator()(data_type content) {
		out.back().records.emplace_back(out_info(std::move(content)));
		return *this;
	}
	constexpr context& operator()(trim_info<factory> before, data_type content, trim_info<factory> after) {
		out.back().records.emplace_back(out_info(before.trim, after.trim, std::move(content)));
		return *this;
	}
	constexpr context& operator()(shift_info si) {
		out.back().records.emplace_back(si);
		return *this;
	}

	constexpr data_type mk_data(auto&&... args) const {
		return env.mk_data_inner(std::forward<decltype(args)>(args)...);
	}

	factory f;
	environment<factory> env;
private:
	decltype(mk_vec<output_frame>(std::declval<factory>())) out;
};
} // namespace jinja_details
