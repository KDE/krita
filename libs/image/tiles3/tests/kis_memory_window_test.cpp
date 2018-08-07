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

#include "kis_memory_window_test.h"
#include <QTest>

#include "kis_debug.h"
#include <QTemporaryDir>

#include "../swap/kis_memory_window.h"

void KisMemoryWindowTest::testWindow()
{
    QTemporaryDir swapDir;
    KisMemoryWindow memory(swapDir.path(), 1024);

    quint8 oddValue = 0xee;
    const quint8 chunkLength = 10;

    quint8 oddBuf[chunkLength];
    memset(oddBuf, oddValue, chunkLength);


    KisChunkData chunk1(0, chunkLength);
    KisChunkData chunk2(1025, chunkLength);

    quint8 *ptr;

    ptr = memory.getWriteChunkPtr(chunk1);
    memcpy(ptr, oddBuf, chunkLength);

    ptr = memory.getWriteChunkPtr(chunk2);
    memcpy(ptr, oddBuf, chunkLength);

    ptr = memory.getReadChunkPtr(chunk2);
    QVERIFY(!memcmp(ptr, oddBuf, chunkLength));

    ptr = memory.getWriteChunkPtr(chunk1);
    QVERIFY(!memcmp(ptr, oddBuf, chunkLength));
}

void KisMemoryWindowTest::testTopReports()
{

    // default window size in 16 MiB
    KisMemoryWindow memory(QString(QDir::currentPath()), DEFAULT_WINDOW_SIZE);

    // write 1024 chunks 4 MiB each, hi-limit 4GiB

    const quint8 oddValue = 0xee;
    const qint64 chunkLength = 4 * MiB;

    QScopedArrayPointer<quint8> writeBuffer(new quint8[chunkLength]);
    memset(writeBuffer.data(), oddValue, chunkLength);

    QScopedArrayPointer<quint8> readBuffer(new quint8[chunkLength]);

    qint64 maxChunk = 0;

    for (int i = 0; i < 1024; i++) {
        {
            int chunkIndex = qrand() % 1024;

            qint64 chunkStart = chunkIndex * chunkLength;
            maxChunk = qMax(chunkStart, maxChunk);

            quint8 *ptr;
            ptr = memory.getWriteChunkPtr(KisChunkData(chunkStart, chunkLength));
            memcpy(ptr, writeBuffer.data(), chunkLength);

            dbgKrita << "Writing chunk at" << chunkStart / chunkLength << "MiB" << "max" << maxChunk / chunkLength;
            QTest::qWait(250);
        }

        {
            int chunkIndex = qrand() % 1024;

            qint64 chunkStart = chunkIndex * chunkLength;

            quint8 *ptr;
            ptr = memory.getReadChunkPtr(KisChunkData(chunkStart, chunkLength));
            memcpy(readBuffer.data(), ptr, chunkLength);

            dbgKrita << "Reading chunk at" << chunkStart / chunkLength << "MiB" << "max" << maxChunk / chunkLength;
            QTest::qWait(250);
        }
    }
}

QTEST_MAIN(KisMemoryWindowTest)

