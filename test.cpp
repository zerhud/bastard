#include "bastard.hpp"
#include "ascip.hpp"

#include <tuple>
#include <memory>
#include <string>
#include <variant>

#include <iostream>

using parser = ascip<std::tuple>;
using data_type = abstract_data<std::variant,std::string,int,double>;
using data_type2 = abstract_data<std::variant,std::string,short,float>;
using op_factory = bastard_details::expr_operators_simple ;

int main(int,char**) {
	auto fwd_ast = [](auto& v){v=std::make_unique<typename std::decay_t<decltype(v)>::element_type>();return v.get();};
	static_assert( bastard<data_type,op_factory>::test<std::unique_ptr,parser>(fwd_ast) );
	//static_assert( bastard<data_type2,op_factory>::test<std::unique_ptr,parser>(fwd_ast) );

	//auto result = bastard<data_type,op_factory>::test_terms<std::unique_ptr,parser>(fwd_ast, "(5+2)*3");
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
