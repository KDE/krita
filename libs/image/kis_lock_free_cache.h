/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        bool result = seq == m_value;
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
        T savedValue;

        if (m_state.startRead(&seqValue)) {
            savedValue = m_value;
            isValid = m_state.endRead(seqValue);
        }

        if (isValid) {
            return savedValue;
        } else if (m_state.startWrite(&seqValue)) {
            savedValue = calculateNewValue();
            m_value = savedValue;
            m_state.endWrite(seqValue);
            return savedValue;
        } else {
            return calculateNewValue();
        }
    }

    bool tryGetValue(T &result) const {
        KisCacheStateValue::SeqValue seqValue;
        bool isValid = false;
        T savedValue;

        if (m_state.startRead(&seqValue)) {
            savedValue = m_value;
            isValid = m_state.endRead(seqValue);
        }

        if (isValid) {
            result = savedValue;
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

template <typename T, typename Mode>
class KisLockFreeCacheWithModeConsistency
{
public:
    virtual ~KisLockFreeCacheWithModeConsistency()
    {
    }

    /**
     * Notify the cache that the value has changed
     */
    void invalidate() {
        m_state.invalidate();
    }

    /**
     * Calculate the value or fetch it from the cache
     */
    T getValue(Mode mode) const {
        KisCacheStateValue::SeqValue seqValue;
        bool isValid = false;
        T savedValue;
        Mode savedMode;

        if (m_state.startRead(&seqValue)) {
            savedValue = m_value;
            savedMode = m_mode;
            isValid = m_state.endRead(seqValue);
            isValid &= savedMode == mode;
        }

        if (isValid) {
            return savedValue;
        } else if (m_state.startWrite(&seqValue)) {
            savedValue = calculateNewValue();
            m_value = savedValue;
            m_mode = mode;
            m_state.endWrite(seqValue);
            return savedValue;
        } else {
            return calculateNewValue();
        }
    }

    bool tryGetValue(T &result, Mode mode) const {
        KisCacheStateValue::SeqValue seqValue;
        bool isValid = false;
        T newValue;
        Mode savedMode;

        if (m_state.startRead(&seqValue)) {
            newValue = m_value;
            savedMode = m_mode;
            isValid = m_state.endRead(seqValue);
            isValid &= savedMode == mode;
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
    mutable Mode m_mode;
};



#endif /* __KIS_CACHE_STATE_VALUE_H */
