/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBezierMesh.h"

#include <QDebug>

#include "kis_dom_utils.h"

QDebug KisBezierMeshDetails::operator<<(QDebug dbg, const BaseMeshNode &n) {
    dbg.nospace() << "Node " << n.node << " "
                  << "(lC: " << n.leftControl << " "
                  << "tC: " << n.topControl << " "
                  << "rC: " << n.rightControl << " "
                  << "bC: " << n.bottomControl << ") ";
    return dbg.nospace();
}



void KisBezierMeshDetails::saveValue(QDomElement *parent, const QString &tag, const KisBezierMeshDetails::BaseMeshNode &node)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "mesh-node");
    KisDomUtils::saveValue(&e, "node", node.node);
    KisDomUtils::saveValue(&e, "left-control", node.leftControl);
    KisDomUtils::saveValue(&e, "right-control", node.rightControl);
    KisDomUtils::saveValue(&e, "top-control", node.topControl);
    KisDomUtils::saveValue(&e, "bottom-control", node.bottomControl);
}

bool KisBezierMeshDetails::loadValue(const QDomElement &parent, KisBezierMeshDetails::BaseMeshNode *node)
{
    if (!KisDomUtils::Private::checkType(parent, "mesh-node")) return false;

    KisDomUtils::loadValue(parent, "node", &node->node);
    KisDomUtils::loadValue(parent, "left-control", &node->leftControl);
    KisDomUtils::loadValue(parent, "right-control", &node->rightControl);
    KisDomUtils::loadValue(parent, "top-control", &node->topControl);
    KisDomUtils::loadValue(parent, "bottom-control", &node->bottomControl);

    return true;
}
