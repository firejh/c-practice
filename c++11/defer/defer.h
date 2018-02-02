// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#pragma once

#ifndef __DEFER_H__
#define __DEFER_H__

#include <functional>

// auto __defer_action_line_???__ = __Defer()- []{ ... };
#define defer auto __DEFER_ACTION_VAR(__defer_action_line_, __LINE__, __) = ::__Defer()-
#define __DEFER_ACTION_VAR(a, b, c) __DEFER_TOKEN_CONNECT(a, b, c)
#define __DEFER_TOKEN_CONNECT(a, b, c) a ## b ## c

// Hold defered func
struct __DeferredAction {
    std::function<void()> func_;

    template<typename T>
    __DeferredAction(T&& p): func_(std::function<void()>(p)) {}

    __DeferredAction();
    __DeferredAction(__DeferredAction const&);
    __DeferredAction& operator=(__DeferredAction const&);
    __DeferredAction& operator=(__DeferredAction&&);

    __DeferredAction(__DeferredAction&& other):
        func_(std::forward<std::function<void()>>(other.func_)) {
        other.func_ = nullptr;
    }
    ~__DeferredAction() {
        if(func_) { func_(); }
    }
};

template<typename T>
__DeferredAction __DeferredActionCtor(T&& p) {
    return __DeferredAction(std::forward<T>(p));
}

struct __Defer {
    template <typename Arg>
    inline __DeferredAction operator-(Arg const& arg) {
        return __DeferredActionCtor(arg);
    }
};

#endif  // __DEFER_H__

