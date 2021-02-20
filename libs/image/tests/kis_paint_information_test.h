/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINT_INFORMATION_TEST_H
#define KIS_PAINT_INFORMATION_TEST_H

#include <QtTest>

class KisPaintInformationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();
    void testSerialisation();

    void benchmarkTausRandomGeneration();
};

#endif
