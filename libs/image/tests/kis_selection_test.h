/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SELECTION_TEST_H
#define KIS_SELECTION_TEST_H

#include <simpletest.h>

class KisSelectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testGrayColorspaceConversion();
    void testGrayColorspaceOverComposition();

    void testSelectionComponents();
    void testSelectionActions();
    void testInvertSelection();
    void testInvertSelectionSemi();
    void testCopy();
    void testSelectionExactBounds();
    void testSetParentNodeAfterCreation();
    void testSetParentNodeBeforeCreation();

    void testOutlineGeneration();
};

#endif

