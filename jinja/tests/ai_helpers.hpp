#pragma once

#include <memory>
#include <utility>
#include <type_traits>

namespace ai_helpers {

template<typename Signature>
struct FunctionBase;

template<typename R, typename... Args>
struct FunctionBase<R(Args...)> {
  virtual constexpr R operator()(Args... args) const = 0;
  virtual ~FunctionBase() = default;
};

template<typename F, typename R, typename... Args>
struct FunctionImpl : FunctionBase<R(Args...)> {
  F func;

  constexpr explicit FunctionImpl(F f) : func(std::move(f)) {}

  constexpr R operator()(Args... args) const override {
    return func(std::forward<Args>(args)...);
  }
};

template<typename Signature> struct constexpr_function;

template<typename R, typename... Args>
struct constexpr_function<R(Args...)> {
  FunctionBase<R(Args...)>* impl;

  template<typename F>
  constexpr constexpr_function(F f) : impl(new FunctionImpl<F, R, Args...>(std::move(f)))
  {
    static_assert(std::is_invocable_r_v<R, F, Args...>, "Function type must match signature R(Args...)");
  }

  constexpr constexpr_function() : impl(nullptr) {}

  constexpr R operator()(Args... args) const {
    if (!impl) {
      if constexpr (std::is_default_constructible_v<R>) return R{};
      else throw "Null function call";
    }
    return (*impl)(std::forward<Args>(args)...);
  }

  constexpr explicit operator bool() const noexcept {
    return impl != nullptr;
  }
};

}