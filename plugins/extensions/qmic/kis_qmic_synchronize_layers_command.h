/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#ifndef _KIS_GMIC_SYNCHRONIZE_LAYERS_COMMAND
#define _KIS_GMIC_SYNCHRONIZE_LAYERS_COMMAND

#include <kundo2command.h>

#include <QSharedPointer>
#include <QList>

#include <kis_image.h>
#include <kis_selection.h>
#include <kis_node.h>
#include <kis_types.h>

#include "gmic.h"

class KisImageCommand;

class KisQmicSynchronizeLayersCommand : public KUndo2Command
{
public:
    KisQmicSynchronizeLayersCommand(KisNodeListSP nodes,
                                    QVector<gmic_image<float> *> images,
                                    KisImageWSP image,
                                    const QRect &dstRect = QRect(),
                                    const KisSelectionSP selection = 0
    );

    virtual ~KisQmicSynchronizeLayersCommand();

    virtual void redo();
    virtual void undo();

private:
    KisNodeListSP m_nodes;
    QVector<gmic_image<float> *> m_images;
    KisImageWSP m_image;
    QRect m_dstRect;
    KisSelectionSP m_selection;
    bool m_firstRedo;

    QVector<KisImageCommand *> m_imageCommands;
};

#endif
