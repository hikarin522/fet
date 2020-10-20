#pragma once

#include <tuple>

#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/for_each.hpp>

#include "../../bind_front.hpp"
#include "../core.hpp"

namespace fet
{

namespace impl
{

template <class... D>
class MuxDrain: IDrain
{
    std::tuple<D ...> m_drains;

public:
    constexpr MuxDrain(D&& ... drains):
        m_drains(std::forward<D>(drains)...)
    { }

private:
    template <class E, size_t ... I>
    constexpr auto _OnConnect(const SourceInfo<E> &info, std::index_sequence<I ...> = std::make_index_sequence<sizeof...(D)>()) const
    {
        return std::make_tuple(std::get<I>(m_drains).OnConnect(info)...);
    }

public:
    template <class E>
    constexpr auto OnConnect(const SourceInfo<E> &info) const
    {
        return _OnConnect(info);
    }

private:
    template <class CTX, class E, size_t ... I>
    constexpr void _OnNext(CTX &ctx, E &&e, std::index_sequence<I ...> = std::make_index_sequence<sizeof...(D)>()) const
    {
        auto onNext = std::make_tuple(std::bind_front(std::get<I>(m_drains).OnNext, std::get<I>(ctx))...);
        boost::fusion::for_each(std::move(onNext), [&](auto &&cb) {
            std::forward<decltype(cb)>(cb)(e);
        });
    }

public:
    template <class CTX, class E>
    constexpr auto OnNext(CTX &ctx, E &&e) const
    {
        return _OnNext(ctx, std::forward<E>(e));
    }

private:
    template <class CTX, class T, size_t ... I>
    static constexpr auto _OnComplete(CTX &&ctx, T &&d, std::index_sequence<I ...> = std::make_index_sequence<sizeof...(D)>())
    {
        return std::make_tuple(std::get<I>(std::forward<T>(d)).OnComplete(std::get<I>(std::forward<CTX>(ctx)))...);
    }

public:
    template <class CTX>
    constexpr decltype(auto) OnComplete(CTX && ctx) const & {
        return _OnComplete(std::forward<CTX>(ctx), m_drains);
    }

    template <class CTX>
    decltype(auto) OnComplete(CTX && ctx) && {
        return _OnComplete(std::forward<CTX>(ctx), std::move(m_drains));
    }
};

// Drain をまとめる
// RX で言うところの Hot に変換するやつ
// 注意
// e が右辺値参照の場合、最初の drain で move されると後続の drain での値は保証されない
// copy_if_rref() 等を用いて適切に対応すること
template <class... D, enable_if<is_drain<D ...>> = nullptr>
constexpr MuxDrain<D ...> mux(D&& ... drains)
{
    return { std::forward<D>(drains)... };
}

} // namespace impl

using impl::mux;

} // namespace fet
