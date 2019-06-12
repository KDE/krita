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
    KisMagneticWorker worker;
    KIS_DUMP_DEVICE_2(dev, rect, "main", "dd");

    const QPointF startPos(30, 10);
    const QPointF endPos(50, 65);


    auto points = worker.computeEdge(grayscaleDev, 10, QPoint(30,10), QPoint(50,65));
    KIS_DUMP_DEVICE_2(grayscaleDev, rect, "draw", "dd");

    QImage img = grayscaleDev->convertToQImage(0, rect);
    img = img.convertToFormat(QImage::Format_ARGB32);
    QPainter gc(&img);

    QPainterPath path;
    path.moveTo(points[0]);
    for (int i = 1; i < points.size(); i++) {
        path.lineTo(points[i]);
    }

    gc.setPen(Qt::white);
    gc.drawPath(path);

    gc.setPen(Qt::green);
    gc.drawEllipse(startPos, 3, 3);
    gc.setPen(Qt::red);
    gc.drawEllipse(endPos, 2, 2);

    img.save("result.png");
}

QTEST_MAIN(KisMagneticWorkerTest)
