/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SCANLINE_FILL_TEST_H
#define __KIS_SCANLINE_FILL_TEST_H

#include <simpletest.h>

class QColor;
class KisFillInterval;


class KisScanlineFillTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSimpleFill();

    void testFillBackwardCollisionOnTheLeft();
    void testFillBackwardCollisionOnTheRight();
    void testFillBackwardCollisionFull();
    void testFillBackwardCollisionSanityCheck();

    void testClearNonZeroComponent();
    void testExternalFill();

private:
    void testFillGeneral(const QVector<KisFillInterval> &initialBackwardIntervals,
                         const QVector<QColor> &expectedResult,
                         const QVector<KisFillInterval> &expectedForwardIntervals,
                         const QVector<KisFillInterval> &expectedBackwardIntervals);
};

#endif /* __KIS_SCANLINE_FILL_TEST_H */
