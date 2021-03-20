/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KoShapeGradientHandles::Handle::Type handleType {KoShapeGradientHandles::Handle::Type::None};
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
}

KUndo2Command *ShapeGradientEditStrategy::createCommand()
{
    return m_d->intermediateCommand.take();
}

void ShapeGradientEditStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    tool()->canvas()->snapGuide()->reset();
}

void ShapeGradientEditStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

