/*
 *  Copyright (c) 2010 Lukáš Tvrdý lukast.dev@gmail.com
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
#include <qtest_kde.h>

#include "kis_projection_benchmark.h"
#include "kis_benchmark_values.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <kis_group_layer.h>
#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>
#include <kis_doc2.h>
#include <kis_image.h>

void KisProjectionBenchmark::initTestCase()
{
    
}

void KisProjectionBenchmark::cleanupTestCase()
{
}


void KisProjectionBenchmark::benchmarkProjection()
{
    QBENCHMARK{
        KisDoc2 doc;
        doc.loadNativeFormat(QString(FILES_DATA_DIR) + QDir::separator() + "load_test.kra");
        doc.image()->refreshGraph();
        doc.saveNativeFormat(QString(FILES_OUTPUT_DIR) + QDir::separator() + "save_test.kra");
    }
}

#include "kis_wait_update_done.h"

void KisProjectionBenchmark::benchmarkOverlapping()
{
    KisDoc2 *doc = new KisDoc2();
    doc->loadNativeFormat(QString(FILES_DATA_DIR) + QDir::separator() + "load_test.kra");
    KisLayerSP rootLayer = doc->image()->rootLayer();
    KisNodeSP dirtyLayer = rootLayer->firstChild()->nextSibling()->firstChild();
    qint32 xShift = 100;
    qint32 yShift = 0;
    qint32 numShifts = 9;
    KisWaitUpdateDone waiter(doc->image(), numShifts);

    QBENCHMARK{
        waiter.startCollectingEvents();

        QRect dirtyRect(10, 10, 200,512);
        for(int i = 0; i < numShifts; i++) {
            // qDebug() << dirtyRect;
            dirtyLayer->setDirty(dirtyRect);
            dirtyRect.translate(xShift, yShift);
        }

        waiter.wait();
        waiter.stopCollectingEvents();
    }

    /**
     * The document must be deleted before the waiter to ensure
     * it's thread leaves waiter's slot
     */
    delete doc;
}



QTEST_KDEMAIN(KisProjectionBenchmark, GUI)
#include "kis_projection_benchmark.moc"
