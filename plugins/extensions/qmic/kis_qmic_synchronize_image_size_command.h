/*
 * SPDX-FileCopyrightText: 2015 Lukáš Tvrdý <lukast.dev@gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GMIC_SYNCHRONIZE_IMAGE_SIZE_COMMAND_H
#define KIS_GMIC_SYNCHRONIZE_IMAGE_SIZE_COMMAND_H

#include <QSharedPointer>


#include <kundo2command.h>

#include <kis_image.h>
#include <kis_types.h>

#include "gmic.h"

class KisImageResizeCommand;

class KisQmicSynchronizeImageSizeCommand : public KUndo2Command
{
public:
    KisQmicSynchronizeImageSizeCommand(QVector<gmic_image<float> *> images, KisImageWSP image);
    ~KisQmicSynchronizeImageSizeCommand();

    void redo();
    void undo();

private:
    QSize findMaxLayerSize(QVector<gmic_image<float> *> images);

private:
    QVector<gmic_image<float> *> m_images;
    KisImageWSP m_image;
    KisImageResizeCommand *m_resizeCommand;

};

#endif
