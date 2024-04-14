#pragma once

//          Copyright Hudyaev Alexey 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

template<typename t>
struct clang_workaround { t v; };

template<typename t> clang_workaround(t) -> clang_workaround<t>;

constexpr auto wrap_for_clang(const auto& v) {
	return clang_workaround{ v };
}

template<typename type>
struct undefined_value {
	const type value;
	static const undefined_value<type> undefined_object;
};

constexpr bool is_sym_allowed_in_cpp_ident(auto sym) {
	//ascii: 48(30)-57(39) - digits, 65(41)-90(5A) A-Z, 97(61)-122(7A) a-z, 95(5F) == _
	return
			  (((0x30<=sym) + (sym<=0x39))==2)
			+ (((0x41<=sym) + (sym<=0x5A))==2)
			+ (((0x61<=sym) + (sym<=0x7A))==2)
			+ (sym == 0x5F)
	;
}

constexpr auto extract_last_name(auto&& src) {
	auto last_pos = -1;
	auto first_pos = -1;
	for(auto cur_pos=src.size()-1;0<cur_pos;--cur_pos) {
		const bool is_normal = is_sym_allowed_in_cpp_ident(src[cur_pos]);
		first_pos += (cur_pos * !is_normal) * (last_pos>0) * (first_pos<0);
		last_pos += (cur_pos * is_normal) * (last_pos<0);
	}
	first_pos += 2;
	last_pos += 2;
	return src.substr(first_pos, last_pos-first_pos);
}

template<typename factory, auto type>
constexpr auto object_name() {
	return extract_last_name( typename factory::string_view{factory::source_location::current().function_name()} );
}

template<typename factory, typename type>
constexpr auto type_name() {
	return extract_last_name( typename factory::string_view{factory::source_location::current().function_name()} );
}

template<typename factory, auto ind, typename type>
constexpr auto field_name() {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#pragma clang diagnostic ignored "-Wundefined-inline"
#endif
	const auto& obj = undefined_value<type>::undefined_object.value;
	return object_name< factory, wrap_for_clang(&get<ind>(obj)) >();
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

template<typename t> struct _type_c { using type = t;};
template<typename t> constexpr _type_c<t> type_c{};
template<auto v> struct _object_c { };