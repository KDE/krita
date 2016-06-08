/*
 * Copyright (c) 2015 Lukáš Tvrdý <lukast.dev@gmail.com
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

#include "kis_gmic_synchronize_image_size_command.h"
#include <commands_new/kis_image_resize_command.h>

KisGmicSynchronizeImageSizeCommand::KisGmicSynchronizeImageSizeCommand(QSharedPointer< cimg_library::CImgList< float > > images, KisImageWSP image)
    :    m_images(images),
         m_image(image),
         m_resizeCommand(0)
{
}

KisGmicSynchronizeImageSizeCommand::~KisGmicSynchronizeImageSizeCommand()
{
    delete m_resizeCommand;
}


void KisGmicSynchronizeImageSizeCommand::redo()
{
    // sync image size
    if (m_image)
    {
        QSize gmicBoundingLayerSize = findMaxLayerSize(m_images);
        QSize kritaSize = m_image->size();

        if (kritaSize != gmicBoundingLayerSize)
        {
            dbgPlugins << "G'Mic resizes Krita canvas from " << kritaSize << " to " << gmicBoundingLayerSize;
            m_resizeCommand = new KisImageResizeCommand(m_image, gmicBoundingLayerSize);
            m_resizeCommand->redo();
        }
    }
}

void KisGmicSynchronizeImageSizeCommand::undo()
{
    if (m_resizeCommand)
    {
        m_resizeCommand->undo();
    }
}


QSize KisGmicSynchronizeImageSizeCommand::findMaxLayerSize(QSharedPointer< gmic_list<float> > images)
{
    // synchronize image size
    int maxWidth = 0;
    int maxHeight = 0;
    for (unsigned int i = 0; i < images->_width; i++)
    {
        int width = images->_data[i].width();
        maxWidth = qMax(width, maxWidth);

        int height = images->_data[i].height();
        maxHeight = qMax(height, maxHeight);
    }

    return QSize(maxWidth, maxHeight);
}
