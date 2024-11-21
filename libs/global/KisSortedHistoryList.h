/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSORTEDHISTORYLIST_H
#define KISSORTEDHISTORYLIST_H

#include <vector>
#include <KisHistoryList.h>

template <typename T>
class KisSortedHistoryList
{
public:
    using const_iterator = typename std::vector<T>::const_iterator;
    using compare_less = std::function<bool(const T&, const T&)>;

public:
    KisSortedHistoryList(int size)
        : m_list(size)
    {
    }

    /**
     * @return the number of elements in the list
     */
    int size() const {
        return m_list.size();
    }

    /**
     * @return @pos's element of the sorted history list
     */
    T at(int pos) const {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(pos < size(), T());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(pos >= 0, T());

        return m_sortedList.at(pos);
    }

    /**
     * Add an element to the history list
     *
     * @return the position of the inserted element in the sorted list
     */
    int append(const T& value) {
        int newElementIndex = m_list.append(value);;
        if (resortList()) {
            auto it = std::find(m_sortedList.begin(), m_sortedList.end(), value);
            KIS_SAFE_ASSERT_RECOVER(it != m_sortedList.end()) {
                it = m_sortedList.begin();
            }
            newElementIndex = std::distance(m_sortedList.begin(), it);
        }

        return newElementIndex;
    }

    /**
     * Clear all historical elements from the list
     */
    void clear() {
        m_list.clear();
        m_sortedList.clear();
    }

    /**
     * @return the maximum possible number of elements in the list
     */
    int maxSize() const {
        return m_list.maxSize();
    }

    /**
     * Set the comparison function for sorting the elements.
     *
     * Set to empty function `compare_less{}` to disable sorting completely
     */
    void setCompareLess(compare_less func) {
        m_compareLess = func;
        resortList();
    }

    const_iterator cbegin() const {
        return m_sortedList.cbegin();
    }

    const_iterator cend() const {
        return m_sortedList.cend();
    }

private:
    bool resortList() {
        m_sortedList.clear();
        m_sortedList.reserve(m_list.size());
        std::copy(m_list.cbegin(), m_list.cend(), std::back_inserter(m_sortedList));

        if (m_compareLess) {
            std::sort(m_sortedList.begin(), m_sortedList.end(), m_compareLess);
        }

        return bool(m_compareLess);
    }

private:
    KisHistoryList<T> m_list;
    std::vector<T> m_sortedList;
    compare_less m_compareLess;
};


#endif // KISSORTEDHISTORYLIST_H
