/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_AUTOBRUSH_RESOURCE_TEST_H
#define KIS_AUTOBRUSH_RESOURCE_TEST_H

#include <QtTest>

class KisAutoBrushTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();
    void testMaskGeneration();
    void testDabSize();
    void testCopyMasking();
    void testClone();

};

#endif
