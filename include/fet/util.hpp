#pragma once

#include <type_traits>
#include <utility>

namespace fet
{

namespace impl
{

template <class T, class... Ts>
struct and_t: std::conditional_t<T::value, and_t<Ts ...>, std::false_type> { };

template <class T>
struct and_t<T>: T { };

template <class... T>
using enable_if = std::enable_if_t<and_t<T ...>::value, std::nullptr_t>;

template <class B, class... D>
using is_base_of = and_t<std::is_base_of<B, std::remove_reference_t<D>> ...>;

template <class T>
using rm_ref_t = std::remove_reference_t<T>;

template <class T>
using rm_cvref_t = std::remove_cv_t<rm_ref_t<T>>;

template <class T>
using rm_rref_t = std::conditional_t<std::is_rvalue_reference<T>::value, rm_ref_t<T>, T>;

} // namespace impl

} // namespace fet
