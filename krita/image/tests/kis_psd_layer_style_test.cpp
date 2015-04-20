/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_psd_layer_style_test.h"
#include "kis_psd_layer_style.h"

#include <QString>
#include <QFile>
#include <QByteArray>
#include <QBuffer>

#include <qtest_kde.h>

void KisPSDLayerStyleTest::testRoundTrip()
{
/*    KisPSDLayerStyle layerStyle;
    QFile f(FILES_DATA_DIR + QDir::separator() + "teststyles.asl");
    bool res = f.open(QIODevice::ReadOnly);
    Q_ASSERT(res);
    QByteArray ba = f.readAll();
    f.close();
    Q_ASSERT(ba.size() > 0);
    QBuffer in(&ba);
    KisPSDLayerStyle::StylesVector styles;
    styles = layerStyle.readASL(&in);

    QByteArray ba2;
    QBuffer out(&ba2);
    res = layerStyle.writeASL(&out, styles);
    Q_ASSERT(res);*/
}

QTEST_KDEMAIN(KisPSDLayerStyleTest, GUI)
