/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoPropertiesTest.h"

#include <qtest_kde.h>
#include <KoProperties.h>

void KoPropertiesTest::testDeserialization()
{
    QString test = "";
    KoProperties props;
    props.setProperty("bla", "bla");

    QVERIFY(!props.load(test));
    QVERIFY(!props.isEmpty());
    QVERIFY(props.stringProperty("bla") == "bla");

    test = "<bla>asdsadasjk</bla>";
    QVERIFY(props.load(test));
    QVERIFY(props.isEmpty());

    props.setProperty("bla", "bla");
    test = "<bla>asdsadasjk</";
    QVERIFY(!props.load(test));
    QVERIFY(!props.isEmpty());
    QVERIFY(props.stringProperty("bla") == "bla");

}

void KoPropertiesTest::testRoundTrip()
{
    KoProperties props;
    props.setProperty("string", "string");
    props.setProperty("xmlstring", "<xml>bla</xml>");
    props.setProperty("xmlstring2", "<xml>&adsa</xml>");
    props.setProperty("cdata", "<![CDATA[blabla]]>");
    props.setProperty("int", 10);
    props.setProperty("bool",  false);
    props.setProperty("qreal",  1.38);

    QString stored = props.store("KoPropertiesTest");
    KoProperties restored;
    restored.load(stored);

    QVERIFY(restored.stringProperty("string") == "string");
    QVERIFY(restored.stringProperty("xmlstring") == "<xml>bla</xml>");
    QVERIFY(restored.stringProperty("xmlstring2") == "<xml>&adsa</xml>");
    QVERIFY(restored.stringProperty("cdata") == "<![CDATA[blabla]]>");
    QVERIFY(restored.intProperty("int") == 10);
    QVERIFY(restored.boolProperty("bool") == false);
    QVERIFY(restored.doubleProperty("qreal") == 1.38);

}

void KoPropertiesTest::testProperties()
{
    KoProperties props;
    QVERIFY(props.isEmpty());

    QString visible = "visible";
    QVERIFY(!props.value(visible).isValid());

    props.setProperty("visible", "bla");
    QVERIFY(props.value("visible") == "bla");
    QVERIFY(props.stringProperty("visible", "blabla") == "bla");

    props.setProperty("bool",  true);
    QVERIFY(props.boolProperty("bool", false) == true);
    props.setProperty("bool",  false);
    QVERIFY(props.boolProperty("bool", true) == false);

    props.setProperty("qreal",  1.0);
    QVERIFY(props.doubleProperty("qreal", 2.0) == 1.0);
    props.setProperty("qreal",  2.0);
    QVERIFY(props.doubleProperty("qreal", 1.0) == 2.0);

    props.setProperty("int",  1);
    QVERIFY(props.intProperty("int", 2) == 1);
    props.setProperty("int",  2);
    QVERIFY(props.intProperty("int", 1) == 2);

    QVariant v;
    QVERIFY(props.property("sdsadsakldjsajd", v) == false);
    QVERIFY(!v.isValid());
    QVERIFY(props.property("visible", v) == true);
    QVERIFY(v.isValid());
    QVERIFY(v == "bla");

    QVERIFY(!props.isEmpty());
    QVERIFY(props.contains("visible"));
    QVERIFY(!props.contains("adsajkdsakj dsaieqwewqoie"));
    QVERIFY(props.contains(visible));

    int count = 0;
    QMapIterator<QString, QVariant> iter = props.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        count++;
    }
    QVERIFY(count == 4);

}

bool checkProps(const KoProperties & props)
{
    return (props.value("bla") == 1);
}

void KoPropertiesTest::testPassAround()
{
    KoProperties props;
    props.setProperty("bla", 1);
    QVERIFY(checkProps(props));

    KoProperties props2 = props;
    QVERIFY(checkProps(props2));

    KoProperties props3(props);
    checkProps(props3);
    props3.setProperty("bla", 3);
    QVERIFY(props3.value("bla") == 3);

    QVERIFY(checkProps(props));
    QVERIFY(checkProps(props2));

}

QTEST_KDEMAIN(KoPropertiesTest, NoGUI)
#include <KoPropertiesTest.moc>
