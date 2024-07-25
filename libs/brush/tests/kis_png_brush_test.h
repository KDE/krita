/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PNG_BRUSH_TEST_H
#define KIS_PNG_BRUSH_TEST_H

#include <simpletest.h>

class KisPngBrushTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testLoading_data();
    void testLoading();
};

#endif
