/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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
