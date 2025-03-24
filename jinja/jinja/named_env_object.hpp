#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include "common.hpp"

namespace jinja_details {

template<typename factory>
struct named_env_object {
  using data = typename factory::data_type;
  using context_type = base_jinja_element<factory>::context_type;

  const element_with_name<factory>* obj;
  context_type* ctx;

  constexpr auto size() const { return 1; }
  constexpr auto at(const data& key) const {
    if (key=="name") return ctx->mk_data(obj->name());
    return data{};
  }
  constexpr auto keys(const auto& f) const {
    using name_t = decltype(mk_name(f));
    auto ret = mk_vec<name_t>(f);
    ret.emplace_back("name");
    return ret;
  }
  constexpr auto call(const data& params) const {
    auto output_holder = ctx->catch_output();
    obj->execute(*ctx);
    return ctx->extract_output_to_data();
  }
};

}
