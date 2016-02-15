/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
