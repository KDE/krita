/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "StarShapeConfigCommand.h"
#include "StarShape.h"
#include <klocalizedstring.h>

StarShapeConfigCommand::StarShapeConfigCommand(StarShape *star, uint cornerCount, qreal innerRadius, qreal outerRadius, bool convex, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_star(star)
    , m_newCornerCount(cornerCount)
    , m_newInnerRadius(innerRadius)
    , m_newOuterRadius(outerRadius)
    , m_newConvex(convex)
{
    Q_ASSERT(m_star);

    setText(kundo2_i18n("Change star"));

    m_oldCornerCount = m_star->cornerCount();
    m_oldInnerRadius = m_star->baseRadius();
    m_oldOuterRadius = m_star->tipRadius();
    m_oldConvex = m_star->convex();
}

void StarShapeConfigCommand::redo()
{
    KUndo2Command::redo();

    m_star->update();

    QPointF position = m_star->absolutePosition();

    if (m_oldCornerCount != m_newCornerCount) {
        m_star->setCornerCount(m_newCornerCount);
    }
    if (m_oldInnerRadius != m_newInnerRadius) {
        m_star->setBaseRadius(m_newInnerRadius);
    }
    if (m_oldOuterRadius != m_newOuterRadius) {
        m_star->setTipRadius(m_newOuterRadius);
    }
    if (m_oldConvex != m_newConvex) {
        m_star->setConvex(m_newConvex);
    }

    m_star->setAbsolutePosition(position);

    m_star->update();
}

void StarShapeConfigCommand::undo()
{
    KUndo2Command::undo();

    m_star->update();

    QPointF position = m_star->absolutePosition();

    if (m_oldCornerCount != m_newCornerCount) {
        m_star->setCornerCount(m_oldCornerCount);
    }
    if (m_oldInnerRadius != m_newInnerRadius) {
        m_star->setBaseRadius(m_oldInnerRadius);
    }
    if (m_oldOuterRadius != m_newOuterRadius) {
        m_star->setTipRadius(m_oldOuterRadius);
    }
    if (m_oldConvex != m_newConvex) {
        m_star->setConvex(m_oldConvex);
    }

    m_star->setAbsolutePosition(position);

    m_star->update();
}
