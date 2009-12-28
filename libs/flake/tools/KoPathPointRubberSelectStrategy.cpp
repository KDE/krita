/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPathPointRubberSelectStrategy.h"
#include "KoShapeRubberSelectStrategy_p.h"

#include "KoCanvasBase.h"
#include "KoPathTool.h"
#include "KoPathToolSelection.h"

KoPathPointRubberSelectStrategy::KoPathPointRubberSelectStrategy(KoPathTool *tool, const QPointF &clicked)
        : KoShapeRubberSelectStrategy(tool, clicked)
        , m_tool(tool)
{
}

void KoPathPointRubberSelectStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_D(KoShapeRubberSelectStrategy);
    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
    if (! selection)
        return;

    selection->selectPoints(d->selectedRect(), !(modifiers & Qt::ControlModifier));
    m_tool->canvas()->updateCanvas(d->selectedRect().normalized());
    selection->repaint();
}
