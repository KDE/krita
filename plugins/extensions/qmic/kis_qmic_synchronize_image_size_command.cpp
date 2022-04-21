/*
 * SPDX-FileCopyrightText: 2015 Lukáš Tvrdý <lukast.dev@gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_qmic_synchronize_image_size_command.h"
#include <commands_new/kis_image_resize_command.h>

KisQmicSynchronizeImageSizeCommand::KisQmicSynchronizeImageSizeCommand(QVector<gmic_image<float> *> images, KisImageWSP image)
    : m_images(images)
    , m_image(image)
    , m_resizeCommand(0)
{
    dbgPlugins << "KisQmicSynchronizeImageSizeCommand" << "gmic images" << m_images.size();
}

KisQmicSynchronizeImageSizeCommand::~KisQmicSynchronizeImageSizeCommand()
{
    delete m_resizeCommand;
}


void KisQmicSynchronizeImageSizeCommand::redo()
{
    dbgPlugins << "KisQmicSynchronizeImageSizeCommand::redo";
    // sync image size
    if (m_image)
    {
        QSize gmicBoundingLayerSize = findMaxLayerSize(m_images);
        QSize kritaSize = m_image->size();

        dbgPlugins << "\tkrita image" << kritaSize << "gmic size" << gmicBoundingLayerSize;

        if (gmicBoundingLayerSize != kritaSize)
        {
            QSize newSize = kritaSize.expandedTo(gmicBoundingLayerSize);
            dbgPlugins << "G'Mic changes layer size from " << kritaSize << " to " << newSize;
            m_resizeCommand = new KisImageResizeCommand(m_image, newSize);
            m_resizeCommand->redo();
        }
    }
}

void KisQmicSynchronizeImageSizeCommand::undo()
{
    dbgPlugins << "KisQmicSynchronizeImageSizeCommand::undo";
    if (m_resizeCommand) {
        m_resizeCommand->undo();
    }
}


QSize KisQmicSynchronizeImageSizeCommand::findMaxLayerSize(QVector<gmic_image<float> *> images)
{
    // synchronize image size
    int maxWidth = 0;
    int maxHeight = 0;
    for (int i = 0; i < images.size(); i++) {
        gmic_image<float> *gimg = images[i];
        int width = gimg->_width;
        maxWidth = qMax(width, maxWidth);

        int height = gimg->_height;
        maxHeight = qMax(height, maxHeight);
    }

    dbgPlugins << "MaxLayerSize" << maxWidth << maxHeight;
    return QSize(maxWidth, maxHeight);
}
