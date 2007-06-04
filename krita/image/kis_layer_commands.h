/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_LAYER_COMMANDS_H_
#define KIS_LAYER_COMMANDS_H_

#include <krita_export.h>

#include <QUndoCommand>

#include "kis_types.h"

class KoCompositeOp;
class KisLayer;

/// the base command for commands altering a layer
class KRITAIMAGE_EXPORT KisLayerCommand : public QUndoCommand {
    typedef QUndoCommand super;

public:
    /**
     * Constructor
     * @param name The name that will be shown in the ui
     * @param layer The layer the command will be working on.
     */
    KisLayerCommand(const QString& name, KisLayerSP layer);
    virtual ~KisLayerCommand() {}

protected:
    KisLayerSP m_layer;
};


/// The command for setting the layer opacity
class KRITAIMAGE_EXPORT KisLayerOpacityCommand : public KisLayerCommand {
    typedef KisLayerCommand super;

public:
    /**
     * Constructor
     * @param layer The layer the command will be working on.
     * @param oldOpacity the old layer opacity
     * @param newOpacity the new layer opacity
     */
    KisLayerOpacityCommand(KisLayerSP layer, quint8 oldOpacity, quint8 newOpacity);

    virtual void redo();
    virtual void undo();

private:
    quint8 m_oldOpacity;
    quint8 m_newOpacity;
};


/// The command for setting the composite op
class KRITAIMAGE_EXPORT KisLayerCompositeOpCommand : public KisLayerCommand {
    typedef KisLayerCommand super;

public:
    /**
     * Constructor
     * @param layer The layer the command will be working on.
     * @param oldCompositeOp the old layer composite op
     * @param newCompositeOp the new layer composite op
     */
    KisLayerCompositeOpCommand(KisLayerSP layer, const KoCompositeOp * oldCompositeOp, const KoCompositeOp * newCompositeOp);

    virtual void redo();
    virtual void undo();

private:
    const KoCompositeOp * m_oldCompositeOp;
    const KoCompositeOp * m_newCompositeOp;
};


/// The command for moving of a layer
class KRITAIMAGE_EXPORT KisLayerMoveCommand : public KisLayerCommand {
    typedef KisLayerCommand super;

public:
    /**
     * Constructor
     * @param layer The layer the command will be working on.
     * @param oldpos the old layer position
     * @param newpos the new layer position
     */
    KisLayerMoveCommand(KisLayerSP layer, const QPoint& oldpos, const QPoint& newpos);
    virtual ~KisLayerMoveCommand();

    virtual void redo();
    virtual void undo();

private:
    void moveTo(const QPoint& pos);

private:
    QRect m_updateRect;
    QPoint m_oldPos;
    QPoint m_newPos;
};

#endif // KIS_LAYER_COMMANDS_H_
