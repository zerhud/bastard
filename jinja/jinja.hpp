#pragma once

/*************************************************************************
 * Copyright © 2024 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of cogen.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#include <utility>

#include "jinja/common.hpp"
#include "jinja/content.hpp"
#include "jinja/operators.hpp"
#include "jinja/block_content.hpp"

namespace jinja_details {

template<typename factory>
struct element_with_name : base_jinja_element<factory> {
	using string_t = decltype(mk_str(std::declval<factory>()));
	virtual const string_t& name() const =0 ;
};

template<typename factory>
struct template_block : element_with_name<factory> {
	using base = element_with_name<factory>;
	using string_t = base::string_t;
	using context_type = base::context_type;
	using p = factory::parser;
	template<auto s> using th = p::template tmpl<s>;

	constexpr const string_t& name() const override { return _name; }
	constexpr void execute(context_type& ctx) const override { }

	constexpr static auto make_holder(const factory& f) {
		using ptr_type = decltype(mk_empty_ptr<const base>(f));
		return mk_vec<ptr_type>(f);
	}

	constexpr template_block() : template_block(factory{}) {}
	constexpr explicit template_block(factory f)
	: f(std::move(f))
#ifdef __clang__
	//TODO:GCC15: remove ifdef when gcc bug will be fixed (cannot compile for some reason the parser.cpp test)
	, _name(mk_str(this->f))
#endif
	, holder(this->f)
	, blocks(make_holder(this->f))
	{}

	constexpr static auto struct_fields_count() { return 4; }
	factory f;
	string_t _name;
	block_content<factory> holder;
	decltype(make_holder(std::declval<factory>())) blocks;

	constexpr static auto mk_parser(factory f) {
		using bp = base_parser<factory>;
		constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
		return
		   bp::mk_block_begin() >> p::template lit<"template"> >> ++ident
		>> ++block_content<factory>::mk_parser(f)
		>> p::template lit<"endtemplate"> >> bp::mk_block_end()
		;
	}
};

template<typename factory>
struct named_block : element_with_name<factory> {
	using base =  element_with_name<factory>;
	using string_t = base::string_t;
	using context_type = base::context_type;
	using p = factory::parser;
	template<auto s> using th = p::template tmpl<s>;

	constexpr const string_t& name() const override { return _name; }
	constexpr void execute(context_type& ctx) const override { }

	constexpr static auto struct_fields_count() { return 5; }
	factory f;
	trim_info<factory> begin_left;
	string_t _name;
	block_content<factory> holder;
	trim_info<factory> end_right;

	constexpr named_block() : named_block(factory{}) {}
	constexpr explicit named_block(factory f)
	: f(std::move(f))
#ifdef __clang__
	//TODO:GCC15: remove ifdef when gcc bug will be fixed (cannot compile for some reason the parser.cpp test)
	, _name(mk_str(this->f))
#endif
	, holder(this->f)
	{}

	constexpr auto size() const { return holder.size(); }
	constexpr auto& operator[](auto ind) const { return holder[ind]; }

	constexpr static auto mk_parser() {
		return mk_parser(factory{});
	}
	constexpr static auto mk_parser(factory f) {
		using bp = base_parser<factory>;
		constexpr auto trim_parser = trim_info<factory>::mk_parser();
		constexpr auto ident = lexeme(p::alpha >> *(p::alpha | p::d10 | th<'_'>::char_));
		return
		++lexeme(bp::mk_block_begin() >> trim_parser)
		>> p::template lit<"block"> >> ++ident
		>> ++block_content<factory>::mk_parser(f)
		>> p::template lit<"endblock">
		>> ++trim_parser >> bp::mk_block_end()
		;
	}
};

} // namespace jinja_details

template<typename factory>
struct jinja {
	factory f;

	constexpr jinja() : jinja(factory{}) {}
	constexpr explicit jinja(factory f) : f(std::move(f)) {}
};
