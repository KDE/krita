/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __NODE_SHAPES_UTILS_H
#define __NODE_SHAPES_UTILS_H

#include "kis_node_shape.h"
#include "kis_node_shapes_graph.h"
#include "kis_node_dummies_graph.h"
#include <testutil.h>

inline KisNodeSP nodeFromId(int id) {
    KisNodeSP node = new TestUtil::TestNode();
    node->setName(QString("node%1").arg(id));
    return node;
}

inline KisNodeShape* nodeShapeFromId(int id) {
    return new KisNodeShape(nodeFromId(id));
}

inline bool checkDummyId(KisNodeDummy *dummy, int id) {
    return dummy->node()->name() == QString("node%1").arg(id);
}

inline KisNodeDummy* findDummyById(KisNodeDummy *root, int id)
{
    if(checkDummyId(root, id)) return root;

    KisNodeDummy *foundDummy = 0;
    KisNodeDummy *child = root->firstChild();
    while(child) {
        foundDummy = findDummyById(child, id);
        if(foundDummy) break;
        child = child->nextSibling();
    }
    return foundDummy;
}

inline KisNodeSP findNodeById(KisNodeDummy *root, int id)
{
    KisNodeDummy *dummy = findDummyById(root, id);
    Q_ASSERT(dummy);

    return dummy->node();
}

inline QString dummyId(KisNodeDummy *dummy, const QString removePrefix) {
    QString nodeName = dummy->node()->name();

    if(!removePrefix.isEmpty()) {
        nodeName = QString::number(nodeName.remove(removePrefix).toInt());
    }

    return nodeName;
}

inline QString collectGraphPattern(KisNodeDummy *root, const QString removePrefix = "node")
{
    QString result = dummyId(root, removePrefix) + ' ';

    KisNodeDummy *child = root->firstChild();
    while(child) {
        result += collectGraphPattern(child, removePrefix) + ' ';
        child = child->nextSibling();
    }
    return result.trimmed();
}

inline QString collectGraphPatternFull(KisNodeDummy *root) {
    return collectGraphPattern(root, "");
}

QString collectGraphPatternReverse(KisNodeDummy *root)
{
    QString result;

    KisNodeDummy *child = root->lastChild();
    while(child) {
        result = collectGraphPattern(child) + ' ' + result;
        child = child->prevSibling();
    }
    result = dummyId(root, "node") + ' ' + result;

    return result.trimmed();
}

#endif /* __NODE_SHAPES_UTILS_H */
