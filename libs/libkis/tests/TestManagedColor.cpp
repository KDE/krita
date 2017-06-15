/* Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "TestManagedColor.h"
#include <QTest>
#include <QColor>
#include <QVector>

#include <KritaVersionWrapper.h>
#include <ManagedColor.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColor.h>

QTEST_MAIN(TestManagedColor)

void TestManagedColor::testOperatorIs()
{
    ManagedColor c1("RGBA", "U8", "");
    ManagedColor c2("RGBA", "U8", "");
    ManagedColor c3("RGBA", "U16", "");

    QVERIFY(c1 == c2);
}

void TestManagedColor::testSetColorSpace()
{
    ManagedColor c("RGBA", "U8", "");
    c.setColorSpace("LABA", "U16", "");
    QVERIFY(c.colorDepth() == "U16");
    QVERIFY(c.colorModel() == "LABA");
}

void TestManagedColor::testComponentsRoundTrip()
{
    ManagedColor c("RGBA", "U8", "");
    QVector<float> components = c.components();
    QVERIFY(components.size() == 4);
    QVERIFY(components[0] == 0);
    QVERIFY(components[1] == 0);
    QVERIFY(components[2] == 0);
    QVERIFY(components[3] == 0);

    components[0] = 0.5;
    components[1] = 0.5;
    components[2] = 0.5;
    components[3] = 0.5;
    c.setComponents(components);

    QVERIFY(c.toQString() == "Red 127 Green 127 Blue 127 Alpha 127");

}

void TestManagedColor::testXMLRoundTrip()
{
    ManagedColor c("RGBA", "U8", "");
    QVector<float> components;
    components << 0.5 << 0.5 << 0.5 << 0.5;
    c.setComponents(components);

    QString xml = c.toXML();
    c.fromXML(xml);
    components = c.components();
    QVERIFY(components.size() == 4);
    QVERIFY(c.toQString() == "Red 127 Green 127 Blue 127 Alpha 127");


}

void TestManagedColor::testToQString()
{
    ManagedColor c("RGBA", "U8", "");
    QVector<float> components;
    components << 0.5 << 0.5 << 0.5 << 0.5;
    c.setComponents(components);
    QVERIFY(c.toQString() == "Red 127 Green 127 Blue 127 Alpha 127");
}
