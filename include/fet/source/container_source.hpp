#pragma once

#include "../core.hpp"

namespace fet
{

namespace impl
{

template <class C>
class ContainerSource: ISource
{
    C m_ctr;

public:
    using value_type = typename rm_cvref_t<C>::value_type;

    constexpr ContainerSource(C &&ctr):
        m_ctr(std::forward<C>(ctr))
    { }

    constexpr SourceInfo<value_type> GetInfo() const
    {
        return {
            . capacity = m_ctr.size(),
        };
    }

    template <class J, enable_if<is_jct<J>> = nullptr>
    constexpr decltype(auto) Emit(J && jct) const & {
        decltype(auto) ctx = jct.OnConnect(GetInfo());
        for (auto &&e : m_ctr) {
            jct.OnNext(ctx, std::forward<decltype(e)>(e));
        }
        return ctx;
    }

    template <class J, enable_if<is_jct<J>> = nullptr>
    decltype(auto) Emit(J && jct) && {
        decltype(auto) ctx = jct.OnConnect(GetInfo());
        for (auto &&e : std::forward<C>(m_ctr)) {
            jct.OnNext(ctx, std::forward<decltype(e)>(e));
        }
        return ctx;
    }
};

// コンテナ型から source を生成
template <class C>
constexpr ContainerSource<C> from_container(C &&ctr)
{
    return { std::forward<C>(ctr) };
}

} // namespace impl

using impl::from_container;

} // namespace fet
