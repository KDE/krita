/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
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

#ifndef KISSHAPECONTROLLER_TEST_H
#define KISSHAPECONTROLLER_TEST_H

#include <QtTest/QtTest>

#include "empty_nodes_test.h"

class KisDoc2;
class KisNameServer;
class KisShapeController;
class KisNodeDummy;


class KisShapeControllerTest : public QObject, public TestUtil::EmptyNodesTest
{
    Q_OBJECT

protected slots:
    void slotNodeActivated(KisNodeSP node);
    void slotEndInsertDummy(KisNodeDummy *dummy);
    void slotBeginRemoveDummy(KisNodeDummy *dummy);

private slots:
    void init();
    void cleanup();

    void testSetImage();
    void testAddNode();
    void testRemoveNode();
    void testMoveNodeSameParent();
    void testMoveNodeDifferentParent();
    void testSubstituteRootNode();

private:
    void verifyActivatedNodes(const QString &nodes);
    void verifyMovedDummies(const QString &nodes);
private:
    KisDoc2 *m_doc;
    KisNameServer *m_nameServer;
    KisShapeController *m_shapeController;

    QString m_activatedNodes;
    QString m_movedDummies;
};

#endif

