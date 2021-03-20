/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTER_TEST_H
#define KIS_FILTER_TEST_H

#include <simpletest.h>

class KisFilterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();
    void testWithProgressUpdater();
    void testSingleThreaded();
    void testDifferentSrcAndDst();
    void testOldDataApiAfterCopy();
    void testBlurFilterApplicationRect();
};

#endif
