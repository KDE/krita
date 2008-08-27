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

#include "kis_brush_test.h"

#include <qtest_kde.h>
#include <QString>
#include <QDir>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "../kis_brush.h"
#include "kis_types.h"
#include "kis_paint_device.h"

void KisBrushTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    QImage img(512, 512, QImage::Format_ARGB32);

    KisBrush a(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gbr");
    KisBrush b(dev, 0, 0, 10, 10);
    KisBrush c(img, "bla");
    KisBrush d(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gih");
}


QTEST_KDEMAIN(KisBrushTest, GUI)
#include "kis_brush_test.moc"
