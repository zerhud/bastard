#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>
#include <concepts>
#include <cassert>
#include <tref.hpp>

#include "jiexpr/details.hpp"
#include "jiexpr/virtual_variant.hpp"

//NOTE: there is a clang bug: https://github.com/llvm/llvm-project/issues/59966
//      workaround:
//      1. the solve methods have to be defined after first instantiation
//      2. call base class methods via a base class pointer,
//         but after it clang can't compile and doesn't produce an understandable message
template< typename data_factory >
struct jiexpr {
	using data_type = data_factory::data_type;
	using operators_factory = decltype(jiexpr_details::make_default_operators(std::declval<data_factory>()));
	template<typename... types> using variant_t = typename data_factory::template variant_t<types...>;
	using operators_executer = operators_factory;
	using env_tuner = decltype(jiexpr_details::make_env_tuner<data_factory>())::type;

	struct solve_info {
		data_type* env;
		operators_factory ops;
		data_factory df;
		env_tuner tune_env;
	};

	using op_holder_base = jiexpr_details::expression_base<data_factory, solve_info>;
	template<typename type> using op_holder_wrapper = jiexpr_details::expression_item_wrapper<data_factory, solve_info, type>;

	template<typename expr_t>
	struct binary_op : op_holder_base {
		std::decay_t<expr_t> left;
		std::decay_t<expr_t> right;
		constexpr static auto struct_fields_count() { return 2; }
		constexpr binary_op() =default ;
		constexpr explicit binary_op(data_factory) {}
	};
	template<typename expr_t>
	struct ternary_op : op_holder_base {
		std::decay_t<expr_t> cond;
		std::decay_t<expr_t> left;
		std::decay_t<expr_t> right;
		constexpr static auto struct_fields_count() { return 3; }
		constexpr ternary_op() =default ;
		constexpr explicit ternary_op(data_factory) {}
		constexpr data_type solve(const solve_info& i) const override {
			if(to_bool<data_type>(i.ops, cond->solve(i))) return left->solve(i);
			if(right) return right->solve(i);
			return data_type{i.df};
		}
	};
	template<typename expr_t> struct op_division : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return int_div<data_type>(i.ops, this->left->solve(i), this->right->solve(i));
		}
	};
	template<typename expr_t> struct op_multiply : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return multiply<data_type>(i.ops, this->left->solve(i), this->right->solve(i));
		}
	};
	template<typename expr_t> struct op_fp_div   : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return fp_div<data_type>(i.ops, this->left->solve(i), this->right->solve(i));
		}
	};
	template<typename expr_t> struct op_subtract : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return sub<data_type>( i.ops, this->left->solve(i), this->right->solve(i) );
		}
	};
	template<typename expr_t> struct op_addition : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return add<data_type>( i.ops, this->left->solve(i), this->right->solve(i) );
		}
	};
	template<typename expr_t> struct op_concat   : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			auto left_str = mk_str(i.df);
			auto right_str = mk_str(i.df);
			back_insert_format(back_inserter(i.df, left_str), this->left->solve(i));
			back_insert_format(back_inserter(i.df, right_str), this->right->solve(i));
			return data_type{ i.df, do_concat(i.ops, std::move(left_str), std::move(right_str)) };
		}
	};
	template<typename expr_t> struct op_power    : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return pow<data_type>(i.ops, this->left->solve(i), this->right->solve(i));
		}
	};

	template<typename expr_t> struct op_ceq      : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return do_ceq<data_type>(i.ops, this->left->solve(i), this->right->solve(i));
		}
	};
	template<typename expr_t> struct op_neq      : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return do_neq<data_type>(i.ops, this->left->solve(i), this->right->solve(i) );
		}
	};
	template<typename expr_t> struct op_gt       : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return do_gt<data_type>(i.ops, this->left->solve(i), this->right->solve(i));
		}
	};
	template<typename expr_t> struct op_lt       : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return do_lt<data_type>(i.ops, this->left->solve(i), this->right->solve(i));
		}
	};
	template<typename expr_t> struct op_get      : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return do_get<data_type>(i.ops, this->left->solve(i), this->right->solve(i));
		}
	};
	template<typename expr_t> struct op_let      : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return do_let<data_type>(i.ops, this->left->solve(i), this->right->solve(i) );
		}
	};
	template<typename expr_t> struct op_in       : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return data_type{ i.df, do_in<data_type>(i.ops, this->left->solve(i), this->right->solve(i)) };
		}
	};

	template<typename expr_t> struct op_and      : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return do_and<data_type>( i.ops, this->left->solve(i), this->right->solve(i) );
		}
	};
	template<typename expr_t> struct op_or       : binary_op<expr_t> {
		using binary_op<expr_t>::binary_op;
		constexpr data_type solve(const solve_info& i) const override {
			return do_or<data_type>( i.ops, this->left->solve(i), this->right->solve(i) );
		}
	};

	template<typename expr_t> struct op_not : op_holder_base {
		std::decay_t<expr_t> expr;
		constexpr static auto struct_fields_count() { return 1; }
		constexpr op_not() =default ;
		constexpr explicit op_not(data_factory) {}
		constexpr data_type solve(const solve_info& i) const override {
			return negate<data_type>( i.ops, expr->solve(i) );
		}
	};

	template<typename expr_t>
	struct list_expr : op_holder_base {
		decltype(mk_vec<expr_t>(std::declval<data_factory>())) list;
		constexpr static auto struct_fields_count() { return 1; }
		constexpr list_expr() =default ;
		constexpr explicit list_expr(data_factory) {}
		constexpr data_type solve(const solve_info& i) const override {
			data_type ret{i.df};
			ret.mk_empty_array();
			for(auto&& item:list) ret.push_back(item->solve(i));
			return ret;
		}
	};
	template<typename expr_t>
	struct dict_expr : op_holder_base {
		decltype(mk_vec<expr_t>(std::declval<data_factory>())) names;
		decltype(mk_vec<expr_t>(std::declval<data_factory>())) values;
		constexpr static auto struct_fields_count() { return 2; }
		constexpr dict_expr() =default ;
		constexpr explicit dict_expr(data_factory) {}
		constexpr data_type solve(const solve_info& info) const override {
			data_type ret{info.df};
			ret.mk_empty_object();
			for(auto i=0;i<names.size();++i)
				ret.put(names[i]->solve(info), values.at(i)->solve(info));
			return ret;
		}
	};


	using string_t = typename data_type::string_t;
	using integer_t = typename data_type::integer_t;
	using float_point_t = typename data_type::float_point_t;

	template<typename expr_t>
	struct var_expr : op_holder_base {
		decltype(mk_vec<expr_t>(std::declval<data_factory>())) path;
		constexpr static auto struct_fields_count() { return 1; }
		constexpr var_expr() =default ;
		constexpr explicit var_expr(data_factory) {}
		constexpr data_type solve(const solve_info& info) const override {
			auto cur = (*info.env)[path.at(0)->solve(info)];
			for(auto pos = ++path.begin();pos!=path.end();++pos) {
				auto key = (*pos)->solve(info);
				if(key.is_int()) cur = cur[(integer_t)key];
				else cur = cur[key];
			}
			return cur;
		}
	};

	struct op_eq_tag{};
	template<typename expr_t> struct op_eq : op_eq_tag, op_holder_base {
		constexpr op_eq() =default ;
		constexpr explicit op_eq(data_factory) {}
		constexpr op_eq(var_expr<expr_t> name, expr_t value) : op_eq(data_factory{}, std::move(name), std::move(value)) {}
		constexpr op_eq(data_factory, var_expr<expr_t> name, expr_t value) : name(std::move(name)), value(std::move(value)) {}

		var_expr<expr_t> name;
		expr_t value;

		constexpr static auto struct_fields_count() { return 2; }
		constexpr bool is_eq_operator() const override { return true; }
		constexpr data_type solve(const solve_info& info) const override {
			auto cur = *info.env;
			auto& left = name.path;
			for(auto i=0;i<left.size()-1;++i) cur = cur[left[i]->solve(info)];
			data_type key = left[left.size()-1]->solve(info);
			cur.put(key, value->solve(info));
			return cur[key];
		}
	};

	template<typename expr_t>
	struct common_fnc_expr {
		var_expr<expr_t> name;
		decltype(mk_vec<expr_t>(std::declval<data_factory>())) params;

		constexpr common_fnc_expr() =default ;
		constexpr explicit common_fnc_expr(data_factory) {}
		constexpr static auto struct_fields_count() { return 2; }

		constexpr auto prepare_call(const solve_info& info) const {
			auto fnc = name.solve(info);
			struct {
				data_type fnc;
				data_type env;
				solve_info info;
			} ret{ fnc, info.tune_env(*info.env, fnc), info };
			ret.info.env = &ret.env;
			return ret;
		}
		constexpr auto exec(auto&& ctx, typename data_type::integer_t ind) const {
			ctx.info.env = &ctx.env;
			for(auto& param:this->params) {
				auto solved = param->solve(ctx.info);
				if( !param->is_eq_operator() )
					ctx.env.put(data_type{ind++}, std::move(solved));
			}
			return ctx.fnc.call(ctx.env);
		}
	};

	template<typename expr_t>
	struct fnc_call_expr : common_fnc_expr<expr_t>, op_holder_base {
		constexpr fnc_call_expr() =default ;
		constexpr explicit fnc_call_expr(data_factory f) : common_fnc_expr<expr_t>(std::move(f)) {}
		constexpr data_type solve(const solve_info& info) const override {
			return this->exec(this->prepare_call(info), 0);
		}
	};

	template<typename expr_t>
	struct apply_filter_expr : op_holder_base {
		std::decay_t<expr_t> object;
		common_fnc_expr<expr_t> filter;

		constexpr apply_filter_expr() =default ;
		constexpr explicit apply_filter_expr(data_factory) {}
		constexpr static auto struct_fields_count() { return 2; }
		constexpr data_type solve(const solve_info& info) const override {
			auto first_param = object->solve(info);
			auto prepared = filter.prepare_call(info);
			prepared.env.put(data_type{0}, first_param);
			return filter.exec(prepared, 1);
		}
	};

	template<typename expr_t>
	struct is_test_expr : op_holder_base {
		std::decay_t<expr_t> object;
		common_fnc_expr<expr_t> test;

		constexpr is_test_expr() =default ;
		constexpr explicit is_test_expr(data_factory) {}
		constexpr static auto struct_fields_count() { return 2; }
		constexpr data_type solve(const solve_info& info) const override {
			auto first_param = object->solve(info);
			auto prepared = test.prepare_call(info);
			prepared.env.put(data_type{0}, first_param);
			return to_bool<data_type>(info.ops, test.exec(prepared, 1));
		}
	};

	template<typename type> using ast_forwarder = typename data_factory::template ast_forwarder<type>;


	template<typename... types>
	using op_holder = jiexpr_details::expression_variant<typename data_factory::template virtual_variant_t< op_holder_base, op_holder_wrapper, types... >>;
	template<template<class>class fa> struct expression_type : op_holder<
			  ternary_op< fa<expression_type<fa>> >
			, apply_filter_expr< fa<expression_type<fa>> >
			, op_holder< op_and< fa<expression_type<fa>> >, op_or< fa<expression_type<fa>> > >
			, op_holder<
			        op_ceq<fa<expression_type<fa>>>, op_neq<fa<expression_type<fa>>>, op_lt<fa<expression_type<fa>>>
			      , op_gt<fa<expression_type<fa>>>, op_get<fa<expression_type<fa>>>, op_let<fa<expression_type<fa>>>
			      , op_in<fa<expression_type<fa>>>, is_test_expr<fa<expression_type<fa>>>
			  >
			, op_holder< op_subtract < fa<expression_type<fa>> >, op_addition< fa<expression_type<fa>> >, op_concat< fa<expression_type<fa>> > >
			, op_holder< op_multiply < fa<expression_type<fa>> >, op_division< fa<expression_type<fa>> >, op_fp_div< fa<expression_type<fa>> > >
			, op_power    < fa<expression_type<fa>> >
			, op_not      < fa<expression_type<fa>> >
			, list_expr   < fa<expression_type<fa>> >
			, dict_expr   < fa<expression_type<fa>> >
			, var_expr    < fa<expression_type<fa>> >
			, fnc_call_expr < fa<expression_type<fa>> >
			, op_eq       < fa<expression_type<fa>> >
			, string_t,integer_t,float_point_t,bool
	> {};

	using parsed_expression = expression_type<ast_forwarder>;

	data_factory df;

	template<typename gh>
	constexpr auto create_parser() const ;
	template<typename gh, typename result_t, typename expr_t, template<auto>class th=gh::template tmpl>
	constexpr auto _create_parser() const ;

	template<typename gh>
	constexpr friend auto create_parser(const jiexpr& e) { return e.create_parser<gh>(); }

	/*
	 * - . and [] operators after literal
	 */
	template<typename gh>
	constexpr auto parse_str(auto&& src) const {
		using result_t = expression_type<ast_forwarder>;
		using expr_t = ast_forwarder<expression_type<ast_forwarder>>;
		expression_type<ast_forwarder> r;
		parse(_create_parser<gh, result_t, expr_t>(), +gh::space, gh::make_source(src), r);
		return r;
	}
};

#include "jiexpr/parser.ipp"
