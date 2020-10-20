#pragma once

#include "util.hpp"

/* ****************************************************************
    説明
    source : LINQ で言うところの IEnumerable 等
    gate   : LINQ で言うところの Where, Select 等
    drain  : LINQ で言うところの ToList, Aggregate 等

    書き方: Range, RxCpp と同じようにパイプ演算子使う
    auto result = source | gate | gate | ... | drain;

    型結合則
    source | gate  => source
    gate   | gate  => gate
    gate   | drain => drain
    source | drain => decltype(drain.OnComplete())
**************************************************************** */
namespace fet
{

namespace impl
{

template <class T>
struct SourceInfo
{
    using value_type = T;
    size_t capacity;
};

/* ****************************************************************
    std::is_base_of<> の SFINAE によるディスパッチ用タグクラス
    それぞれコメント内のメソッドを実装すること
 */

class IJunction
{
    // CTX OnConnect(const SourceInfo<E>&) const;
    // void OnNext(CTX&, E&&) const;

protected:
    template <class T>
    constexpr auto OnConnect(const SourceInfo<T>&) const { return nullptr; }
};

class ISource
{
    // using value_type;
    // SourceInfo<value_type> GetInfo() const;
    // CTX Emit(J&&); enable_if J: IJunction
};

class IGate
{
    // SourceInfo<T> GetInfo(const SourceInfo<E>&) const;
    // CTX OnConnect(const SourceInfo<E>&) const;
    // void OnNext(CTX&, E&&, callback) const;

protected:
    template <class E>
    constexpr auto GetInfo(const SourceInfo<E> &info) const { return info; }

    template <class E>
    constexpr auto OnConnect(const SourceInfo<E>&) const { return nullptr; }
};

class IDrain: IJunction
{
    // CTX OnConnect(const SourceInfo<E>&) const;
    // void OnNext(CTX&, E&&) const;
    // R OnComplete(CTX&&) const;

protected:
    using IJunction::OnConnect;
};

/* ****************************************************************
    SFINAE 用 type_traits
    gcc4.9 では変数テンプレート使えなくて辛い
    C++20 になったら concepts に移行したい
 */

template <class... T>
using is_jct = is_base_of<IJunction, T ...>;

template <class... T>
using is_src = is_base_of<ISource, T ...>;

template <class... T>
using is_gate = is_base_of<IGate, T ...>;

template <class... T>
using is_drain = is_base_of<IDrain, T ...>;

/* ****************************************************************
    型結合用クラス
    source | gate  => source
    gate   | gate  => gate
    gate   | drain => drain
 */

template <class G, class J, enable_if<is_gate<G>, is_jct<J>> = nullptr>
class Junction: IJunction
{
protected:
    G m_gate;
    J m_jct;

public:
    constexpr Junction(G &&gate, J &&jct):
        m_gate (std::forward<G>(gate)),
        m_jct  (std::forward<J>(jct))
    { }

    template <class E>
    constexpr auto OnConnect(const SourceInfo<E> &info) const
    {
        return std::pair<decltype(m_gate.OnConnect(info)), decltype(m_jct.OnConnect(m_gate.GetInfo(info)))> {
            m_gate.OnConnect(info), m_jct.OnConnect(m_gate.GetInfo(info))
        };
    }

    template <class CTX, class E>
    constexpr auto OnNext(CTX &ctx, E &&e) const
    {
        return m_gate.OnNext(ctx.first, std::forward<E>(e), [&](auto &&e) {
            return m_jct.OnNext(ctx.second, std::forward<decltype(e)>(e));
        });
    }
};

template <class G, class J, enable_if<is_gate<G>, is_jct<J>> = nullptr>
constexpr Junction<G, J> make_jct(G &&gate, J &&jct)
{
    return { std::forward<G>(gate), std::forward<J>(jct) };
}

template <class S, class G, enable_if<is_src<S>, is_gate<G>> = nullptr>
class Source: ISource
{
    S m_src;
    G m_gate;

public:
    constexpr auto GetInfo() const
    {
        return m_gate.GetInfo(m_src.GetInfo());
    }

    using value_type = typename decltype(std::declval<Source<S, G>>().GetInfo())::value_type;

    constexpr Source(S &&src, G &&gate):
        m_src  (std::forward<S>(src)),
        m_gate (std::forward<G>(gate))
    { }

