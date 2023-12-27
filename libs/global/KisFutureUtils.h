/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFUTUREUTILS_H
#define KISFUTUREUTILS_H

namespace kismpl {

/**
 * Create a future whose value has already been evaluated
 *
 * See rejected C++ proposal for details:
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3721.pdf
 */
template <typename T>
std::future<std::decay_t<T>> make_ready_future(T &&value) {
    std::promise<T> promise;
    promise.set_value(std::forward<T>(value));
    return promise.get_future();
}

std::future<void> make_ready_future() {
    std::promise<void> promise;
    promise.set_value();
    return promise.get_future();
}

/**
 * Execute a given function \p func when the provided
 * future \p future is completed. The future is not
 * deferefenced outside the passed function to avoid
 * spilling the exceptions.
 *
 * See rejected C++ proposal for details:
 * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3721.pdf
 */
template<class T, class Function>
auto then(std::future<T>&& future, Function&& func)
    -> std::future<decltype(func(std::move(future)))>
{
    return std::async(std::launch::deferred,
        [](std::future<T>&& future, Function&& func)
        {
            future.wait();
            return std::forward<Function>(func)(std::move(future));
        },
        std::move(future),
        std::forward<Function>(func)
        );
}

}

#endif // KISFUTUREUTILS_H
