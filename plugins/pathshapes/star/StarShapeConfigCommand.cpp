/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "StarShapeConfigCommand.h"
#include "StarShape.h"
#include <klocale.h>

StarShapeConfigCommand::StarShapeConfigCommand(StarShape * star, uint cornerCount, qreal innerRadius, qreal outerRadius, bool convex, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_star(star)
    , m_newCornerCount(cornerCount)
    , m_newInnerRadius(innerRadius)
    , m_newOuterRadius(outerRadius)
    , m_newConvex(convex)
{
    Q_ASSERT(m_star);

    setText(i18n("Change star"));

    m_oldCornerCount = m_star->cornerCount();
    m_oldInnerRadius = m_star->baseRadius();
    m_oldOuterRadius = m_star->tipRadius();
    m_oldConvex = m_star->convex();
}

void StarShapeConfigCommand::redo()
{
    QUndoCommand::redo();

    m_star->update();

    QPointF position = m_star->absolutePosition();

    if (m_oldCornerCount != m_newCornerCount)
        m_star->setCornerCount(m_newCornerCount);
    if (m_oldInnerRadius != m_newInnerRadius)
        m_star->setBaseRadius(m_newInnerRadius);
    if (m_oldOuterRadius != m_newOuterRadius)
        m_star->setTipRadius(m_newOuterRadius);
    if (m_oldConvex != m_newConvex)
        m_star->setConvex(m_newConvex);

    m_star->setAbsolutePosition(position);

    m_star->update();
}

void StarShapeConfigCommand::undo()
{
    QUndoCommand::undo();

    m_star->update();

    QPointF position = m_star->absolutePosition();

    if (m_oldCornerCount != m_newCornerCount)
        m_star->setCornerCount(m_oldCornerCount);
    if (m_oldInnerRadius != m_newInnerRadius)
        m_star->setBaseRadius(m_oldInnerRadius);
    if (m_oldOuterRadius != m_newOuterRadius)
        m_star->setTipRadius(m_oldOuterRadius);
    if (m_oldConvex != m_newConvex)
        m_star->setConvex(m_oldConvex);

    m_star->setAbsolutePosition(position);

    m_star->update();
}
