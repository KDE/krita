/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_CACHE_STATE_VALUE_H
#define __KIS_CACHE_STATE_VALUE_H

#include <QAtomicInt>

/**
 * This class implements a state variable for a lockfree cache object.
 * The implementation is actually a variation of a 'seqlock', but with
 * a simple modification: the values of "cache validity", "number of
 * writers" and "sequence number" are multiplexed in a single int
 * value.
 */
class KisCacheStateValue
{
    static const int WritersCountMask = 0x00FF;
    static const int WritersCountIncrement = 0x0001;
    static const int IsValidMask = 0x0100;
    static const int SeqNoMask = ~(WritersCountMask | IsValidMask);
    static const int SeqNoIncrement = 0x0200;
public:
    typedef int SeqValue;
public:
    inline void invalidate() {
        int oldValue;
        int newValue = -1;
        do {
            oldValue = m_value;
            newValue = incrementSeqNo(oldValue) & ~IsValidMask;
        } while(!m_value.testAndSetOrdered(oldValue, newValue));
    }

    inline bool startRead(int *seq) const {
        *seq = m_value;

        return (*seq & IsValidMask) &&
            !(*seq & WritersCountMask);
    }

    inline bool endRead(int seq) const {
        bool result =
            seq == m_value &&
            (seq & IsValidMask) &&
            !(seq & WritersCountMask);

        return result;
    }


    inline bool startWrite(int *seq) {
        int oldValue;
        int newValue;
        do {
            oldValue = m_value;
            if ((oldValue & IsValidMask) ||
                (oldValue & WritersCountMask)) {

                return false;
            }
            newValue = incrementSeqNo(oldValue) + WritersCountIncrement;
        } while(!m_value.testAndSetOrdered(oldValue, newValue));

        *seq = newValue;
        return true;
    }

    inline void endWrite(int seq) {
        int oldValue;
        int newValue;
        do {
            oldValue = m_value;

            if (oldValue == seq) {
                newValue = (incrementSeqNo(oldValue) - WritersCountIncrement) | IsValidMask;
            } else {
                newValue = (incrementSeqNo(oldValue) - WritersCountIncrement) & ~IsValidMask;
            }
        } while(!m_value.testAndSetOrdered(oldValue, newValue));
    }

private:
    int incrementSeqNo(int value) {
        // handle overflow properly
        if ((value & SeqNoMask) == SeqNoMask) {
            value = value & ~SeqNoMask;
        } else {
            value += SeqNoIncrement;
        }
        return value;
    }

private:
    QAtomicInt m_value;
};

template <typename T>
class KisLockFreeCache
{
public:
    virtual ~KisLockFreeCache() {}

    /**
     * Notify the cache that the value has changed
     */
    void invalidate() {
        m_state.invalidate();
    }

    /**
     * Calculate the value or fetch it from the cache
     */
    T getValue() const {
        KisCacheStateValue::SeqValue seqValue;
        bool isValid = false;
        T newValue;

        if (m_state.startRead(&seqValue)) {
            newValue = m_value;
            isValid = m_state.endRead(seqValue);
        }

        if (isValid) {
            return newValue;
        } else if (m_state.startWrite(&seqValue)) {
            newValue = calculateNewValue();
            m_value = newValue;
            m_state.endWrite(seqValue);
            return newValue;
        } else {
            return calculateNewValue();
        }
    }

    bool tryGetValue(T &result) const {
        KisCacheStateValue::SeqValue seqValue;
        bool isValid = false;
        T newValue;

        if (m_state.startRead(&seqValue)) {
            newValue = m_value;
            isValid = m_state.endRead(seqValue);
        }

        if (isValid) {
            result = newValue;
        }

        return isValid;
    }

protected:
    /**
     * Calculate the value. Used by the cache
     * internally. Reimplemented by the user.
     */
    virtual T calculateNewValue() const = 0;

private:
    mutable KisCacheStateValue m_state;
    mutable T m_value;
};



#endif /* __KIS_CACHE_STATE_VALUE_H */
