#pragma once

#include "../core.hpp"

namespace fet
{

namespace impl
{

template <class F>
class FilterGate: IGate
{
    F m_pred;

public:
    constexpr FilterGate(F &&pred):
        m_pred(std::forward<F>(pred))
    { }

    using IGate::GetInfo;
    using IGate::OnConnect;

    template <class E, class CB>
    constexpr void OnNext(std::nullptr_t, E &&e, CB &&cb) const
    {
        if (m_pred(e)) {
            std::forward<CB>(cb)(std::forward<decltype(e)>(e));
        }
    }
};

// LINQ で言うところの Where()
template <class F>
constexpr FilterGate<F> filter(F &&pred)
{
    return { std::forward<F>(pred) };
}

inline auto filterNull()
{
    return filter([](const auto &e) { return e != nullptr; });
}

template <size_t N>
auto filterNull()
{
    return filter([](const auto &e) { return std::get<N>(e) != nullptr; });
}

} // namespace impl

using impl::filter;
using impl::filterNull;

} // namespace fet
