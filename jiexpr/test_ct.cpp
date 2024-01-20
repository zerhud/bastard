#include "absd.hpp"
#include "jiexpr.hpp"

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
	constexpr static void throw_wrong_interface_error(auto&& op) {
		throw std::runtime_error("cannot perform operation "s + op);
	}
};

struct absd_data1 : absd::data<absd_factory<double>, absd_data1> {using base_data_type::operator=;};

using bs1 = bastard<absd_data1, bastard_details::expr_operators_simple, absd_factory<double>>;

int main(int,char**) {
	return 0;
}
