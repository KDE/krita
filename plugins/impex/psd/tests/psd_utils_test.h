/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _PSD_UTILS_TEST_H_
#define _PSD_UTILS_TEST_H_

#include <QtTest>

class PSDUtilsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void test_psdwrite_quint8();
    void test_psdwrite_quint16();
    void test_psdwrite_quint32();
    void test_psdwrite_qstring();
    void test_psdwrite_pascalstring();
    void test_psdpad();

    void test_psdread_quint8();
    void test_psdread_quint16();
    void test_psdread_quint32();
    void test_psdread_pascalstring();

};

#endif
