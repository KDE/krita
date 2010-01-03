/*
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TestKoColor.h"

#include <qtest_kde.h>

#include <QDomElement>

#include "KoColorModelStandardIds.h"

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

bool nearEqualValue(int a, int b)
{
    return qAbs(a -b) <= 1;
}

void TestKoColor::testForModel(QString model)
{
    QColor qc(200, 125, 100);
    QList<KoID> depthIDs = KoColorSpaceRegistry::instance()->colorDepthList(model, KoColorSpaceRegistry::AllColorSpaces);
    foreach(KoID depthId, depthIDs) {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(KoColorSpaceRegistry::instance()->colorSpaceId(model, depthId.id()) , "");
        KoColor kc(cs);
        kc.fromQColor(qc);
        QDomDocument doc;
        QDomElement elt = doc.createElement("color");
        kc.toXML(doc, elt);
        doc.appendChild(elt);
        KoColor kcu = KoColor::fromXML(elt.firstChildElement(), depthId.id(), QHash<QString, QString>());
        QVERIFY2(*(kc.colorSpace()) == *(kcu.colorSpace()),
                 QString("Not identical color space (colorModelId = %1 depthId = %2) != (colorModelId = %3 depthId = %4) ")
                 .arg(kc.colorSpace()->colorModelId().id())
                 .arg(kc.colorSpace()->colorDepthId().id())
                 .arg(kcu.colorSpace()->colorModelId().id())
                 .arg(kcu.colorSpace()->colorDepthId().id()).toLatin1());
        QColor qc2 = kc.toQColor();
        QColor qcu = kcu.toQColor();
        QVERIFY2(nearEqualValue(qcu.red(), qc2.red()), QString("Unserialization failed colorModelId = %1 depthId = %2 red = %3 red = %4").arg(model).arg(depthId.id()).arg(qcu.red()).arg(qc2.red()).toLatin1());
        QVERIFY2(nearEqualValue(qcu.green(), qc2.green()), QString("Unserialization failed colorModelId = %1 depthId = %2 green = %3 green = %4").arg(model).arg(depthId.id()).arg(qcu.green()).arg(qc2.green()).toLatin1());
        QVERIFY2(nearEqualValue(qcu.blue(), qc2.blue()), QString("Unserialization failed colorModelId = %1 depthId = %2 blue = %3 blue = %4").arg(model).arg(depthId.id()).arg(qcu.blue()).arg(qc2.blue()).toLatin1());
    }

}

void TestKoColor::testSerialization()
{
    testForModel(RGBAColorModelID.id());
    testForModel(XYZAColorModelID.id());
    testForModel(LABAColorModelID.id());
    testForModel(CMYKAColorModelID.id());
    testForModel(GrayAColorModelID.id());
    testForModel(YCbCrAColorModelID.id());
}

QTEST_KDEMAIN(TestKoColor, NoGUI)
#include <TestKoColor.moc>

