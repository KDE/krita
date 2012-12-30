/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_FILTER_WEIGHTS_BUFFER_H
#define __KIS_FILTER_WEIGHTS_BUFFER_H

#include "kis_fixed_point_maths.h"

#ifdef SANITY_CHECKS_ENABLED
static bool checkForAsymmetricZeros = false;

#define SANITY_CENTER_POSITION()                        \
    do {                                                \
        Q_ASSERT(scaledIter >= beginDst);               \
        Q_ASSERT(scaledIter <= endDst);                 \
                                                        \
        if (j == centerIndex) {                         \
            Q_ASSERT(scaledIter == centerSrc);          \
        }                                               \
    } while(0)

#define SANITY_ZEROS()                                                  \
    do {                                                                \
        if (checkForAsymmetricZeros) {                                  \
            for (int j = 0; j < span; j++) {                            \
                int idx2 = span - j - 1;                                \
                                                                        \
                if ((m_filterWeights[i].weight[j] && !m_filterWeights[i].weight[idx2]) || \
                    (!m_filterWeights[i].weight[j] && m_filterWeights[i].weight[idx2])) { \
                                                                        \
                    qDebug() << "*******";                              \
                    qDebug() << "Non-symmetric zero found:" << centerSrc; \
                    qDebug() << "Weight" << j << ":" << m_filterWeights[i].weight[j]; \
                    qDebug() << "Weight" << idx2 << ":" << m_filterWeights[i].weight[idx2]; \
                    qFatal("Non-symmetric zero -> fail");               \
                }                                                       \
            }                                                           \
        }                                                               \
    } while (0)

#define SANITY_CHECKSUM()                       \
    do {                                        \
        Q_ASSERT(sum == 255);                   \
    } while (0)

#else
#define SANITY_CENTER_POSITION()
#define SANITY_ZEROS()
#define SANITY_CHECKSUM()
#endif

#ifdef DEBUG_ENABLED
#define DEBUG_ALL()                                                     \
    do {                                                                \
        qDebug() << "************** i =" << i;                          \
        qDebug() << ppVar(centerSrc);                                   \
        qDebug() << ppVar(centerIndex);                                 \
        qDebug() << ppVar(beginSrc) << ppVar(endSrc);                   \
        qDebug() << ppVar(beginDst) << ppVar(endDst);                   \
        qDebug() << ppVar(scaledIter) << ppVar(scaledInc);              \
        qDebug() << ppVar(span);                                        \
        qDebug() << "===";                                              \
    } while (0)

#define DEBUG_SAMPLE()                                                  \
    do {                                                                \
        qDebug() << ppVar(scaledIter) << ppVar(t);                      \
    } while (0)
#else
#define DEBUG_ALL()
#define DEBUG_SAMPLE()
#endif



/**
 * Simple case with scale == 1.0
 *
 *                       +----+----+----+-- scaledIter (samples, measured in dst pixels,
 *                       |    |    |    |               correspond to integers in src)
 *
 *                              +---------+-- supportDst == filterStrategy->intSupport()
 *                              |         |
 *                    +-- beginDst        +-- endDst
 *                    |         |         |
 *                    |         +-- centerDst (always zero)
 *                    |         |         |
 *
 * dst: ----|----|----|----|----*----|----|----|----|----|-->
 *         -4   -3   -2   -1    0    1    2    3    4    5
 *
 * src: --|----|----|----|----|----|----|----|----|----|---->
 *       -4   -3   -2   -1    0    1    2    3    4    5
 *
 *                    ^         ^         ^
 *                    |         |         |
 *                    |         +-- centerSrc
 *                    |         |         |
 *                    +-- beginSrc        +endSrc
 *                    |         |         |
 *                    |         +---------+-- supportSrc ~= supportDst / realScale
 *                    |                   |
 *                    +-------------------+-- span (number of integers in the region)
 */

class KisFilterWeightsBuffer
{
public:
    struct FilterWeights {
        ~FilterWeights() {
            delete[] weight;
        }

        qint16 *weight;
        int span;
        int centerIndex;
    };

public:
    KisFilterWeightsBuffer(KisFilterStrategy *filterStrategy, qreal realScale) {
        m_filterWeights = new FilterWeights[256];
        m_maxSpan = 0;

        KisFixedPoint transformScale(1);

        KisFixedPoint supportSrc;
        KisFixedPoint supportDst;

        if (realScale < 1.0) {
            supportSrc.from256Frac(filterStrategy->intSupport() / realScale);
            supportDst.from256Frac(filterStrategy->intSupport());
            transformScale = KisFixedPoint(realScale);
        } else {
            supportSrc.from256Frac(filterStrategy->intSupport());
            supportDst.from256Frac(filterStrategy->intSupport());
        }

        for (int i = 0; i < 256; i++) {
            KisFixedPoint centerSrc;
            centerSrc.from256Frac(i);

            KisFixedPoint beginDst = -supportDst;
            KisFixedPoint endDst = supportDst;

            KisFixedPoint beginSrc = -supportSrc - centerSrc / transformScale;
            KisFixedPoint endSrc = supportSrc - centerSrc / transformScale;

            int span = (2 * supportSrc).toInt() +
                (beginSrc.isInteger() && endSrc.isInteger());

            int centerIndex = -beginSrc.toInt();

            m_filterWeights[i].centerIndex = centerIndex;
            m_filterWeights[i].span = span;
            m_filterWeights[i].weight = new qint16[span];
            m_maxSpan = qMax(m_maxSpan, span);

            // in dst coordinate system:
            KisFixedPoint scaledIter = centerSrc + beginSrc.toInt() * transformScale;
            KisFixedPoint scaledInc = transformScale;

            DEBUG_ALL();

            int sum = 0;
            for (int j = 0; j < span; j++) {
                int t = filterStrategy->intValueAt(scaledIter.to256Frac());
                m_filterWeights[i].weight[j] = t;
                sum += t;

                DEBUG_SAMPLE();
                SANITY_CENTER_POSITION();

                scaledIter += scaledInc;
            }

            SANITY_ZEROS();

            if (sum != 255) {
                qreal fixFactor = 255.0 / sum;
                sum = 0;

                for (int j = 0; j < span; j++) {
                    int t = qRound(m_filterWeights[i].weight[j] * fixFactor);

                    m_filterWeights[i].weight[j] = t;
                    sum += t;
                }
            }

            while (sum != 255) {
                int diff = sum < 255 ? 1 : -1;
                int index = findMaxIndex(m_filterWeights[i].weight, span);
                m_filterWeights[i].weight[index] += diff;
                sum += diff;
            }

            SANITY_CHECKSUM();
        }
    }

    ~KisFilterWeightsBuffer() {
        delete[] m_filterWeights;
    }

    FilterWeights* weights(KisFixedPoint pos) {
        return m_filterWeights + pos.to256Frac();
    }

private:
    int findMaxIndex(qint16 *buf, int size) {
        int maxValue = buf[0];
        int maxIndex = 0;

        for (int i = 1; i < size; i++) {
            if (buf[i] > maxValue) {
                maxValue = buf[i];
                maxIndex = i;
            }
        }

        return maxIndex;
    }

private:
    FilterWeights *m_filterWeights;
    int m_maxSpan;
};

#endif /* __KIS_FILTER_WEIGHTS_BUFFER_H */
