/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_count_visitor.h"
#include "kis_image.h"

bool KisCountVisitor::inList(KisNode* node)
{
    Q_FOREACH (const QString& nodeType, m_nodeTypes) {
        if (node->inherits(nodeType.toLatin1()))
            return true;
    }
    return false;
}

bool KisCountVisitor::check(KisNode * node)
{
    if (m_nodeTypes.isEmpty() || inList(node)) {

        if (m_properties.isEmpty() || node->check(m_properties)) {
            m_count++;
        }
    }
    visitAll(node);

    return true;
}
