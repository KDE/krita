/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFORESTTEST_H
#define KISFORESTTEST_H


#include <QtTest>
#include <QObject>

class KisForestTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAddToRoot();
    void testAddToRootChained();
    void testAddToLeaf();
    void testAddToLeafChained();

    void testDFSIteration();
    void testHierarchyIteration();
    void testSiblingIteration();
    void testCompositionIteration();
    void testCompositionIterationSubtree();

    void testSubtreeIteration();
    void testSubtreeTailIteration();

    void testEraseNode();
    void testEraseSubtree();
    void testEraseRange();

    void testMoveSubtree();

    void testReversedChildIteration();

    void testConversionsFromEnd();
};

#endif // KISFORESTTEST_H
