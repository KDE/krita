/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisMagneticWorkerTest.h"

#include <KisMagneticWorker.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <testutil.h>
#include <kis_paint_device_debug_utils.h>

#include <QDebug>

inline KisPaintDeviceSP loadTestImage(const QString &name, bool convertToAlpha)
{
    QImage image(TestUtil::fetchDataFileLazy(name));
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(image, 0);

    if (convertToAlpha) {
        dev = KisPainter::convertToAlphaAsAlpha(dev);
    }

    return dev;
}

void KisMagneticWorkerTest::testWorker()
{
    KisPaintDeviceSP dev = loadTestImage("test_main.png", false);
    const QRect rect = dev->exactBounds();
    KisPaintDeviceSP grayscaleDev = KisPainter::convertToAlphaAsGray(dev);
    KisMagneticWorker worker(grayscaleDev);
    KIS_DUMP_DEVICE_2(dev, rect, "main", "dd");

    const QPoint startPos(40, 10);
    const QPoint endPos(50, 65);


    auto points = worker.computeEdge(10, startPos, endPos);
    KIS_DUMP_DEVICE_2(grayscaleDev, rect, "draw", "dd");

    QVector<QPointF> result = { QPointF(50,65),
                                QPointF(49,64),
                                QPointF(48,63),
                                QPointF(47,62),
                                QPointF(46,61),
                                QPointF(45,60),
                                QPointF(44,59),
                                QPointF(44,58),
                                QPointF(44,57),
                                QPointF(44,56),
                                QPointF(44,55),
                                QPointF(44,54),
                                QPointF(44,53),
                                QPointF(44,52),
                                QPointF(44,51),
                                QPointF(44,50),
                                QPointF(44,49),
                                QPointF(44,48),
                                QPointF(44,47),
                                QPointF(44,46),
                                QPointF(44,45),
                                QPointF(44,44),
                                QPointF(44,43),
                                QPointF(44,42),
                                QPointF(43,41),
                                QPointF(43,40),
                                QPointF(43,39),
                                QPointF(44,38),
                                QPointF(44,37),
                                QPointF(44,36),
                                QPointF(44,35),
                                QPointF(44,34),
                                QPointF(44,33),
                                QPointF(44,32),
                                QPointF(44,31),
                                QPointF(44,30),
                                QPointF(44,29),
                                QPointF(44,28),
                                QPointF(44,27),
                                QPointF(44,26),
                                QPointF(44,25),
                                QPointF(44,24),
                                QPointF(44,23),
                                QPointF(44,22),
                                QPointF(44,21),
                                QPointF(44,20),
                                QPointF(44,19),
                                QPointF(44,18),
                                QPointF(43,17),
                                QPointF(44,16),
                                QPointF(44,15),
                                QPointF(44,14),
                                QPointF(44,13),
                                QPointF(43,12),
                                QPointF(42,11),
                                QPointF(41,11),
                                QPointF(40,10)};

    QCOMPARE(result, points);
}

QTEST_MAIN(KisMagneticWorkerTest)
