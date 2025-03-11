/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

//#include <utility>
//#include "ascip_all_gcc.hpp"
#include <ascip.hpp>

#include <absd.hpp>
#include <absd/iostream_op.hpp>

#include "jiexpr.hpp"
#include "jinja.hpp"

#include "virtual_variant.hpp"

#include <memory>
#include <vector>
#include <variant>


template<auto base = 10>
constexpr auto symbols_count(auto v) {
	unsigned int len = v >= 0 ? 1 : 2;
	for (auto n = v < 0 ? -v : v; n; ++len, n /= base);
	return ((len-1)*(v!=0)) + (len*(v==0));
}

template<auto base = 10>
constexpr std::string test_to_string(auto v) {
	std::string ret;
	ret.resize(symbols_count<base>(v));
	std::to_chars(ret.data(), ret.data() + ret.size(), v);
	return ret;
}

struct std_factory;

template<typename... types> struct type_list {};

struct std_factory {
	using empty_t = std::monostate;
	using string_t = std::string;
	using integer_t = int;
	using float_point_t = double;

	using extra_types = type_list<jinja_details::environment<std_factory>>;

	using parser = ascip;
	template<typename type> using ast_forwarder = std::unique_ptr<type>;
	template<typename... types> using variant = std::variant<types...>;
	template<typename... types> using variant_t = std::variant<types...>;
	template<
			typename base_type,
			template<typename>class wrapper, typename... types
	> using virtual_variant_t = virtual_variant<variant_t, base_type, wrapper, types...>;

	using data_type = absd::data<std_factory>;

	constexpr static void deallocate(auto* ptr) noexcept { delete ptr; }
};

template<typename type> constexpr auto mk_vec(const std_factory&) { return std::vector<type>{}; }
template<typename type> constexpr auto mk_empty_ptr(const std_factory&) { return std::unique_ptr<type>{}; }
template<typename type> constexpr auto mk_ptr(const std_factory&, auto&&... args) { return std::make_unique<type>(std::forward<decltype(args)>(args)...); }
constexpr auto mk_ptr_from_obj(const std_factory&, auto d) { return std::make_unique<decltype(d)>( std::move(d) ); }
constexpr auto mk_str(const std_factory&) { return std::string{}; }
constexpr auto mk_data(const std_factory& f) { return std_factory::data_type{f}; }
constexpr auto mk_data(const std_factory&, std::string_view src) {
	using dt = std_factory::data_type;
	return dt{ dt::string_t{src} };
}
constexpr auto mk_data(const std_factory& f, auto&& src) { return std_factory::data_type::mk(f, std::forward<decltype(src)>(src)); }
constexpr auto mk_fwd(const std_factory&, auto& v) {
	using v_type = std::decay_t<decltype(v)>;
	static_assert( !std::is_pointer_v<v_type>, "the result have to be a unique_ptr like type" );
	static_assert( !std::is_reference_v<v_type>, "the result have to be a unique_ptr like type" );
	v = std::make_unique<typename v_type::element_type>();
	return v.get();
}
constexpr auto mk_result(const std_factory&, auto&& v) {
	using expr_t = std::decay_t<decltype(v)>;
	return std::make_unique<expr_t>(std::forward<decltype(v)>(v));
}

struct jiexpr_abstracter : std_factory::parser::base_parser<jiexpr_abstracter> {
	decltype(jiexpr<std_factory>{}.create_parser<std_factory::parser>()) p;
	constexpr auto parse(auto&& ctx, auto src, auto& result) const {
		return continue_parse(ctx, p, src, result);
	}
};

constexpr auto mk_jinja_expression(const std_factory&) { return jiexpr<std_factory>::parsed_expression{}; }
constexpr auto mk_jinja_expression(const std_factory&, bool val) { jiexpr<std_factory>::parsed_expression ret{}; create<bool>(ret) = val; return ret; }
constexpr auto mk_jinja_expression_parser(const std_factory& f) {
	return jiexpr_abstracter{ {}, jiexpr{f}.create_parser<std_factory::parser>() };
}
template<typename interface>
[[noreturn]] constexpr void throw_wrong_interface_error(const std_factory&) {
	using namespace std::literals;
	throw std::runtime_error("cannot perform operation "s + interface::describe_with_chars());
}
[[noreturn]] void throw_key_not_found(const std_factory&, const auto&) {
	using namespace std::literals;
	throw std::runtime_error("attempt to get value by nonexistent key: [cannot convert to string here]"s);
}
template<auto cnt> [[noreturn]] void throw_wrong_parameters_count(const std_factory&) {
	throw std::runtime_error("wrong arguments count: " + std::to_string(cnt));
}

using parser = ascip;
using absd_data = absd::data<std_factory>;
using expr_type = jiexpr<std_factory>;
using jinja_type = jinja<std_factory>;

constexpr auto back_inserter(const std_factory&, auto& v) {
	return std::back_inserter(v);
}

constexpr std::string jinja_to_string(const std_factory& f, const absd_data& obj) {
	auto ret = mk_str(f);
	back_insert_format(back_inserter(ret), obj);;
	return ret;
}

constexpr std::string jinja_to_string(const std_factory& f, const jiexpr<std_factory>::parsed_expression& e) {
	return std::string{};
}

constexpr absd_data jinja_to_data(const std_factory&, const auto& env, const auto& data) {
	//if (data.index()==0) return env.at(std_factory::data_type{get<0>(data)});
	//return visit([](auto& v){return std_factory::data_type{v};}, data);
	return absd_data{};
}

template<typename out_info>
constexpr absd_data jinja_to_data(const std_factory& f, const std::vector<out_info>& d) {
	return absd_data{};
}

int main(int,char**) {
	jinja_type j;
	auto parsed = j.parse_file(ascip::make_source("<% block main %>test<% endblock %>"), "test");
}
