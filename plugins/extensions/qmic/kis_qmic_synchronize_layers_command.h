/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
