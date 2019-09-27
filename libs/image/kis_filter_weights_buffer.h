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
#include "kis_filter_strategy.h"
#include "kis_debug.h"

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
                    dbgKrita << "*******";                              \
                    dbgKrita << "Non-symmetric zero found:" << centerSrc; \
                    dbgKrita << "Weight" << j << ":" << m_filterWeights[i].weight[j]; \
                    dbgKrita << "Weight" << idx2 << ":" << m_filterWeights[i].weight[idx2]; \
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
        dbgKrita << "************** i =" << i;                          \
        dbgKrita << ppVar(centerSrc);                                   \
        dbgKrita << ppVar(centerIndex);                                 \
        dbgKrita << ppVar(beginSrc) << ppVar(endSrc);                   \
        dbgKrita << ppVar(beginDst) << ppVar(endDst);                   \
        dbgKrita << ppVar(scaledIter) << ppVar(scaledInc);              \
        dbgKrita << ppVar(span);                                        \
        dbgKrita << "===";                                              \
    } while (0)

#define DEBUG_SAMPLE()                                                  \
    do {                                                                \
        dbgKrita << ppVar(scaledIter) << ppVar(t);                      \
    } while (0)
#else
#define DEBUG_ALL() Q_UNUSED(beginDst); Q_UNUSED(endDst)
#define DEBUG_SAMPLE()
#endif



/**
 * \class KisFilterWeightsBuffer
 *
 * Stores the cached values for the weights of neighbouring pixels
 * that would form the pixel in a result of resampling. The object of
 * this class is created before a pass of the transformation basing on
 * the desired scale factor and the filter strategy used for resampling.
 *
 * Here is an example of a calculation of the span for a pixel with
 * scale equal to 1.0. The result of the blending will be written into
 * the dst(0) pixel, which is marked with '*' sign. Note that all the
 * coordinates here are related to the center of the pixel, not to its
 * leftmost border as it is common in other systems. The centerSrc
 * coordinate represents the offset between the source and the
 * destination buffers.
 *
 * dst-coordinates: the coordinates in the resulting image. The values
 *                  of the filter strategy are calculated in these
 *                  coordinates.
 *
 * src-coordinates: the coordinates in the source image/buffer. We
 *                  pick integer values from there and calculate their
 *                  dst-position to know their weights.
 *
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
        Q_ASSERT(realScale > 0);

        m_filterWeights = new FilterWeights[256];
        m_maxSpan = 0;
        m_weightsPositionScale = 1;

        KisFixedPoint supportSrc;
        KisFixedPoint supportDst;

        if (realScale < 1.0) {
            m_weightsPositionScale = KisFixedPoint(realScale);
            supportSrc.from256Frac(filterStrategy->intSupport(m_weightsPositionScale.toFloat()) / realScale);
            supportDst.from256Frac(filterStrategy->intSupport(m_weightsPositionScale.toFloat()));

        } else {
            supportSrc.from256Frac(filterStrategy->intSupport(m_weightsPositionScale.toFloat()));
            supportDst.from256Frac(filterStrategy->intSupport(m_weightsPositionScale.toFloat()));
        }

        for (int i = 0; i < 256; i++) {
            KisFixedPoint centerSrc;
            centerSrc.from256Frac(i);

            KisFixedPoint beginDst = -supportDst;
            KisFixedPoint endDst = supportDst;

            KisFixedPoint beginSrc = -supportSrc - centerSrc / m_weightsPositionScale;
            KisFixedPoint endSrc = supportSrc - centerSrc / m_weightsPositionScale;

            int span = (2 * supportSrc).toInt() +
                (beginSrc.isInteger() && endSrc.isInteger());

            int centerIndex = -beginSrc.toInt();

            m_filterWeights[i].centerIndex = centerIndex;
            m_filterWeights[i].span = span;
            m_filterWeights[i].weight = new qint16[span];
            m_maxSpan = qMax(m_maxSpan, span);

            // in dst coordinate system:
            KisFixedPoint scaledIter = centerSrc + beginSrc.toInt() * m_weightsPositionScale;
            KisFixedPoint scaledInc = m_weightsPositionScale;

            DEBUG_ALL();

            int sum = 0;
            for (int j = 0; j < span; j++) {
                int t = filterStrategy->intValueAt(scaledIter.to256Frac(), m_weightsPositionScale.toFloat());
                m_filterWeights[i].weight[j] = t;
                sum += t;

                DEBUG_SAMPLE();
                SANITY_CENTER_POSITION();

                scaledIter += scaledInc;
            }

            SANITY_ZEROS();

            if (sum != 255 && sum > 0) {
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

    /**
     * Return a weights buffer for a particular value of offset
     */
    FilterWeights* weights(KisFixedPoint pos) const {
        return m_filterWeights + pos.to256Frac();
    }

    /**
     * The maximum width of the buffer that would be needed for
     * calculation of a pixel value. In other words, the maximum
     * number of support pixels that are needed for calculation of a
     * single result pixel
     */
    int maxSpan() const {
        return m_maxSpan;
    }

    /**
     * The scale of the support buffer. Note that it is not always
     * equal to the real scale of the transformation due to
     * interpolation/blending difference.
     */
    KisFixedPoint weightsPositionScale() const {
        return m_weightsPositionScale;
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
    KisFixedPoint m_weightsPositionScale;
};

#endif /* __KIS_FILTER_WEIGHTS_BUFFER_H */
