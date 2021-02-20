/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DATAMANAGER_TEST_H
#define KIS_DATAMANAGER_TEST_H

#include <QtTest>

class KisDataManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();
    void testDefaultPixel();
//    void testMemento();
//    void testReadWrite();
//    void testExtent();
//    void testClear();
//    void testSetPixel();
//    void testReadBytes();
//    void testWriteBytes();
//    void testPlanarBytes();
//    void testContiguousColumns();
//    void testRowStride();
//    void testThreadedReadAccess();
//    void testThreadedWriteAccess();
//    void testThreadedReadWriteAccess();

private:
    bool memoryIsFilled(quint8 c, quint8 *mem, qint32 size);
};

#endif
