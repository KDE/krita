/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_count_visitor.h"
#include "kis_image.h"

bool KisCountVisitor::inList(KisNode* node)
{
    foreach(const QString& nodeType, m_nodeTypes) {
        if (node->inherits(nodeType.toAscii()))
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
