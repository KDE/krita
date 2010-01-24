/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_datamanager_benchmark.h"

#include <qtest_kde.h>
#include <kis_datamanager.h>

void KisDatamanagerBenchmark::initTestCase()
{
    // To make sure all the first-time startup costs are done
    quint8 * p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);
}

void KisDatamanagerBenchmark::benchmarkCreation()
{
    // tests the cost of creating a new datamanager

    QBENCHMARK {
        quint8 * p = new quint8[3];
        memset(p, 0, 3);
        KisDataManager dm(3, p);
    }
}

void KisDatamanagerBenchmark::benchmarkWriteBytes()
{
    quint8 *p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);

    quint8 *bytes = new quint8[3 * 1024 * 1024];
    memset(bytes, 0, 3 * 1024 * 1024);

    QBENCHMARK {
        dm.writeBytes(bytes, 0, 0, 1024, 1024);
    }

    delete[] bytes;
}

void KisDatamanagerBenchmark::benchmarkReadBytes()
{
    quint8 *p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);

    quint8 *bytes = new quint8[3 * 1024 * 1024];
    memset(bytes, 0, 3 * 1024 * 1024);

    QBENCHMARK {
        dm.readBytes(bytes, 0, 0, 1024, 1024);
    }

    delete[] bytes;
}


void KisDatamanagerBenchmark::benchmarkReadWriteBytes()
{
    quint8 *p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);

    quint8 *bytes = new quint8[3 * 1024 * 1024];
    memset(bytes, 0, 3 * 1024 * 1024);

    QBENCHMARK {
        dm.readBytes(bytes, 0, 0, 1024, 1024);
        dm.writeBytes(bytes, 0, 0, 1024, 1024);
    }

    delete[] bytes;
}

void KisDatamanagerBenchmark::benchmarkExtent()
{
    quint8 *p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);
    quint8 *bytes = new quint8[3 * 1021 * 1084];
    memset(bytes, 0, 3 * 1021 * 1084);
    dm.writeBytes(bytes, 0, 0, 1021, 1084);
    QBENCHMARK {
        QRect extent = dm.extent();
    }
}

QTEST_KDEMAIN(KisDatamanagerBenchmark, GUI)
#include "kis_datamanager_benchmark.moc"
