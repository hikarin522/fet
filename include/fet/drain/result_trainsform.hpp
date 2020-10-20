#pragma once

#include "../core.hpp"

namespace fet
{

namespace impl
{

template <class D, class F, enable_if<is_drain<D>> = nullptr>
class ResultTransformDrain: D
{
    F m_func;

public:
    constexpr ResultTransformDrain(D &&drain, F &&func):
        D      (std::forward<D>(drain)),
        m_func (std::forward<F>(func))
    { }

    using D::OnConnect;
    using D::OnNext;

    template <class CTX>
    constexpr decltype(auto) OnComplete(CTX && ctx) const & {
        return m_func(D::OnComplete(std::forward<CTX>(ctx)));
    }

    template <class CTX>
    decltype(auto) OnComplete(CTX && ctx) && {
        return std::forward<F>(m_func)(D::OnComplete(std::forward<CTX>(ctx)));
    }
};

template <class D, class F, enable_if<is_drain<D>> = nullptr>
constexpr ResultTransformDrain<D, F> result_transform(D &&drain, F &&func)
{
    return { std::forward<D>(drain), std::forward<F>(func) };
}

} // namespace impl

using impl::result_transform;

} // namespace fet
