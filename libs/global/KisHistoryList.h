/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISHISTORYLIST_H
#define KISHISTORYLIST_H

#include "kis_debug.h"
#include "kis_assert.h"
#include <deque>

template <typename T>
class KisHistoryList
{
public:
    using const_iterator = typename std::deque<T>::const_iterator;

public:
    KisHistoryList(int size)
        : m_maxSize(size)
    {
    }

    /**
     * @return the number of elements in the list
     */
    int size() const {
        return m_values.size();
    }

    /**
     * @return @pos's element of the history list
     */
    T at(int pos) const {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(pos < size(), T());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(pos >= 0, T());

        return m_values.at(pos);
    }

    /**
     * Add an element to the history list
     *
     * If an element already exists in the list, then this
     * existing element is put into the top of the list (as
     * recently used).
     *
     * If the number of elements exceeds the maxSize() value,
     * the oldest element of the history list is removed.
     */
    int append(const T& value) {
        auto it = std::find(m_values.begin(), m_values.end(), value);

        if (it != m_values.end()) {
            std::rotate(m_values.begin(), it, std::next(it));
        } else {
            m_values.push_front(value);
            if (int(m_values.size()) > m_maxSize) {
                m_values.pop_back();
            }
        }

        // return the index of just added color (always 0)
        return 0;
    }

    /**
     * Clear all historical elements from the list
     */
    void clear() {
        m_values.clear();
    }

    /**
     * @return the maximum possible number of elements in the list
     */
    int maxSize() const {
        return m_maxSize;
    }

    const_iterator cbegin() const {
        return m_values.cbegin();
    }

    const_iterator cend() const {
        return m_values.cend();
    }


private:
    int m_maxSize = 0;
    std::deque<T> m_values;
};

#endif // KISHISTORYLIST_H
