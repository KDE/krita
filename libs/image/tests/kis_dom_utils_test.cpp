/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dom_utils_test.h"

#include <simpletest.h>

#include "kis_dom_utils.h"
#include "kis_debug.h"


const qreal f1 = 0.0003;
const qreal f2 = 1e-15;
const qreal f3 = 1356.78301;
const qreal f4 = 1234567.8901;

const int i1 = 0;
const int i2 = 13;
const int i3 = -13;

inline bool checkDifference(qreal a, qreal b, qreal portionTolerance)
{
    return qAbs(a) > 1e-10 ?
        qAbs(a - b) / qAbs(a) < portionTolerance :
        qAbs(a - b) < 1e-10;
}

QString saveData()
{
    QDomDocument doc("testdoc");

    QDomElement root = doc.createElement("rootNode");
    doc.appendChild(root);

    KisDomUtils::saveValue(&root, "valueF1", f1);
    KisDomUtils::saveValue(&root, "valueF2", f2);
    KisDomUtils::saveValue(&root, "valueF3", f3);
    KisDomUtils::saveValue(&root, "valueF4", f4);

    KisDomUtils::saveValue(&root, "valueI1", i1);
    KisDomUtils::saveValue(&root, "valueI2", i2);
    KisDomUtils::saveValue(&root, "valueI3", i3);

    return doc.toString();
}

void checkLoadData(const QString &xmlData)
{
    QDomDocument doc;
    doc.setContent(xmlData);

    QDomElement root = doc.documentElement();

    qreal value = 0.0;

    QVERIFY(KisDomUtils::loadValue(root, "valueF1", &value));
    QVERIFY(checkDifference(f1, value, 0.01));

    QVERIFY(KisDomUtils::loadValue(root, "valueF2", &value));
    QVERIFY(checkDifference(f2, value, 0.01));

    QVERIFY(KisDomUtils::loadValue(root, "valueF3", &value));
    QVERIFY(checkDifference(f3, value, 0.01));

    QVERIFY(KisDomUtils::loadValue(root, "valueF4", &value));
    QVERIFY(checkDifference(f4, value, 0.01));

    int iValue = 0;

    QVERIFY(KisDomUtils::loadValue(root, "valueI1", &iValue));
    QCOMPARE(i1, iValue);

    QVERIFY(KisDomUtils::loadValue(root, "valueI2", &iValue));
    QCOMPARE(i2, iValue);

    QVERIFY(KisDomUtils::loadValue(root, "valueI3", &iValue));
    QCOMPARE(i3, iValue);
}

void KisDomUtilsTest::testC2C()
{
    QLocale::setDefault(QLocale::C);
    QString xmlData = saveData();

    QLocale::setDefault(QLocale::C);
    checkLoadData(xmlData);
}

void KisDomUtilsTest::testG2G()
{
    QLocale::setDefault(QLocale::German);
    QString xmlData = saveData();

    QLocale::setDefault(QLocale::German);
    checkLoadData(xmlData);
}

void KisDomUtilsTest::testR2R()
{
    QLocale::setDefault(QLocale::Russian);
    QString xmlData = saveData();

    QLocale::setDefault(QLocale::Russian);
    checkLoadData(xmlData);
}

void KisDomUtilsTest::testC2G()
{
    QLocale::setDefault(QLocale::C);
    QString xmlData = saveData();

    QLocale::setDefault(QLocale::German);
    checkLoadData(xmlData);
}

void KisDomUtilsTest::testR2G()
{
    QLocale::setDefault(QLocale::Russian);
    QString xmlData = saveData();

    QLocale::setDefault(QLocale::German);
    checkLoadData(xmlData);
}

void KisDomUtilsTest::testG2C()
{
    QLocale::setDefault(QLocale::German);
    QString xmlData = saveData();

    QLocale::setDefault(QLocale::C);
    checkLoadData(xmlData);
}

void KisDomUtilsTest::testG2R()
{
    QLocale::setDefault(QLocale::German);
    QString xmlData = saveData();

    QLocale::setDefault(QLocale::Russian);
    checkLoadData(xmlData);
}

#include "kis_time_span.h"

void KisDomUtilsTest::testIntegralType()
{
    KisTimeSpan r1 = KisTimeSpan::fromTimeWithDuration(1, 10);
    KisTimeSpan r2 = KisTimeSpan::fromTimeWithDuration(5, 15);


    QDomDocument doc("testdoc");

    QDomElement root = doc.createElement("rootNode");
    doc.appendChild(root);

    KisDomUtils::saveValue(&root, "timeRange", r1);
    KisDomUtils::loadValue(root, "timeRange", &r2);

    QCOMPARE(r2, r1);
}

SIMPLE_TEST_MAIN(KisDomUtilsTest)
