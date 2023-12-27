/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisFilteredRollingMean.h"

#include <algorithm>
#include <numeric>
#include <QtMath>
#include "kis_assert.h"
#include "kis_debug.h"


KisFilteredRollingMean::KisFilteredRollingMean(int windowSize, qreal effectivePortion)
    : m_values(windowSize),
      m_rollingSum(0.0),
      m_effectivePortion(effectivePortion),
      m_cutOffBuffer(qCeil(0.5 * (qCeil(windowSize * (1.0 - effectivePortion)))))
{
}

void KisFilteredRollingMean::addValue(qreal value)
{
    if (m_values.full()) {
        m_rollingSum -= m_values.front();
    }

    m_values.push_back(value);
    m_rollingSum += value;
}

qreal KisFilteredRollingMean::filteredMean() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_values.empty(), 0.0);

    const int usefulElements = qMax(1, qRound(m_effectivePortion * m_values.size()));

    qreal sum = 0.0;
    int num = 0;

    const int cutOffTotal = m_values.size() - usefulElements;

    if (cutOffTotal > 0) {
        const std::vector<double>::size_type cutMin = qRound(0.5 * cutOffTotal);
        const std::vector<double>::size_type cutMax = cutOffTotal - cutMin;

        KIS_SAFE_ASSERT_RECOVER(cutMin <= m_cutOffBuffer.size()) {
            m_cutOffBuffer.resize(cutMin);
        }
        KIS_SAFE_ASSERT_RECOVER(cutMax <= m_cutOffBuffer.size()) {
            m_cutOffBuffer.resize(cutMax);
        }

        sum = m_rollingSum;
        num = usefulElements;

        std::partial_sort_copy(m_values.begin(),
                               m_values.end(),
                               m_cutOffBuffer.begin(),
                               m_cutOffBuffer.begin() + cutMin);

        sum -= std::accumulate(m_cutOffBuffer.begin(),
                               m_cutOffBuffer.begin() + cutMin,
                               0.0);

        std::partial_sort_copy(m_values.begin(),
                               m_values.end(),
                               m_cutOffBuffer.begin(),
                               m_cutOffBuffer.begin() + cutMax,
                               std::greater<qreal>());

        sum -= std::accumulate(m_cutOffBuffer.begin(),
                               m_cutOffBuffer.begin() + cutMax,
                               0.0);
    } else {
        sum = m_rollingSum;
        num = m_values.size();
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(num > 0, 0.0);

    return sum / num;
}

bool KisFilteredRollingMean::isEmpty() const
{
    return m_values.empty();
}
