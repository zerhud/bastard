//
// Created by zerhud on 08/03/24.
//

#pragma once

namespace absd {

namespace details {

template<typename final_name>
struct interface_describer {
	constexpr static const char* describe_with_chars() {
		return __PRETTY_FUNCTION__ ;
	}
};
namespace interfaces {
struct contains : interface_describer<contains> {};
struct put : interface_describer<put> {};
struct keys : interface_describer<keys> {};
struct push_back : interface_describer<push_back> {};
struct at_ind : interface_describer<at_ind> {};
struct at_key : interface_describer<at_key> {};
struct cmpget_workaround : interface_describer<cmpget_workaround> {};
struct call : interface_describer<call> {};
} // namespace interfaces
} // namespace details

} // namespace absd
