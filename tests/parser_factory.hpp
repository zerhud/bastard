#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <memory>
#include <variant>

namespace tests {

struct parser_factory {
	template<typename... types> using variant_t = std::variant<types...>;
	template<typename type> using ast_forwarder = std::unique_ptr<type>;
};
constexpr auto mk_fwd(const parser_factory&, auto& v) {
	using v_type = std::decay_t<decltype(v)>;
	static_assert( !std::is_pointer_v<v_type>, "the result have to be a unique_ptr like type" );
	static_assert( !std::is_reference_v<v_type>, "the result have to be a unique_ptr like type" );
	v = std::make_unique<typename v_type::element_type>();
	return v.get();
}
constexpr auto mk_result(const parser_factory&, auto&& v) {
	using expr_t = std::decay_t<decltype(v)>;
	return std::make_unique<expr_t>(std::forward<decltype(v)>(v));
}

} // namespace tests
