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