    template <class J, enable_if<is_jct<J>> = nullptr>
    constexpr decltype(auto) Emit(J && jct) const & {
        return m_src.Emit(make_jct(m_gate, std::forward<J>(jct))).second;
    }

    template <class J, enable_if<is_jct<J>> = nullptr>
    decltype(auto) Emit(J && jct) && {
        return std::forward<S>(m_src).Emit(make_jct(std::forward<G>(m_gate), std::forward<J>(jct))).second;
    }
};

template <class S, class G, enable_if<is_src<S>, is_gate<G>> = nullptr>
constexpr Source<S, G> make_src(S &&src, G &&gate)
{
    return { std::forward<S>(src), std::forward<G>(gate) };
}

template <class G1, class G2, enable_if<is_gate<G1, G2>> = nullptr>
class Gate: IGate
{
    G1 m_gate1;
    G2 m_gate2;

public:
    constexpr Gate(G1 &&gate1, G2 &&gate2):
        m_gate1 (std::forward<G1>(gate1)),
        m_gate2 (std::forward<G2>(gate2))
    { }

    template <class E>
    constexpr auto GetInfo(const SourceInfo<E> &info) const
    {
        return m_gate2.GetInfo(m_gate1.GetInfo(info));
    }

    template <class E>
    constexpr auto OnConnect(const SourceInfo<E> &info) const
    {
        return std::pair<decltype(m_gate1.OnConnect(info)), decltype(m_gate2.OnConnect(m_gate1.GetInfo(info)))> {
            m_gate1.OnConnect(info), m_gate2.OnConnect(m_gate1.GetInfo(info))
        };
    }

    template <class CTX, class E, class CB>
    constexpr decltype(auto) OnNext(CTX & ctx, E && e, CB && cb) const {
        return m_gate1.OnNext(ctx.first, std::forward<E>(e), [&](auto &&e) {
            return m_gate2.OnNext(ctx.second, std::forward<decltype(e)>(e), std::forward<CB>(cb));
        });
    }
};

template <class G1, class G2, enable_if<is_gate<G1, G2>> = nullptr>
constexpr Gate<G1, G2> make_gate(G1 &&gate1, G2 &&gate2)
{
    return { std::forward<G1>(gate1), std::forward<G2>(gate2) };
}

template <class G, class D, enable_if<is_gate<G>, is_drain<D>> = nullptr>
class Drain: IDrain,
             Junction<G, D>
{
public:
    constexpr Drain(G &&gate, D &&drain):
        Junction<G, D>(std::forward<G>(gate), std::forward<D>(drain))
    { }

    using Junction<G, D>::OnConnect;
    using Junction<G, D>::OnNext;

    template <class CTX>
    constexpr decltype(auto) OnComplete(CTX && ctx) const & {
        return this->m_jct.OnComplete(std::forward<CTX>(ctx).second);
    }

    template <class CTX>
    decltype(auto) OnComplete(CTX && ctx) && {
        return std::forward<D>(this->m_jct).OnComplete(std::forward<CTX>(ctx).second);
    }
};

template <class G, class D, enable_if<is_gate<G>, is_drain<D>> = nullptr>
constexpr Drain<G, D> make_drain(G &&gate, D &&drain)
{
    return { std::forward<G>(gate), std::forward<D>(drain) };
}

/* ****************************************************************
    パイプ演算子オーバーロード
    source | gate  => source
    gate   | gate  => gate
    gate   | drain => drain
    source | drain => decltype(drain.OnComplete())
 */

template <class S, class G, enable_if<is_src<S>, is_gate<G>> = nullptr>
constexpr auto operator |(S &&src, G &&gate)
{
    return make_src(std::forward<S>(src), std::forward<G>(gate));
}

template <class G1, class G2, enable_if<is_gate<G1, G2>> = nullptr>
constexpr auto operator |(G1 &&gate1, G2 &&gate2)
{
    return make_gate(std::forward<G1>(gate1), std::forward<G2>(gate2));
}

template <class G, class D, enable_if<is_gate<G>, is_drain<D>> = nullptr>
constexpr auto operator |(G &&gate, D &&drain)
{
    return make_drain(std::forward<G>(gate), std::forward<D>(drain));
}

template <class S, class D, enable_if<is_src<S>, is_drain<D>> = nullptr>
constexpr decltype(auto) operator |(S && src, D && drain) {
    return std::forward<D>(drain).OnComplete(std::forward<S>(src).Emit(drain));
}

} // namespace impl

} // namespace fet
