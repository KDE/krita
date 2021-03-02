/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PATTERN_TEST_H
#define KIS_PATTERN_TEST_H

#include <simpletest.h>

class KoPatternTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();
    void testRoundTripMd5();

};

#endif
