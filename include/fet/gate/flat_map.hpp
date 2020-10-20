#pragma once

#include "../core.hpp"

namespace fet
{

namespace impl
{

template <class F>
class FlatMapGate: IGate
{
    F m_func;

public:
    constexpr FlatMapGate(F &&func):
        m_func(std::forward<F>(func))
    { }

    using IGate::OnConnect;

    template <class T>
    constexpr auto GetInfo(const SourceInfo<T> &info) const
    {
        return SourceInfo<typename decltype(m_func(std::declval<T>()))::value_type> {
            . capacity = 0,
        };
    }

private:
    template <class CB>
    class JCT: IJunction
    {
        CB m_cb;

    public:
        constexpr JCT(CB &&cb):
            m_cb(std::forward<CB>(cb))
        { }

        using IJunction::OnConnect;

        template <class E>
        constexpr auto OnNext(std::nullptr_t, E &&e) const
        {
            return m_cb(std::forward<E>(e));
        }
    };

    template <class CB>
    constexpr JCT<CB> make_jct(CB &&cb)
    {
        return { std::forward<CB>(cb) };
    }

public:
    template <class E, class CB>
    constexpr void OnNext(std::nullptr_t, E &&e, CB &&cb) const
    {
        m_func(std::forward<decltype(e)>(e)).Emit(make_jct(std::forward<CB>(cb)));
    }
};

// LINQ で言うところの SelectMany()
// F は source を返すこと
template <class F>
constexpr FlatMapGate<F> flat_map(F &&func)
{
    return { std::forward<F>(func) };
}

} // namespace impl

using impl::flat_map;

} // namespace fet
