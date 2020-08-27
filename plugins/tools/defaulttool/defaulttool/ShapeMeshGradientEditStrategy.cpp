/*
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
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

#include "ShapeMeshGradientEditStrategy.h"

#include <KoToolBase.h>
#include <KoCanvasBase.h>

struct ShapeMeshGradientEditStrategy::Private {
    Private(const QPointF& start, KoShape *shape, KoFlake::FillVariant fillVariant)
        : start(start)
        , handles(fillVariant, shape)
    {
    }

    // TODO: for snapping..
    QPointF start;
    KoShapeMeshGradientHandles::Handle startHandle;
    KoShapeMeshGradientHandles handles;
};

ShapeMeshGradientEditStrategy::ShapeMeshGradientEditStrategy(KoToolBase *tool,
                                                             KoFlake::FillVariant fillVariant,
                                                             KoShape *shape,
                                                             KoShapeMeshGradientHandles::Handle startHandle,
                                                             const QPointF &clicked)
    : KoInteractionStrategy(tool)
    , m_d(new Private(clicked, shape, fillVariant))
{
    m_d->startHandle = startHandle;
}

ShapeMeshGradientEditStrategy::~ShapeMeshGradientEditStrategy()
{
}

void ShapeMeshGradientEditStrategy::handleMouseMove(const QPointF &mouseLocation,
                                                    Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    tool()->canvas()->addCommand(m_d->handles.moveGradientHandle(m_d->startHandle, mouseLocation));
}

KUndo2Command* ShapeMeshGradientEditStrategy::createCommand()
{
    return nullptr;
}

void ShapeMeshGradientEditStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)
}
