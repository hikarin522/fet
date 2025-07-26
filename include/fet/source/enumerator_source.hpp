#pragma once

#include "../../bind_front.hpp"
#include "../callable_info.hpp"
#include "../core.hpp"

namespace fet
{

namespace impl
{

template <class E, class F>
class EnumeratorSource: ISource
{
    F m_func;
    const size_t m_size;

public:
    using value_type = E;

    constexpr EnumeratorSource(F &&func, size_t n):
        m_func(std::forward<F>(func)), m_size(n)
    { }

    constexpr SourceInfo<value_type> GetInfo() const
    {
        return {
            . capacity = m_size,
        };
    }

    template <class J, enable_if<is_jct<J>> = nullptr>
    constexpr decltype(auto) Emit(J && jct) const & {
        decltype(auto) ctx = jct.OnConnect(GetInfo());
        m_func([&](auto &&e) {
            jct.OnNext(ctx, std::forward<decltype(e)>(e));
        });
        return ctx;
    }

    template <class J, enable_if<is_jct<J>> = nullptr>
    decltype(auto) Emit(J && jct) && {
        decltype(auto) ctx = jct.OnConnect(GetInfo());
        std::forward<F>(m_func)([&](auto &&e) {
            jct.OnNext(ctx, std::forward<decltype(e)>(e));
        });
        return ctx;
    }
};

// EnumerateXXX 関数から source を生成
template <class E, class F>
constexpr EnumeratorSource<E, F> from_enumerator(F &&func, size_t n = 0)
{
    return { std::forward<F>(func), n };
}

template <class F>
constexpr auto from_enumerator(F &&func, size_t n = 0)
{
    return from_enumerator<argument_t<0, argument_t<0, F>>>(std::forward<F>(func), n);
}

template <class E, class F, class... Args>
constexpr auto from_enumerator(F &&func, Args&& ... args)
{
    return from_enumerator<E>(std::bind_front(std::forward<F>(func), std::forward<Args>(args)...));
}

template <class F, class... Args>
constexpr auto from_enumerator(F &&func, Args&& ... args)
{
    return from_enumerator<argument_t<0, argument_t<-1, F>>>(std::forward<F>(func), std::forward<Args>(args)...);
}

} // namespace impl

using impl::from_enumerator;

} // namespace fet
