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
#ifndef KIS_LAYER_COMMAND_H_
#define KIS_LAYER_COMMAND_H_

#include <QUndoCommand>
#include <krita_export.h>
#include "kis_types.h"

class KisLayer;

/// the base command for commands altering a layer
class KRITAIMAGE_EXPORT KisLayerCommand : public QUndoCommand
{

public:
    /**
     * Constructor
     * @param name The name that will be shown in the ui
     * @param layer The layer the command will be working on.
     */
    KisLayerCommand(const QString& name, KisLayerSP layer);
    virtual ~KisLayerCommand();

protected:
    KisLayerSP m_layer;
};
#endif
