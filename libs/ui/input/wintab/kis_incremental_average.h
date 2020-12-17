/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_INCREMENTAL_AVERAGE_H
#define __KIS_INCREMENTAL_AVERAGE_H

#include <kis_debug.h>
#include <QVector>


class KisIncrementalAverage
{
public:
    KisIncrementalAverage(int size) 
        : m_size(size),
          m_index(-1),
          m_sum(0),
          m_values(size)
    {
    }

    inline int pushThrough(int value) {
        if (m_index < 0) {
            for (int i = 0; i < m_size; i++) {
                m_values[i] = value;
            }
            m_index = 0;
            m_sum = 3 * value;
            return value;
        }

        int oldValue = m_values[m_index];
        m_values[m_index] = value;

        m_sum += value - oldValue;

        if (++m_index >= m_size) {
            m_index = 0;
        }

        return m_sum / m_size;
    }

private:
    int m_size;
    int m_index;
    int m_sum;
    QVector<int> m_values;
};

#endif /* __KIS_INCREMENTAL_AVERAGE_H */
