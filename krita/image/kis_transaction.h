/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_TRANSACTION_H_
#define KIS_TRANSACTION_H_

#include <QString>
#include <QUndoCommand>

#include "kis_types.h"
#include <krita_export.h>

/**
 * A tile based undo command.
 *
 * Ordinary QUndoCommand subclasses store parameters and apply the action in
 * the redo() command, however, Krita doesn't work like this. Undo replaces
 * the current tiles in a paint device with the old tiles, redo replaces them
 * again with the new tiles without actually executing the command that changed
 * the image data again.
 */
class KRITAIMAGE_EXPORT KisTransaction : public QObject, public QUndoCommand
{

    Q_OBJECT

public:

    KisTransaction(const QString& name, KisPaintDeviceSP device, QUndoCommand* parent = 0);
    virtual ~KisTransaction();

public:
    virtual void redo();
    virtual void undo();
    virtual void undoNoUpdate();

private:
    class Private;
    Private * const m_d;
};

#endif // KIS_TILE_COMMAND_H_

