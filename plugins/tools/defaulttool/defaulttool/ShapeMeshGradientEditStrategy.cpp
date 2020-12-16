/*
 *  SPDX-FileCopyrightText: 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
