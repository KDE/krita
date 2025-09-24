/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISADAPTEDLOCK_H
#define KISADAPTEDLOCK_H

#include <mutex>

/**
 * A wrapper class that adapts std::unique_lock to any kind
 * of locking that might be necessary to a particular class.
 *
 * Just define an Adapter class that implements `lock()`,
 * `unlock()` and (optionally) `try_lock()` interface and
 * pass it to `KisAdaptedLock`. The resulting class will
 * behave as normal `std::unique_lock` and lock/unlock the
 * object as you instructed it.
 *
 * See examples in `KisCursorOverrideLockAdapter` and
 * `KisLockFrameGenerationLock`
 */
template <typename Adapter>
class KisAdaptedLock
    : protected Adapter,
      public std::unique_lock<Adapter>
{
public:
    template<typename Object>
    KisAdaptedLock(Object object)
        : Adapter(object)
        , std::unique_lock<Adapter>(
              static_cast<Adapter&>(*this))
    {}

    template<typename Object>
    KisAdaptedLock(Object object, std::try_to_lock_t t)
        : Adapter(object)
        , std::unique_lock<Adapter>(static_cast<Adapter&>(*this), t)
    {}

    template<typename Object>
    KisAdaptedLock(Object object, std::defer_lock_t t)
        : Adapter(object)
        , std::unique_lock<Adapter>(static_cast<Adapter&>(*this), t)
    {}

    template<typename Object>
    KisAdaptedLock(Object object, std::adopt_lock_t t)
        : Adapter(object)
        , std::unique_lock<Adapter>(static_cast<Adapter&>(*this), t)
    {}

    KisAdaptedLock(KisAdaptedLock &&rhs)
        : Adapter(static_cast<Adapter&>(rhs))
        , std::unique_lock<Adapter>(
              static_cast<Adapter&>(*this), std::adopt_lock)
    {
        rhs.release();
    }

    KisAdaptedLock& operator=(KisAdaptedLock &&rhs)
    {
        static_cast<Adapter&>(*this) = rhs;
        static_cast<std::unique_lock<Adapter>&>(*this) =
            std::unique_lock<Adapter>(static_cast<Adapter&>(*this),
                                      std::adopt_lock);
        rhs.release();
        return *this;
    }

    using std::unique_lock<Adapter>::try_lock;
    using std::unique_lock<Adapter>::lock;
    using std::unique_lock<Adapter>::unlock;
    using std::unique_lock<Adapter>::owns_lock;
};

/**
 * A macro to make sure that the resulting lock is
 * a 'class' and can be forward-declared instead of
 * the entire include pulling
 */
#define KIS_DECLARE_ADAPTED_LOCK(Name, Adapter) \
class Name : public KisAdaptedLock<Adapter>     \
{                                               \
    public:                                     \
    using BaseClass = KisAdaptedLock<Adapter>;  \
    using BaseClass::BaseClass;                 \
};

#endif // KISADAPTEDLOCK_H
