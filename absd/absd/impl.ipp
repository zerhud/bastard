//
// Created by zerhud on 08/03/24.
//

#pragma once

namespace absd {

template<typename factory>
template<typename interface, typename ret_val_t>
constexpr auto data<factory>::throw_wrong_interface_error(ret_val_t ret_val) {
	factory::template throw_wrong_interface_error2<interface>();
	std::unreachable();
	return ret_val;
}

} // namespace absd
