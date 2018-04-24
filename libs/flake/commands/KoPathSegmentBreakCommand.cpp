/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPathSegmentBreakCommand.h"
#include "KoPathPoint.h"
#include <klocalizedstring.h>
#include <math.h>

KoPathSegmentBreakCommand::KoPathSegmentBreakCommand(const KoPathPointData & pointData, KUndo2Command *parent)
        : KUndo2Command(parent)
        , m_pointData(pointData)
        , m_startIndex(-1, -1)
        , m_broken(false)
{
    if (m_pointData.pathShape->isClosedSubpath(m_pointData.pointIndex.first)) {
        m_startIndex = m_pointData.pointIndex;

        const int numPoints = m_pointData.pathShape->subpathPointCount(m_startIndex.first);
        m_startIndex.second = (m_startIndex.second + 1) % numPoints;
    }
    setText(kundo2_i18n("Break subpath"));
}

KoPathSegmentBreakCommand::~KoPathSegmentBreakCommand()
{
}

void KoPathSegmentBreakCommand::redo()
{
    KUndo2Command::redo();
    // a repaint before is needed as the shape can shrink during the break
    m_pointData.pathShape->update();
    if (m_startIndex.first != -1) {
        m_startIndex = m_pointData.pathShape->openSubpath(m_startIndex);
        m_pointData.pathShape->normalize();
        m_pointData.pathShape->update();
    } else {
        m_broken = m_pointData.pathShape->breakAfter(m_pointData.pointIndex);
        if (m_broken) {
            m_pointData.pathShape->normalize();
            m_pointData.pathShape->update();
        }
    }
}

void KoPathSegmentBreakCommand::undo()
{
    KUndo2Command::undo();
    if (m_startIndex.first != -1) {
        m_startIndex = m_pointData.pathShape->closeSubpath(m_startIndex);
        m_pointData.pathShape->normalize();
        m_pointData.pathShape->update();
    } else if (m_broken) {
        m_pointData.pathShape->join(m_pointData.pointIndex.first);
        m_pointData.pathShape->normalize();
        m_pointData.pathShape->update();
    }
}
