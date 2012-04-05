/* This file is part of the KDE project
   Copyright 2009 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "ChangeImageCommand.h"

#include <cmath>
#include <klocale.h>
#include <KoImageData.h>
#include <KDebug>

ChangeImageCommand::ChangeImageCommand(PictureShape *shape, KoImageData *newImageData, KUndo2Command *parent):
    KUndo2Command(parent),
    m_imageChanged(true),
    m_shape(shape),
    m_newImageData(newImageData),
    m_oldSize(shape->size()),
    m_newSize(newImageData->imageSize()),
    m_oldCroppingRect(shape->cropRect()),
    m_newCroppingRect(0, 0, 1, 1),
    m_oldColorMode(shape->colorMode()),
    m_newColorMode(shape->colorMode())
{
    setText(i18nc("(qtundo-format)", "Change image"));

    // we need new here as setUserData deletes the old data
    m_oldImageData = m_shape->imageData() ? new KoImageData(*m_shape->imageData()): 0;

    qreal oldArea = m_oldSize.width() * m_oldSize.height();
    qreal newArea = m_newSize.width() * m_newSize.height();
    m_newSize *= std::sqrt(oldArea / newArea);
}

ChangeImageCommand::ChangeImageCommand(PictureShape *shape, const QRectF &croppingRect, KUndo2Command *parent):
    KUndo2Command(parent),
    m_imageChanged(false),
    m_shape(shape),
    m_oldImageData(0),
    m_newImageData(0),
    m_oldSize(shape->size()),
    m_newSize(shape->size()),
    m_oldCroppingRect(shape->cropRect()),
    m_newCroppingRect(croppingRect),
    m_oldColorMode(shape->colorMode()),
    m_newColorMode(shape->colorMode())
{
}

ChangeImageCommand::ChangeImageCommand(PictureShape *shape, PictureShape::ColorMode colorMode, KUndo2Command *parent):
    KUndo2Command(parent),
    m_imageChanged(false),
    m_shape(shape),
    m_oldImageData(0),
    m_newImageData(0),
    m_oldSize(shape->size()),
    m_newSize(shape->size()),
    m_oldCroppingRect(shape->cropRect()),
    m_newCroppingRect(shape->cropRect()),
    m_oldColorMode(shape->colorMode()),
    m_newColorMode(colorMode)
{
}

ChangeImageCommand::~ChangeImageCommand()
{
    delete m_oldImageData;
    delete m_newImageData;
}

void ChangeImageCommand::redo()
{
    if (m_imageChanged) {
        // we need new here as setUserData deletes the old data
        m_shape->setUserData(m_newImageData ? new KoImageData(*m_newImageData): 0);
    }

    m_shape->setSize(m_newSize);
    m_shape->setColorMode(m_newColorMode);
    m_shape->setCropRect(m_newCroppingRect);
    emit sigExecuted();
}

void ChangeImageCommand::undo()
{
    if (m_imageChanged) {
        // we need new here as setUserData deletes the old data
        m_shape->setUserData(m_oldImageData ? new KoImageData(*m_oldImageData): 0);
    }

    m_shape->setSize(m_oldSize);
    m_shape->setColorMode(m_oldColorMode);
    m_shape->setCropRect(m_oldCroppingRect);
    emit sigExecuted();
}
