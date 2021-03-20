/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CS_CONVERSION_TESTER_H
#define KIS_CS_CONVERSION_TESTER_H

#include <simpletest.h>

class KisCsConversionTest : public QObject
{
    Q_OBJECT


private Q_SLOTS:

    void testColorSpaceConversion();
};

#endif

