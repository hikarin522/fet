#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <cstddef>

namespace fet
{

namespace impl
{

template <class F, class C, class R, class... Arg>
struct CallableInfo
{
    using type = F;

    using is_function_pointer = std::is_function<std::remove_pointer_t<type>>;

    using is_member_function_pointer = std::is_member_function_pointer<type>;

    using is_functor = std::is_class<type>;

    using class_type = C;

    using result_type = R;

    using argument_tuple = std::tuple<Arg ...>;

    using argument_count = std::tuple_size<argument_tuple>;

    template <std::ptrdiff_t N>
    using argument_type = std::tuple_element_t < N<0 ? argument_count::value + N : N, argument_tuple>;
};

template <class F, class R, class... A>
auto getCallableInfo(R (*)(A ...)) -> CallableInfo<F, void, R, A ...>;

template <class F, class C, class R, class... A>
auto getCallableInfo(R (C::*)(A ...))->CallableInfo<F, C, R, A ...>;

template <class F, class C, class R, class... A>
auto getCallableInfo(R (C::*)(A ...) const)->CallableInfo<F, C, R, A ...>;

template <class F>
auto getCallableInfo(F)->decltype(getCallableInfo<F>(&F::operator ()));

template <class F>
using callable_info_t = decltype(impl::getCallableInfo<F>(std::declval<F>()));

} // namespace impl

template <class F>
using callable_info_t = impl::callable_info_t<std::remove_cv_t<std::remove_reference_t<F>>>;

template <std::ptrdiff_t N, class F>
using argument_t = typename callable_info_t<F>::template argument_type<N>;

} // namespace fet
