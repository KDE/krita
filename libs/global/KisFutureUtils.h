/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFUTUREUTILS_H
#define KISFUTUREUTILS_H

namespace kismpl {

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
