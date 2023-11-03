#include "absd.hpp"
#include "bastard.hpp"
//#include "ascip_all.hpp"
#include "ascip.hpp"

#include <map>
#include <tuple>
#include <vector>
#include <memory>
#include <string>
#include <variant>
#include <cassert>

#include <iostream>

using namespace std::literals;

//TODO: inject into absd own array, object and functor (also functor with object's methods)
//      use this types_set to transfer few implementations into absd
template<typename... types> struct types_set{};

template<typename float_point>
struct absd_factory {
	template<typename... types> using variant = std::variant<types...>;
	template<typename type> using vector = std::vector<type>;
	using float_point_t = float_point;
	using string_t = std::string;
	using empty_t = std::monostate;
//	template<typename key,typename value> using map_t = std::map<key,value>;

	template<typename type> constexpr static auto mk_vec(){ return std::vector<type>{}; }
//	template<typename key, typename value>
//	constexpr static auto mk_map(){ return map_t<key,value>{}; }
	constexpr static auto mk_ptr(auto d) { return std::make_unique<decltype(d)>( std::move(d) ); }
	constexpr static void deallocate(auto* ptr) noexcept { delete ptr; }
	constexpr static void throw_wrong_interface_error(auto&& op) {
		throw std::runtime_error("cannot perform operation "s + op);
	}
};

struct absd_data1 : absd::data<absd_factory<double>, absd_data1> {using base_data_type::operator=;};

template<typename integer, typename float_point>
struct abstract_data_factory {
	template<typename... types> using variant = std::variant<types...>;
	template<typename type> using smart_ptr = std::unique_ptr<type>;
	using float_point_t = float_point;
	using string_t = std::string;
	using integer_t = integer;

	template<typename type>
	constexpr auto mk_ptr(auto&&... args) { return std::make_unique<type>(std::forward<decltype(args)>(args)...); }

	[[noreturn]] constexpr static void throw_wrong_interface_error(auto&& op) {
		throw std::runtime_error("cannot perform operation "s + op);
	}
};

using parser = ascip<std::tuple>;
using data_type = abstract_data<abstract_data_factory<int,double>,std::vector<int>>;
using data_type2 = abstract_data<abstract_data_factory<short,float>,std::vector<short>>;
using op_factory = bastard_details::expr_operators_simple ;

struct data_factory {
	template<typename type> using ast_forwarder = std::unique_ptr<type>;
	template<typename type> using vec_type = std::vector<type> ;

	constexpr auto mk_fwd(auto& v) const {
		v = std::make_unique<typename std::decay_t<decltype(v)>::element_type>();
		return v.get();
	}
	constexpr auto mk_result(auto& v) const {
		using expr_t = std::decay_t<decltype(v)>;
		return std::make_unique<expr_t>(std::move(v));
	}
	template<typename type> constexpr auto mk_vec() const {
		return std::vector<type>{};
	}
	template<typename key, typename value> constexpr auto mk_map() const {
		//return std::map<key,value>{};
		return bastard_details::constexpr_kinda_map<std::vector, key, value>{};
	}

	template<typename type>
	constexpr auto mk_ptr(auto&&... args) { return std::make_unique<type>(std::forward<decltype(args)>(args)...); }
};

bool check_exception(auto fnc) {
	bool ok = false;
	try{ fnc(); }
	catch(const std::exception&){ ok = true; }
	return ok;
}

int main(int,char**) {
	static_assert( bastard<data_type,op_factory,data_factory>::test<parser>() );
	static_assert( bastard<data_type2,op_factory,data_factory>::test<parser>() );

	static_assert( absd_data1::test() );

	assert(absd_data1::test_callable_cases_rt());
	assert(check_exception([]{ absd_data1{1}.push_back(absd_data1(2)); }));

	//auto result = bastard<data_type,op_factory>::test_terms<std::unique_ptr,parser>(fwd_ast, result_maker, "(5+2)*3");
	//std::cout << "===\n" << result << std::endl;

	/*
	data_type env;
	op_factory ops;
	bastard b{ &env, ops };
	std::string str;
	getline(std::cin, str);
	std::cout << (int)(b( b.parse_str<std::unique_ptr,parser>( std::string_view{ str }, fwd_ast ))) << std::endl;
	*/

	return 0;
}
