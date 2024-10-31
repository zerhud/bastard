#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

namespace absd::details {

template<typename...> struct type_list {};
template<typename t> struct _type_c{ using type=t; };
template<typename t> constexpr _type_c<t> type_c;
template<typename t> constexpr bool operator==(_type_c<t>, _type_c<t>) { return true; }
template<typename l, typename r> constexpr bool operator==(_type_c<l>, _type_c<r>) { return false; }
template<typename t> constexpr bool operator!=(_type_c<t>, _type_c<t>) { return false; }
template<typename l, typename r> constexpr bool operator!=(_type_c<l>, _type_c<r>) { return true; }

template<typename type>
concept iteratable =
		requires(const type& v){ begin(v); end(v); } ||
		requires(const type& v){ v.begin(); v.end(); };
template<typename type, typename key_type>
concept has_contains = requires(const type& obj, key_type key){ obj.contains(key); };
template<typename type, typename data_type>
concept as_object =
		   requires(const type& v, data_type key){ v.at(key); v.size(); }
		&& has_contains<type, data_type>
		&& ( iteratable<type> || requires(const type& v){ {v.keys()}->iteratable;} )
		;
template<typename type, typename data_type>
concept as_array =
		   requires(const type& v){ data_type{v.at(0)}; v.size(); }
		&& ( iteratable<type> || has_contains<type, data_type> );

} // namespace absd::details
