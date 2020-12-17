/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTSHAPEREORDERCOMMAND_H
#define TESTSHAPEREORDERCOMMAND_H

#include <QObject>

class TestShapeReorderCommand : public QObject
{
    Q_OBJECT
public:
    TestShapeReorderCommand();
    ~TestShapeReorderCommand() override;
private Q_SLOTS:
    void testZIndexSorting();
    void testRunThroughSorting();
    void testParentChildSorting();

    void testBringToFront();
    void testSendToBack();
    void testMoveUp();
    void testMoveDown();
    void testMoveUpOverlapping();
    void testMoveDownOverlapping();
    void testSendToBackChildren();
    void testNoCommand();
    void testMergeInShape();
    void testMergeInShapeDistant();
};

#endif // TESTSHAPEREORDERCOMMAND_H
