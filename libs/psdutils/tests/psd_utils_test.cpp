/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "psd_utils_test.h"

#include <QBuffer>
#include <QByteArray>
#include <QCoreApplication>
#include <klocalizedstring.h>

#include <kis_debug.h>
#include <psd.h>
#include <psd_utils.h>
#include <simpletest.h>

void PSDUtilsTest::test_psdwrite_quint8()
{
    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint8 i = 3;
        QVERIFY(psdwrite<psd_byte_order::psdLittleEndian>(buf, i));
        QCOMPARE(buf.buffer().size(), 1);
        QCOMPARE(buf.buffer().at(0), '\3');
    }

    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint8 i = 3;
        QVERIFY(psdwrite(buf, i));
        QCOMPARE(buf.buffer().size(), 1);
        QCOMPARE(buf.buffer().at(0), '\3');
    }
}

void PSDUtilsTest::test_psdwrite_quint16()
{
    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint16 i = 0x3u;
        QVERIFY(psdwrite<psd_byte_order::psdLittleEndian>(buf, i));
        QCOMPARE(buf.buffer().size(), 2);
        QCOMPARE(buf.buffer().at(0), '\x03');
        QCOMPARE(buf.buffer().at(1), '\x00');
    }

    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint16 i = 0x3u;
        QVERIFY(psdwrite(buf, i));
        QCOMPARE(buf.buffer().size(), 2);
        QCOMPARE(buf.buffer().at(0), '\x00');
        QCOMPARE(buf.buffer().at(1), '\x03');
    }
}

void PSDUtilsTest::test_psdwrite_quint32()
{
    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint32 i = 0x64u;
        QVERIFY(psdwrite<psd_byte_order::psdLittleEndian>(buf, i));
        QCOMPARE(buf.buffer().size(), 4);
        QCOMPARE(buf.buffer().at(0), '\x64');
        QCOMPARE(buf.buffer().at(1), '\x00');
        QCOMPARE(buf.buffer().at(2), '\x00');
        QCOMPARE(buf.buffer().at(3), '\x00');
    }

    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint32 i = 0x64u;
        QVERIFY(psdwrite(buf, i));
        QCOMPARE(buf.buffer().size(), 4);
        QCOMPARE(buf.buffer().at(0), '\x00');
        QCOMPARE(buf.buffer().at(1), '\x00');
        QCOMPARE(buf.buffer().at(2), '\x00');
        QCOMPARE(buf.buffer().at(3), '\x64');
    }
}

void PSDUtilsTest::test_psdwrite_qstring()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    QString s = "8BPS";
    QVERIFY(psdwrite(buf, s));
    QCOMPARE(buf.buffer().size(), 4);
    QCOMPARE(buf.buffer().at(0), '8');
    QCOMPARE(buf.buffer().at(1), 'B');
    QCOMPARE(buf.buffer().at(2), 'P');
    QCOMPARE(buf.buffer().at(3), 'S');
}

void PSDUtilsTest::test_psdwrite_pascalstring()
{
    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);

        // test null string
        QString s;
        QVERIFY(psdwrite_pascalstring<psd_byte_order::psdLittleEndian>(buf, s));
        QCOMPARE(buf.buffer().size(), 2);
        QCOMPARE(buf.buffer().at(0), '\0');
        QCOMPARE(buf.buffer().at(1), '\0');

        buf.close();
        buf.buffer().clear();
        buf.open(QBuffer::ReadWrite);

        // test even string
        s = QString("bl");
        QVERIFY(psdwrite_pascalstring<psd_byte_order::psdLittleEndian>(buf, s));
        QCOMPARE(buf.buffer().size(), 3);
        QCOMPARE(buf.buffer().at(0), '\x02');
        QCOMPARE(buf.buffer().at(1), 'b');
        QCOMPARE(buf.buffer().at(2), 'l');

        buf.close();
        buf.buffer().clear();
        buf.open(QBuffer::ReadWrite);

        // test uneven string
        s = QString("bla");
        QVERIFY(psdwrite_pascalstring<psd_byte_order::psdLittleEndian>(buf, s));
        QCOMPARE(buf.buffer().size(), 5);
        QCOMPARE(buf.buffer().at(0), '\x03');
        QCOMPARE(buf.buffer().at(1), 'b');
        QCOMPARE(buf.buffer().at(2), 'l');
        QCOMPARE(buf.buffer().at(3), 'a');
        QCOMPARE(buf.buffer().at(4), '\0');
    }

    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);

        // test null string
        QString s;
        QVERIFY(psdwrite_pascalstring(buf, s));
        QCOMPARE(buf.buffer().size(), 2);
        QCOMPARE(buf.buffer().at(0), '\0');
        QCOMPARE(buf.buffer().at(1), '\0');

        buf.close();
        buf.buffer().clear();
        buf.open(QBuffer::ReadWrite);

        // test even string
        s = QString("bl");
        QVERIFY(psdwrite_pascalstring(buf, s));
        QCOMPARE(buf.buffer().size(), 3);
        QCOMPARE(buf.buffer().at(0), '\2');
        QCOMPARE(buf.buffer().at(1), 'b');
        QCOMPARE(buf.buffer().at(2), 'l');

        buf.close();
        buf.buffer().clear();
        buf.open(QBuffer::ReadWrite);

        // test uneven string
        s = QString("bla");
        QVERIFY(psdwrite_pascalstring(buf, s));
        QCOMPARE(buf.buffer().size(), 5);
        QCOMPARE(buf.buffer().at(0), '\3');
        QCOMPARE(buf.buffer().at(1), 'b');
        QCOMPARE(buf.buffer().at(2), 'l');
        QCOMPARE(buf.buffer().at(3), 'a');
        QCOMPARE(buf.buffer().at(4), '\0');
    }
}

