#pragma once

#include <vector>

#include "../core.hpp"

namespace fet
{

namespace impl
{

template <template <class> class C>
class ToContainerDrain: IDrain
{
public:
    template <class E>
    constexpr auto OnConnect(const SourceInfo<E> &info) const
    {
        C<E> ctr;
        ctr.reserve(info.capacity);
        return std::move(ctr);
    }

    template <class E, class T>
    constexpr void OnNext(C<E> &ctx, T &&e) const
    {
        ctx.push_back(std::forward<T>(e));
    }

    template <class E>
    constexpr auto OnComplete(C<E> &&ctx) const
    {
        return std::move(ctx);
    }
};

template <class E>
using vector = std::vector<E>;

inline constexpr auto to_vector()
{
    return ToContainerDrain<vector>();
}

template <class F>
constexpr auto to_vector(F &&func)
{
    return transform(std::forward<F>(func)) | to_vector();
}

} // namespace impl

using impl::to_vector;

} // namespace fet
