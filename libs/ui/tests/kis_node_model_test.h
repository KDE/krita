/*
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISNODEMODEL_TEST_H
#define KISNODEMODEL_TEST_H

#include <simpletest.h>

#include "empty_nodes_test.h"

class KisDocument;
class KisNameServer;
class KisShapeController;
class KisNodeModel;


class KisNodeModelTest : public QObject, public TestUtil::EmptyNodesTest
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void testSetImage();
    void testAddNode();
    void testRemoveAllNodes();
    void testRemoveIncludingRoot();

private:
    void testSubstituteRootNode();

    KisDocument *m_doc;
    KisNameServer *m_nameServer;
    KisShapeController *m_shapeController;
    KisNodeModel *m_nodeModel;
};

#endif

