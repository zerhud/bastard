#pragma once

/*************************************************************************
 * Copyright © 2023 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

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
struct call : interface_describer<call> {};
template<typename left, typename right> struct exec_op : interface_describer<exec_op<left, right>> {};
} // namespace interfaces
} // namespace details

} // namespace absd
