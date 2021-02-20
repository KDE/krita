/*
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_PNG_TEST_H_
#define _KIS_PNG_TEST_H_

#include <QtTest>

class KisPngTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFiles();
    void testWriteonly();
    void testSaveHDR();
};

#endif
