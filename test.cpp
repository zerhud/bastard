#include "bastard.hpp"
//#include "ascip_all.hpp"
#include "ascip.hpp"

#include <tuple>
#include <vector>
#include <memory>
#include <string>
#include <variant>

#include <iostream>

using namespace std::literals;
using parser = ascip<std::tuple>;
using data_type = abstract_data<std::variant,std::string,int,double>;
using data_type2 = abstract_data<std::variant,std::string,short,float>;
using op_factory = bastard_details::expr_operators_simple ;

struct data_factory {
	template<typename type> using ast_forwarder = std::unique_ptr<type>;
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
};

int main(int,char**) {
	static_assert( bastard<data_type,op_factory,data_factory>::test<parser>() );
	static_assert( bastard<data_type2,op_factory,data_factory>::test<parser>() );

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
