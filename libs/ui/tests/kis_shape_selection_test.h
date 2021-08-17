/*
 *  SPDX-FileCopyrightText: 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SHAPE_SELECTION_TEST_H
#define KIS_SHAPE_SELECTION_TEST_H

#include <simpletest.h>

class KisShapeSelectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testAddChild();

    void testUndoFlattening();

    void testHistoryOnFlattening();
};

#endif

