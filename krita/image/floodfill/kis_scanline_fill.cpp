/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_scanline_fill.h"

#include <QStack>
#include "kis_fill_interval_map.h"
#include "kis_paint_device.h"
#include "kis_random_accessor_ng.h"
#include "kis_fill_sanity_checks.h"


struct KisScanlineFill::Private
{
    KisPaintDeviceSP device;
    KisRandomAccessorSP it;
    QPoint startPoint;
    QRect boundingRect;

    int rowIncrement;
    KisFillIntervalMap backwardMap;
    QStack<KisFillInterval> forwardStack;


    void swapDirection() {
        rowIncrement *= -1;
        SANITY_ASSERT_MSG(forwardStack.isEmpty(),
                          "FATAL: the forward stack must be empty "
                          "on a direction swap");

        forwardStack = QStack<KisFillInterval>(backwardMap.fetchAllIntervals(rowIncrement));
        backwardMap.clear();
    }
};


KisScanlineFill::KisScanlineFill(KisPaintDeviceSP device, const QPoint &startPoint, const QRect &boundingRect)
    : m_d(new Private)
{
    m_d->device = device;
    m_d->it = device->createRandomAccessorNG(startPoint.x(), startPoint.y());
    m_d->startPoint = startPoint;
    m_d->boundingRect = boundingRect;

    m_d->rowIncrement = 1;
}

KisScanlineFill::~KisScanlineFill()
{
}

inline quint8 selectionValue(quint8 *srcPtr)
{
    if (srcPtr[0] < 128 &&
        srcPtr[1] < 128 &&
        srcPtr[2] < 128) return 255;

    return 0;
}

inline void fillPixel(quint8 *dstPtr, quint8 opacity)
{
    Q_UNUSED(opacity);

    dstPtr[0] = 200;
    dstPtr[1] = 200;
    dstPtr[2] = 200;
    dstPtr[3] = 200;
}

void KisScanlineFill::extendedPass(KisFillInterval *currentInterval, int srcRow, bool extendRight)
{
    int x;
    int endX;
    int columnIncrement;
    int *intervalBorder;
    int *backwardIntervalBorder;
    KisFillInterval backwardInterval(currentInterval->start, currentInterval->end, srcRow);

    if (extendRight) {
        x = currentInterval->end;
        endX = m_d->boundingRect.right();
        if (x >= endX) return;
        columnIncrement = 1;
        intervalBorder = &currentInterval->end;

        backwardInterval.start = currentInterval->end + 1;
        backwardIntervalBorder = &backwardInterval.end;
    } else {
        x = currentInterval->start;
        endX = m_d->boundingRect.left();
        if (x <= endX) return;
        columnIncrement = -1;
        intervalBorder = &currentInterval->start;

        backwardInterval.end = currentInterval->start - 1;
        backwardIntervalBorder = &backwardInterval.start;
    }

    do {
        x += columnIncrement;

        m_d->it->moveTo(x, srcRow);
        quint8 *pixelPtr = m_d->it->rawData();
        quint8 opacity = selectionValue(pixelPtr);

        if (opacity) {
            *intervalBorder = x;
            *backwardIntervalBorder = x;
            fillPixel(pixelPtr, opacity);
        } else {
            break;
        }
    } while (x != endX);

    if (backwardInterval.isValid()) {
        m_d->backwardMap.insertInterval(backwardInterval);
    }
}

void KisScanlineFill::processLine(KisFillInterval interval, const int rowIncrement)
{
    m_d->backwardMap.cropInterval(&interval);

    if (!interval.isValid()) return;

    int firstX = interval.start;
    int lastX = interval.end;
    int x = firstX;
    int row = interval.row;
    int nextRow = row + rowIncrement;

    KisFillInterval currentForwardInterval;

    while(x <= lastX) {
        m_d->it->moveTo(x, row);
        quint8 *pixelPtr = m_d->it->rawData();
        quint8 opacity = selectionValue(pixelPtr);

        if (opacity) {
            if (!currentForwardInterval.isValid()) {
                currentForwardInterval.start = x;
                currentForwardInterval.end = x;
                currentForwardInterval.row = nextRow;
            } else {
                currentForwardInterval.end = x;
            }

            fillPixel(pixelPtr, opacity);

            if (x == firstX) {
                extendedPass(&currentForwardInterval, row, false);
            }

            if (x == lastX) {
                extendedPass(&currentForwardInterval, row, true);
            }

        } else {
            if (currentForwardInterval.isValid()) {
                m_d->forwardStack.push(currentForwardInterval);
                currentForwardInterval.invalidate();
            }
        }

        x++;
    }

    if (currentForwardInterval.isValid()) {
        m_d->forwardStack.push(currentForwardInterval);
    }
}

void KisScanlineFill::run()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->forwardStack.isEmpty());
    m_d->forwardStack.push(KisFillInterval(m_d->startPoint.x(), m_d->startPoint.x(), m_d->startPoint.y()));

    while (!m_d->forwardStack.isEmpty()) {
        while (!m_d->forwardStack.isEmpty()) {
            KisFillInterval interval = m_d->forwardStack.pop();

            if (interval.row > m_d->boundingRect.bottom() ||
                interval.row < m_d->boundingRect.top()) {

                continue;
            }

            processLine(interval, m_d->rowIncrement);
        }
        m_d->swapDirection();
    }
}

QVector<KisFillInterval> KisScanlineFill::testingGetForwardIntervals() const
{
    return QVector<KisFillInterval>(m_d->forwardStack);
}

KisFillIntervalMap* KisScanlineFill::testingGetBackwardIntervals() const
{
    return &m_d->backwardMap;
}
