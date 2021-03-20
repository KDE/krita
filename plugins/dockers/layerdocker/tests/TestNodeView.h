/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NODE_VIEW_TEST_H
#define __KIS_NODE_VIEW_TEST_H

#include <simpletest.h>
#include "empty_nodes_test.h"

class KisDocument;
class KisNameServer;
class KisShapeController;


class NodeViewTest : public QObject, public TestUtil::EmptyNodesTest
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testLayers();
    void testColorLabels();

private:
    KisDocument *m_doc;
    KisNameServer *m_nameServer;
    KisShapeController *m_shapeController;
};

#endif /* __KIS_NODE_VIEW_TEST_H */
