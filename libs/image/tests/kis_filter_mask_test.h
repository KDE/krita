
/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTER_MASK_TEST_H
#define KIS_FILTER_MASK_TEST_H

#include <QtTest>

class KisFilterMaskTest : public QObject
{
    Q_OBJECT


private Q_SLOTS:

    void testProjectionNotSelected();
    void testProjectionSelected();

};

#endif

