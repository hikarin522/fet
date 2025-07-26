#pragma once

#include <tuple>

#include "../core.hpp"

namespace fet
{

namespace impl
{

template <class F>
class TransformGate: IGate
{
    F m_func;

public:
    constexpr TransformGate(F &&func):
        m_func(std::forward<F>(func))
    { }

    using IGate::OnConnect;

    template <class T>
    constexpr auto GetInfo(const SourceInfo<T> &info) const
    {
        return SourceInfo<decltype(m_func(std::declval<T>()))> {
            . capacity = info.capacity,
        };
    }

    template <class E, class CB>
    constexpr void OnNext(std::nullptr_t, E &&e, CB &&cb) const
    {
        std::forward<CB>(cb)(m_func(std::forward<decltype(e)>(e)));
    }
};

// LINQ で言うところの Select()
template <class F>
constexpr TransformGate<F> transform(F &&func)
{
    return { std::forward<F>(func) };
}

// keySelector に指定するラムダの制約
// - (auto &&e) の様に参照型(右/左辺値参照)を引数とすること ※ (auto e) の様な値型引数は禁止
// - e が右辺値参照の場合 e のメンバへの参照を返してはならない
// - e が右辺値参照であってもラムダの中で move, forward をしてはならない
template <class F>
constexpr auto pair_transform(F &&keySelector)
{
    return transform([fwd = std::tuple<F>(std::forward<F>(keySelector))](auto &&e) {
        using E = rm_rref_t<decltype(e)>;
        return std::pair < std::result_of_t < F(E &&) >, E > {
            std::get<0>(fwd)(std::forward<E>(e)), std::forward<E>(e)
        };
    });
}

// keySelector, valueSelector に指定するラムダの制約
// - (auto &&e) の様に参照型(右/左辺値参照)を引数とすること ※ (auto e) の様な値型引数は禁止
// - e が右辺値参照の場合 e のメンバへの参照を返してはならない
// - e が右辺値参照であってもラムダの中で move, forward をしてはならない
template <class FK, class FV>
constexpr auto pair_transform(FK &&keySelector, FV &&valueSelector)
{
    return transform([fwd = std::tuple<FK, FV>(std::forward<FK>(keySelector), std::forward<FV>(valueSelector))](auto &&e) {
        using E = rm_rref_t<decltype(e)>;
        return std::pair < std::result_of_t < FK(E &&) >, std::result_of_t < FV(E &&) >> {
            std::get<0>(fwd)(std::forward<E>(e)), std::get<1>(fwd)(std::forward<E>(e))
        };
    });
}

template <class E, class T, size_t ... I>
constexpr auto _tuple_transform(E &&e, T &&tpl, std::index_sequence<I ...>)
{
    return std::tuple_cat(
        std::tuple<E> { std::forward<E>(e) },
        std::tuple<std::result_of_t<std::tuple_element_t<I, rm_cvref_t<T>>(E &&)> ...> {
            std::get<I>(std::forward<T>(tpl))(std::forward<E>(e))...
        }
    );
}

// func に指定するラムダの制約
// - (auto &&e) の様に参照型(右/左辺値参照)を引数とすること ※ (auto e) の様な値型引数は禁止
// - e が右辺値参照の場合 e のメンバへの参照を返してはならない
// - e が右辺値参照であってもラムダの中で move, forward をしてはならない
template <class... F>
constexpr auto tuple_transform(F&& ... func)
{
    return transform([fwd = std::tuple<F ...>(std::forward<F>(func)...)](auto &&e) {
        return _tuple_transform(std::forward<decltype(e)>(e), fwd, std::make_index_sequence<sizeof...(F)>());
    });
}

} // namespace impl

using impl::transform;
using impl::pair_transform;
using impl::tuple_transform;

} // namespace fet
