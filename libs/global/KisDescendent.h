/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DESCENDENT_H_
#define KIS_DESCENDENT_H_

#include <memory>
#include <utility>

/**
 * KisDescendent<T> holds an instance of any subclass U of T.
 *
 * Its copy/move constructor will call the respective constructor in class U.
 * Same for assignments.
 *
 * After moving, the original KisDescendent will be invalidated.
 *
 * Example: (suppose Derived is a subclass of Base)
 *     KisDescendent<Base> ins1 = Derived();
 *     ins1->someMethod();
 *     KisDescendent<Base> ins2 = ins1; // ins2 is a clone of ins1, created by Derived's copy constructor
 *     KisDescendent<Base> ins3 = std::move(ins2); // move-constructs ins3; ins2 is now invalidated.
 */
template<typename T>
class KisDescendent
{
    struct concept
    {
        virtual ~concept() = default;
        virtual const T *ptr() const = 0;
        virtual T *ptr() = 0;
        virtual std::unique_ptr<concept> clone() const = 0;
    };

    template<typename U>
    struct model : public concept
    {
        model(U x) : instance(std::move(x)) {}
        const T *ptr() const { return &instance; }
        T *ptr() { return &instance; }
        std::unique_ptr<concept> clone() const { return std::unique_ptr<model<U> >(new model<U>(U(instance))); }
        U instance;
    };

    std::unique_ptr<concept> m_d;
public:
    template<typename U>
    KisDescendent(U x) : m_d(std::unique_ptr<model<U> >(new model<U>(std::move(x)))) {}

    /**
     * Copy-constructs the instance stored by that. The constructor of type U will be used.
     */
    KisDescendent(const KisDescendent &that) : m_d(std::move(that.m_d->clone())) {}
    /**
     * Transfers the ownership of the instance from that to this.
     * This will NOT move-construct any instance of U nor T.
     */
    KisDescendent(KisDescendent &&that) : m_d(std::move(that.m_d)) {}

    KisDescendent & operator=(const KisDescendent &that) { Descendent t(that); *this = std::move(t); return *this; }
    KisDescendent & operator=(KisDescendent &&that) { m_d = std::move(that.m_d); return *this; }

    /**
     * Returns a const pointer to the stored instance. The pointer can be cast to const U*.
     */
    const T *data() const { return m_d->ptr(); }
    const T *constData() const { return m_d->ptr(); }
    /**
     * Returns a non-const pointer to the stored instance. The pointer can be cast to U*.
     */
    T *data() { return m_d->ptr(); }
    const T *operator->() const { return m_d->ptr(); }
    T *operator->() { return m_d->ptr(); }
};

#endif // KIS_DESCENDENT_H_
