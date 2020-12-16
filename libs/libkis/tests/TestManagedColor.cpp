/* SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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

#include <sdk/tests/testui.h>

KISTEST_MAIN(TestManagedColor)

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
