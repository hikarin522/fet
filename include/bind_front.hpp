#pragma once

#include <functional>
#include <type_traits>

// Simple implementation of bind_front for C++14 compatibility
// This is a simplified version of std::bind_front which was introduced in C++20

namespace std {

template<typename F, typename... Args>
auto bind_front(F&& f, Args&&... args) {
    return [f = std::forward<F>(f), args...](auto&&... rest) mutable {
        return f(args..., std::forward<decltype(rest)>(rest)...);
    };
}

} // namespace std