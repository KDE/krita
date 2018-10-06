/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "ShapeGradientEditStrategy.h"

#include <KoToolBase.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include "kis_assert.h"
#include "SelectionDecorator.h"
#include <kundo2command.h>
#include <KoSnapGuide.h>
#include <KisSnapPointStrategy.h>

#include "kis_debug.h"


struct ShapeGradientEditStrategy::Private
{
    Private(const QPointF &_start, KoShape *shape, KoFlake::FillVariant fillVariant)
        : start(_start),
          gradientHandles(fillVariant, shape)
    {
    }

    QPointF start;
    QPointF initialOffset;
    KoShapeGradientHandles gradientHandles;
    KoShapeGradientHandles::Handle::Type handleType;
    QScopedPointer<KUndo2Command> intermediateCommand;
};


ShapeGradientEditStrategy::ShapeGradientEditStrategy(KoToolBase *tool,
                                                     KoFlake::FillVariant fillVariant,
                                                     KoShape *shape,
                                                     KoShapeGradientHandles::Handle::Type startHandleType,
                                                     const QPointF &clicked)
    : KoInteractionStrategy(tool)
    , m_d(new Private(clicked, shape, fillVariant))
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);

    m_d->handleType = startHandleType;

    KoShapeGradientHandles::Handle handle = m_d->gradientHandles.getHandle(m_d->handleType);
    m_d->initialOffset = handle.pos - clicked;

    KisSnapPointStrategy *strategy = new KisSnapPointStrategy();
    Q_FOREACH (const KoShapeGradientHandles::Handle &h, m_d->gradientHandles.handles()) {
        strategy->addPoint(h.pos);
    }
    tool->canvas()->snapGuide()->addCustomSnapStrategy(strategy);
}

ShapeGradientEditStrategy::~ShapeGradientEditStrategy()
{
}

void ShapeGradientEditStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    if (m_d->intermediateCommand) {
        m_d->intermediateCommand->undo();
        m_d->intermediateCommand.reset();
    }

    const QPointF snappedPosition = tool()->canvas()->snapGuide()->snap(mouseLocation, m_d->initialOffset, modifiers);
    const QPointF diff = snappedPosition- m_d->start;
    m_d->intermediateCommand.reset(m_d->gradientHandles.moveGradientHandle(m_d->handleType, diff));
    m_d->intermediateCommand->redo();

    tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
}

KUndo2Command *ShapeGradientEditStrategy::createCommand()
{
    return m_d->intermediateCommand.take();
}

void ShapeGradientEditStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    const QRectF dirtyRect = tool()->canvas()->snapGuide()->boundingRect();
    tool()->canvas()->snapGuide()->reset();
    tool()->canvas()->updateCanvas(dirtyRect);
}

void ShapeGradientEditStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

