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

  constexpr auto call(const data& params) const {
    auto output_holder = ctx->catch_output();
    obj->execute(*ctx);
    return ctx->extract_output_to_data();
  }
};

}
