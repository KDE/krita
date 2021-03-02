/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_KRA_LOADER_TEST_H
#define KIS_KRA_LOADER_TEST_H

#include <simpletest.h>

class KisKraLoaderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
     void initTestCase();

    void testLoading();
    void testObligeSingleChild();
    void testObligeSingleChildNonTranspPixel();

    void testLoadAnimated();

    void testImportFromWriteonly();
    void testImportIncorrectFormat();


};

#endif
