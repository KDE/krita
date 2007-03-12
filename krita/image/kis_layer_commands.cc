/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#include <klocale.h>

#include "KoCompositeOp.h"

#include "kis_layer_commands.h"
#include "kis_layer.h"

KisLayerCommand::KisLayerCommand(const QString& name, KisLayerSP layer) :
    super(name), m_layer(layer)
{
}



KisLayerOpacityCommand::KisLayerOpacityCommand(KisLayerSP layer, quint8 oldOpacity, quint8 newOpacity) :
    super(i18n("Layer Opacity"), layer)
{
    m_oldOpacity = oldOpacity;
    m_newOpacity = newOpacity;
}

void KisLayerOpacityCommand::redo()
{
    m_layer->setOpacity(m_newOpacity);
}

void KisLayerOpacityCommand::undo()
{
    m_layer->setOpacity(m_oldOpacity);
}



KisLayerCompositeOpCommand::KisLayerCompositeOpCommand(KisLayerSP layer, const KoCompositeOp* oldCompositeOp,
                                const KoCompositeOp* newCompositeOp) :
    super(i18n("Layer Composite Mode"), layer)
{
    m_oldCompositeOp = oldCompositeOp;
    m_newCompositeOp = newCompositeOp;
}

void KisLayerCompositeOpCommand::redo()
{
    m_layer->setCompositeOp(m_newCompositeOp);
}

void KisLayerCompositeOpCommand::undo()
{
    m_layer->setCompositeOp(m_oldCompositeOp);
}



KisLayerMoveCommand::KisLayerMoveCommand(KisLayerSP layer, const QPoint& oldpos, const QPoint& newpos) :
    super(i18n("Move Layer"), layer)
{
    m_oldPos = oldpos;
    m_newPos = newpos;

    QRect currentBounds = m_layer->exactBounds();
    QRect oldBounds = currentBounds;
    oldBounds.translate(oldpos.x() - newpos.x(), oldpos.y() - newpos.y());

    m_updateRect = currentBounds | oldBounds;
}

KisLayerMoveCommand::~KisLayerMoveCommand()
{
}

void KisLayerMoveCommand::redo()
{
    moveTo(m_newPos);
}

void KisLayerMoveCommand::undo()
{
    moveTo(m_oldPos);
}

void KisLayerMoveCommand::moveTo(const QPoint& pos)
{
    m_layer->setX(pos.x());
    m_layer->setY(pos.y());

    m_layer->setDirty(m_updateRect);
}
