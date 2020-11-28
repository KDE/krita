/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DUMMIES_FACADE_BASE_TEST_H
#define __KIS_DUMMIES_FACADE_BASE_TEST_H

#include <QtTest>

#include "empty_nodes_test.h"

class KisNodeDummy;
class KisDummiesFacadeBase;

class KisDummiesFacadeBaseTest : public QObject, public TestUtil::EmptyNodesTest
{
    Q_OBJECT

protected:
    virtual KisDummiesFacadeBase* dummiesFacadeFactory() = 0;
    virtual void destroyDummiesFacade(KisDummiesFacadeBase *dummiesFacade) = 0;

private Q_SLOTS:
    void slotNodeActivated(KisNodeSP node);
    void slotEndInsertDummy(KisNodeDummy *dummy);
    void slotBeginRemoveDummy(KisNodeDummy *dummy);

private Q_SLOTS:
    void init();
    void cleanup();

    void testSetImage();
    void testAddNode();
    void testRemoveNode();
    void testMoveNodeSameParent();
    void testMoveNodeDifferentParent();
    void testSubstituteRootNode();
    void testAddSelectionMasksNoActivation();

private:

    void verifyActivatedNodes(const QString &nodes);
    void verifyMovedDummies(const QString &nodes);

private:
    KisDummiesFacadeBase *m_dummiesFacade;

    QString m_activatedNodes;
    QString m_movedDummies;
};

#endif /* __KIS_DUMMIES_FACADE_BASE_TEST_H */
