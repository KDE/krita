/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_UPDATE_SCHEDULER_TEST_H
#define KIS_UPDATE_SCHEDULER_TEST_H

#include <QtTest/QtTest>

#include "kis_types.h"

class KisUpdateSchedulerTest : public QObject
{
    Q_OBJECT

private:
    KisImageSP buildTestingImage();

private slots:
    void testMerge();
    void benchmarkOverlappedMerge();
    void testLocking();
    void testExclusiveStrokes();
    void testEmptyStroke();
    void testLazyWaitCondition();
    void testBlockUpdates();
};

#endif /* KIS_UPDATE_SCHEDULER_TEST_H */

