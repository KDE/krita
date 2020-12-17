/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_datamanager_test.h"

#include <QTest>
#include "kis_datamanager.h"

void KisDataManagerTest::testCreation()
{
    quint8 c = 0;
    KisDataManager test(1, &c);
}


void KisDataManagerTest::testDefaultPixel()
{
    {
        quint8 c = 0;
        KisDataManager dm(1, &c);
        QVERIFY(dm.pixelSize() == 1);
        QVERIFY(*dm.defaultPixel() == 0);
    }
    {
        quint8 * p = new quint8[3];
        memset(p, 0, 3);

        // The default pixel is copied, we still own the pointer
        KisDataManager dm(3, p);
        QVERIFY(dm.pixelSize() == 3);

        // A pointer, not a copy is returned. Changing it changes the
        // default pixel in the data manager.
        const quint8 * defaultPixelC = dm.defaultPixel();
        QVERIFY(defaultPixelC[0] == 0);
        QVERIFY(defaultPixelC[1] == 0);
        QVERIFY(defaultPixelC[2] == 0);

        // unconst it, so we can change it.
        quint8 * defaultPixel = const_cast<quint8*>(defaultPixelC);
        defaultPixel[0] = 50;
        defaultPixel[1] = 150;
        defaultPixel[2] = 200;

        // Check that our original pixel isn't changed
        QVERIFY(p[0] == 0);
        QVERIFY(p[1] == 0);
        QVERIFY(p[2] == 0);

        // Reset the default pixel
        dm.setDefaultPixel(p);
        defaultPixelC = dm.defaultPixel();
        QVERIFY(defaultPixelC[0] == 0);
        QVERIFY(defaultPixelC[1] == 0);
        QVERIFY(defaultPixelC[2] == 0);

        delete[]p;

    }
}

//void KisDataManagerTest::testMemento() {}
//
//void KisDataManagerTest::testReadWrite() {}
//
//void KisDataManagerTest::testExtent() {}
//
//void KisDataManagerTest::testClear() {}
//
//void KisDataManagerTest::testSetPixel() {}
//
//void KisDataManagerTest::testReadBytes() {}
//
//void KisDataManagerTest::testWriteBytes() {}
//
//void KisDataManagerTest::testPlanarBytes() {}
//
//void KisDataManagerTest::testContiguousColumns() {}
//
//void KisDataManagerTest::testRowStride() {}
//
//void KisDataManagerTest::testThreadedReadAccess() {}
//
//void KisDataManagerTest::testThreadedWriteAccess() {}
//
//void KisDataManagerTest::testThreadedReadWriteAccess() {}

QTEST_MAIN(KisDataManagerTest)