void PSDUtilsTest::test_psdpad()
{
    QBuffer buf;
    buf.open(QBuffer::ReadWrite);
    QVERIFY(psdpad(buf, 6));
    QCOMPARE(buf.buffer().size(), 6);
    QCOMPARE(buf.buffer().at(0), '\x00');
    QCOMPARE(buf.buffer().at(1), '\x00');
    QCOMPARE(buf.buffer().at(2), '\x00');
    QCOMPARE(buf.buffer().at(3), '\x00');
    QCOMPARE(buf.buffer().at(4), '\x00');
    QCOMPARE(buf.buffer().at(5), '\x00');
}

void PSDUtilsTest::test_psdread_quint8()
{
    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint8 s = 3;
        QVERIFY(psdwrite<psd_byte_order::psdLittleEndian>(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        quint8 r;
        QVERIFY(psdread<psd_byte_order::psdLittleEndian>(buf, r));
        QCOMPARE(r, s);
    }

    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint8 s = 3;
        QVERIFY(psdwrite(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        quint8 r;
        QVERIFY(psdread(buf, r));
        QCOMPARE(r, s);
    }
}

void PSDUtilsTest::test_psdread_quint16()
{
    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        const quint16 s = 1024;
        QVERIFY(psdwrite<psd_byte_order::psdLittleEndian>(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        quint16 r;
        QVERIFY(psdread<psd_byte_order::psdLittleEndian>(buf, r));
        QCOMPARE(r, s);
    }

    {
        QBuffer buf;
        buf.open(QBuffer::ReadWrite);
        quint16 s = 1024;
        QVERIFY(psdwrite(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        quint16 r;
        QVERIFY(psdread(buf, r));
        QCOMPARE(r, s);
    }
}

void PSDUtilsTest::test_psdread_quint32()
{
    {
        QBuffer buf;
        QVERIFY(buf.open(QBuffer::ReadWrite));
        const quint32 s = 300000;
        QVERIFY(psdwrite<psd_byte_order::psdLittleEndian>(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        quint32 r;
        QVERIFY(psdread<psd_byte_order::psdLittleEndian>(buf, r));
        QCOMPARE(r, s);
    }

    {
        QBuffer buf;
        QVERIFY(buf.open(QBuffer::ReadWrite));
        quint32 s = 300000;
        QVERIFY(psdwrite(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        quint32 r;
        QVERIFY(psdread(buf, r));
        QCOMPARE(r, s);
    }
}

void PSDUtilsTest::test_psdread_pascalstring()
{
    {
        QBuffer buf;

        QString s;
        QString r;

        // test null string
        buf.open(QBuffer::ReadWrite);
        QVERIFY(psdwrite_pascalstring<psd_byte_order::psdLittleEndian>(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        QVERIFY(psdread_pascalstring<psd_byte_order::psdLittleEndian>(buf, r, 2));
        QCOMPARE(r, s);
        QVERIFY(buf.bytesAvailable() == 0);

        // test even string
        buf.close();
        buf.buffer().clear();
        r.clear();
        buf.open(QBuffer::ReadWrite);
        s = QString("bl");
        QVERIFY(psdwrite_pascalstring<psd_byte_order::psdLittleEndian>(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        QVERIFY(psdread_pascalstring<psd_byte_order::psdLittleEndian>(buf, r, 1));
        QCOMPARE(r, s);
        QVERIFY(buf.bytesAvailable() == 0);

        // test uneven string
        buf.close();
        buf.buffer().clear();
        r.clear();
        buf.open(QBuffer::ReadWrite);
        s = QString("bla");
        QVERIFY(psdwrite_pascalstring<psd_byte_order::psdLittleEndian>(buf, s, 2));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        QVERIFY(psdread_pascalstring<psd_byte_order::psdLittleEndian>(buf, r, 2));
        QCOMPARE(r, s);
        dbgKrita << buf.bytesAvailable();
        QVERIFY(buf.bytesAvailable() == 0);
    }

    {
        QBuffer buf;

        QString s;
        QString r;

        // test null string
        buf.open(QBuffer::ReadWrite);
        QVERIFY(psdwrite_pascalstring(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        QVERIFY(psdread_pascalstring(buf, r, 2));
        QCOMPARE(r, s);
        QVERIFY(buf.bytesAvailable() == 0);

        // test even string
        buf.close();
        buf.buffer().clear();
        r.clear();
        buf.open(QBuffer::ReadWrite);
        s = QString("bl");
        QVERIFY(psdwrite_pascalstring(buf, s));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        QVERIFY(psdread_pascalstring(buf, r, 1));
        QCOMPARE(r, s);
        QVERIFY(buf.bytesAvailable() == 0);

        // test uneven string
        buf.close();
        buf.buffer().clear();
        r.clear();
        buf.open(QBuffer::ReadWrite);
        s = QString("bla");
        QVERIFY(psdwrite_pascalstring(buf, s, 2));
        buf.close();
        buf.open(QBuffer::ReadOnly);
        QVERIFY(psdread_pascalstring(buf, r, 2));
        QCOMPARE(r, s);
        dbgKrita << buf.bytesAvailable();
        QVERIFY(buf.bytesAvailable() == 0);
    }
}

SIMPLE_TEST_MAIN(PSDUtilsTest)
