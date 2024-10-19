#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "tests/factory.hpp"

#include "absd.hpp"
#include "jiexpr.hpp"
#include "jiexpr/default_operators.hpp"
#include "ascip.hpp"

struct factory : tests::factory {
	using data_type = absd::data<factory>;
};

using parser = ascip<std::tuple>;
using absd_data = absd::data<factory>;
using jiexpr_test = jiexpr<absd_data, jiexpr_details::expr_operators_simple, factory>;

constexpr absd_data eval(std::string_view src, absd_data& env) {
	jiexpr_test::operators_executer ops;
	jiexpr_test ev{&env, ops};
	auto parsed = ev.parse_str<parser>(src);
	return ev(parsed);
}

constexpr absd_data eval(std::string_view src) {
	absd_data env;
	env.mk_empty_object();
	return eval(src, env);
}

constexpr auto eval2(std::string_view src, absd_data& env) {
	jiexpr_test::operators_executer ops;
	jiexpr_test ev{&env, ops};
	auto parsed = ev.parse_str2<parser>(src);
	jiexpr_test::solve_info info{};
	info.env = &env;
	return parsed.solve(info);
}

constexpr auto eval2(std::string_view src) {
	absd_data env;
	env.mk_empty_object();
	return eval2(src, env);
}