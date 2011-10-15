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
#include <qtest_kde.h>

#include "kis_debug.h"

#include "tiles3/swap/kis_memory_window.h"

void KisMemoryWindowTest::testWindow()
{
    KisMemoryWindow memory(QString(), 1024);

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

QTEST_KDEMAIN(KisMemoryWindowTest, NoGUI)
#include "kis_memory_window_test.moc"

