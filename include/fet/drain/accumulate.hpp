#pragma once

#include <tuple>

#include <boost/logic/tribool.hpp>

#include "../core.hpp"
#include "result_trainsform.hpp"

namespace fet
{

namespace impl
{

template <class R, class F>
class AccumulateDrain: IDrain
{
    R m_init;
    F m_op;

public:
    constexpr AccumulateDrain(R &&init, F &&op):
        m_init (std::forward<R>(init)),
        m_op   (std::forward<F>(op))
    { }

    template <class E>
    constexpr R OnConnect(const SourceInfo<E>&) const
    {
        return m_init;
    }

    template <class E, enable_if<std::is_same<R, std::result_of_t<F(R &&, E &&)>>> = nullptr>
    constexpr void OnNext(R &ctx, E &&e) const
    {
        ctx = m_op(std::move(ctx), std::forward<E>(e));
    }

    template <class E, enable_if<std::is_same<void, std::result_of_t<F(R&, E &&)>>> = nullptr>
    constexpr void OnNext(R &ctx, E &&e) const
    {
        m_op(ctx, std::forward<E>(e));
    }

    constexpr R OnComplete(R &&ctx) const
    {
        return std::forward<decltype(ctx)>(ctx);
    }
};

// LINQ で言うところの Aggregate()
template <class R, class F>
constexpr AccumulateDrain<R, F> accumulate(R &&init, F &&op)
{
    return { std::forward<R>(init), std::forward<F>(op) };
}

template <class A, class F, class R>
constexpr auto accumulate(A &&init, F &&op, R &&sel)
{
    return result_transform(accumulate(std::forward<A>(init), std::forward<F>(op)), std::forward<R>(sel));
}

template <class I, class F>
constexpr auto count_if(I init, F &&pred)
{
    return accumulate(init, [fwd = std::tuple<F>(std::forward<F>(pred))](auto &count, auto &&e) {
        if (std::get<0>(fwd)(std::forward<decltype(e)>(e))) {
            ++count;
        }
    });
}

template <class I = size_t, class F>
constexpr auto count_if(F &&pred)
{
    return count_if<I>(0, std::forward<F>(pred));
}

template <class B, class F, enable_if<std::is_same<B, boost::tribool>> = nullptr>
constexpr auto all_of(F &&pred)
{
    return accumulate(boost::indeterminate, [fwd = std::tuple<F>(std::forward<F>(pred))](auto &&r, auto &&e) {
        return !!r && std::get<0>(fwd)(std::forward<decltype(e)>(e));
    });
}

template <class B = bool, class F, enable_if<std::is_same<B, bool>> = nullptr>
constexpr auto all_of(F &&pred)
{
    return result_transform(all_of<boost::tribool>(std::forward<F>(pred)), [](auto &&e) { return e == true; });
}

template <class B, B init, class F>
constexpr auto _any_of(F &&pred)
{
    return accumulate(init, [fwd = std::tuple<F>(std::forward<F>(pred))](auto &&r, auto &&e) {
        return r || std::get<0>(fwd)(std::forward<decltype(e)>(e));
    });
}

template <class B = bool, class F, enable_if<std::is_same<B, bool>> = nullptr>
constexpr auto any_of(F &&pred)
{
    return _any_of<B, false>(std::forward<F>(pred));
}

template <class B, class F, enable_if<std::is_same<B, boost::tribool>> = nullptr>
constexpr auto any_of(F &&pred)
{
    return _any_of<B, boost::indeterminate>(std::forward<F>(pred));
}

} // namespace impl

using impl::accumulate;
using impl::count_if;
using impl::all_of;
using impl::any_of;

} // namespace fet
