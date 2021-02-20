/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SELECTION_MANAGER_TEST_H
#define __KIS_SELECTION_MANAGER_TEST_H

#include <QtTest>

class KisSelectionManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFillForegroundWithoutSelection();
    void testFillForegroundWithSelection();
    void testFillBackgroundWithSelection();
    void testFillPatternWithSelection();

    void testResizeToSelection();

    void testSelectAll();
    void testDeselectReselect();

    void testCopyPaste();
    void testCopyPasteMerged();
    void testCutPaste();

    void testInvertSelection();

    void testScanline16bit();
private:
    // These come from a plugin and cannot be tested here
    void testFeatherSelection();
    void testGrowSelectionSimplified();
    void testShrinkSelectionUnlockedSimplified();
    void testShrinkSelectionLockedSimplified();
    void testSmoothSelectionSimplified();
    void testErodeSelectionSimplified();
    void testDilateSelectionSimplified();
    void testBorderSelectionSimplified();

};

#endif /* __KIS_SELECTION_MANAGER_TEST_H */
