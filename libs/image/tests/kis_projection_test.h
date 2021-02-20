/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PROJECTION_TEST_H
#define KIS_PROJECTION_TEST_H

#include <QtTest>

class KisProjectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDirty();
};

#endif

