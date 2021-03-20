/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "psd_utils_test.h"

#include <simpletest.h>
#include <QCoreApplication>
#include <klocalizedstring.h>
#include <psd_utils.h>
#include <QByteArray>
#include <QBuffer>

#include <kis_debug.h>

void PSDUtilsTest::test_psdwrite_quint8()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    quint8 i = 3;
    QVERIFY(psdwrite(&buf, i));
    QCOMPARE(buf.buffer().size(), 1);
    QCOMPARE(buf.buffer().at(0), '\3');
}


void PSDUtilsTest::test_psdwrite_quint16()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    quint16 i = 3;
    QVERIFY(psdwrite(&buf, i));
    QCOMPARE(buf.buffer().size(), 2);
    QCOMPARE(buf.buffer().at(0), '\0');
    QCOMPARE(buf.buffer().at(1), '\3');
}

void PSDUtilsTest::test_psdwrite_quint32()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    quint32 i = 100;
    QVERIFY(psdwrite(&buf, i));
    QCOMPARE(buf.buffer().size(), 4);
    QCOMPARE(buf.buffer().at(0), '\0');
    QCOMPARE(buf.buffer().at(1), '\0');
    QCOMPARE(buf.buffer().at(2), '\0');
    QCOMPARE(buf.buffer().at(3), 'd');
}

void PSDUtilsTest::test_psdwrite_qstring()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    QString s = "8BPS";
    QVERIFY(psdwrite(&buf, s));
    QCOMPARE(buf.buffer().size(), 4);
    QCOMPARE(buf.buffer().at(0), '8');
    QCOMPARE(buf.buffer().at(1), 'B');
    QCOMPARE(buf.buffer().at(2), 'P');
    QCOMPARE(buf.buffer().at(3), 'S');
}

void PSDUtilsTest::test_psdwrite_pascalstring()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);

    // test null string
    QString s;
    QVERIFY(psdwrite_pascalstring(&buf, s));
    QCOMPARE(buf.buffer().size(), 2);
    QCOMPARE(buf.buffer().at(0), '\0');
    QCOMPARE(buf.buffer().at(1), '\0');

    buf.close();
    buf.buffer().clear();
    buf.open(QBuffer::ReadWrite);

    // test even string
    s = QString("bl");
    QVERIFY(psdwrite_pascalstring(&buf, s));
    QCOMPARE(buf.buffer().size(), 3);
    QCOMPARE(buf.buffer().at(0), '\2');
    QCOMPARE(buf.buffer().at(1), 'b');
    QCOMPARE(buf.buffer().at(2), 'l');

    buf.close();
    buf.buffer().clear();
    buf.open(QBuffer::ReadWrite);

    // test uneven string
    s = QString("bla");
    QVERIFY(psdwrite_pascalstring(&buf, s));
    QCOMPARE(buf.buffer().size(), 5);
    QCOMPARE(buf.buffer().at(0), '\3');
    QCOMPARE(buf.buffer().at(1), 'b');
    QCOMPARE(buf.buffer().at(2), 'l');
    QCOMPARE(buf.buffer().at(3), 'a');
    QCOMPARE(buf.buffer().at(4), '\0');
}


void PSDUtilsTest::test_psdpad()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    QVERIFY(psdpad(&buf, 6));
    QCOMPARE(buf.buffer().size(), 6);
    QCOMPARE(buf.buffer().at(0), '\0');
    QCOMPARE(buf.buffer().at(1), '\0');
    QCOMPARE(buf.buffer().at(2), '\0');
    QCOMPARE(buf.buffer().at(3), '\0');
    QCOMPARE(buf.buffer().at(4), '\0');
    QCOMPARE(buf.buffer().at(5), '\0');
}

void PSDUtilsTest::test_psdread_quint8()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    quint8 s = 3;
    QVERIFY(psdwrite(&buf, s));
    buf.close();
    buf.open(QBuffer::ReadOnly);
    quint8 r;
    QVERIFY(psdread(&buf, &r));
    QCOMPARE(r, s);
}

void PSDUtilsTest::test_psdread_quint16()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    quint16 s = 1024;
    QVERIFY(psdwrite(&buf, s));
    buf.close();
    buf.open(QBuffer::ReadOnly);
    quint16 r;
    QVERIFY(psdread(&buf, &r));
    QCOMPARE(r, s);
}

void PSDUtilsTest::test_psdread_quint32()
{
    QBuffer buf;
    QVERIFY(buf.open(QBuffer::ReadWrite));
    quint32 s = 300000;
    psdwrite(&buf, s);
    buf.close();
    buf.open(QBuffer::ReadOnly);
    quint32 r;
    QVERIFY(psdread(&buf, &r));
    QCOMPARE(r, s);
}

void PSDUtilsTest::test_psdread_pascalstring()
{
    QBuffer buf;

    QString s;
    QString r;

    // test null string
    buf.open(QBuffer::ReadWrite);
    QVERIFY(psdwrite_pascalstring(&buf, s));
    buf.close();
    buf.open(QBuffer::ReadOnly);
    psdread_pascalstring(&buf, r, 2);
    QCOMPARE(r, s);
    QVERIFY(buf.bytesAvailable() == 0);

    // test even string
    buf.close();
    buf.buffer().clear();
    r.clear();
    buf.open(QBuffer::ReadWrite);
    s = QString("bl");
    QVERIFY(psdwrite_pascalstring(&buf, s));
    buf.close();
    buf.open(QBuffer::ReadOnly);
    psdread_pascalstring(&buf, r, 1);
    QCOMPARE(r, s);
    QVERIFY(buf.bytesAvailable() == 0);

    // test uneven string
    buf.close();
    buf.buffer().clear();
    r.clear();
    buf.open(QBuffer::ReadWrite);
    s = QString("bla");
    QVERIFY(psdwrite_pascalstring(&buf, s, 2));
    buf.close();
    buf.open(QBuffer::ReadOnly);
    psdread_pascalstring(&buf, r, 2);
    QCOMPARE(r, s);
    dbgKrita << buf.bytesAvailable();
    QVERIFY(buf.bytesAvailable() == 0);
}

SIMPLE_TEST_MAIN(PSDUtilsTest)

