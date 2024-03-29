#include "absd.hpp"
#include "jiexpr.hpp"

//#include <ascip.hpp>
#include "../ascip_all.hpp"
#include <vector>
#include <memory>
#include <variant>

using namespace std::literals;

template<typename float_point>
struct absd_factory {
	template<typename... types> using variant = std::variant<types...>;
	template<typename type> using vector = std::vector<type>;
	using float_point_t = float_point;
	using string_t = std::string;
	using empty_t = std::monostate;

	template<typename type> constexpr static auto mk_vec(){ return std::vector<type>{}; }
//	template<typename key, typename value>
//	constexpr static auto mk_map(){ return map_t<key,value>{}; }
	constexpr static auto mk_ptr(auto d) { return std::make_unique<decltype(d)>( std::move(d) ); }
	constexpr static void deallocate(auto* ptr) noexcept { delete ptr; }
	template<typename interface>
	constexpr static void throw_wrong_interface_error() {
		throw std::runtime_error("cannot perform operation "s + interface::describe_with_chars());
	}
	template<auto cnt>
	constexpr static void throw_wrong_parameters_count() {
		throw std::runtime_error("wrong arguments count: " + std::to_string(cnt));
	}
};

struct bastard_factory {
	template<typename... types> using variant_t = std::variant<types...>;
	template<typename type> using ast_forwarder = std::unique_ptr<type>;
	template<typename type> using vec_type = std::vector<type> ;

	constexpr auto mk_fwd(auto& v) const {
		using v_type = std::decay_t<decltype(v)>;
		static_assert( !std::is_pointer_v<v_type>, "the result have to be a unique_ptr like type" );
		static_assert( !std::is_reference_v<v_type>, "the result have to be a unique_ptr like type" );
		v = std::make_unique<typename v_type::element_type>();
		return v.get();
	}
	constexpr auto mk_result(auto& v) const {
		using expr_t = std::decay_t<decltype(v)>;
		return std::make_unique<expr_t>(std::move(v));
	}
	template<typename type> constexpr auto mk_vec() const {
		return std::vector<type>{};
	}

	template<typename type>
	constexpr auto mk_ptr(auto&&... args) { return std::make_unique<type>(std::forward<decltype(args)>(args)...); }
};

using parser = ascip<std::tuple>;
using absd_data1 = absd::data<absd_factory<double>>;

using bs1 = bastard<absd_data1, bastard_details::expr_operators_simple, bastard_factory>;

int main(int,char**) {
	static_assert( bs1::test<parser>() );
	return 0;
}
