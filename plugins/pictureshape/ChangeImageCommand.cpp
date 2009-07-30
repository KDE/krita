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

#include <math.h>
#include <klocale.h>
#include <KoImageData.h>
#include <KDebug>

#include "PictureShape.h"

ChangeImageCommand::ChangeImageCommand(PictureShape *shape, KoImageData *newImageData, QUndoCommand *parent)
: QUndoCommand(parent)
, m_shape(shape)
, m_oldImageData(0)
, m_newImageData(newImageData)
{
    Q_ASSERT( shape );
    KoImageData *oldImageData = qobject_cast<KoImageData*>(m_shape->userData());
    // we need new here as setUserData deletes the old data
    m_oldImageData = oldImageData ? new KoImageData( *oldImageData ): 0;
    setText(i18n("Change image"));

    m_oldSize = shape->size();
    m_newSize = newImageData->imageSize();
    qreal oldarea = m_oldSize.width() * m_oldSize.height();
    qreal newarea = m_newSize.width() * m_newSize.height();
    m_newSize *= sqrt( oldarea / newarea );
}

ChangeImageCommand::~ChangeImageCommand()
{
    delete m_oldImageData;
    delete m_newImageData;
}

void ChangeImageCommand::redo()
{
    m_shape->update();
    // we need new here as setUserData deletes the old data
    m_shape->setUserData( m_newImageData ? new KoImageData( *m_newImageData ): 0 );
    m_shape->setSize( m_newSize );
    m_shape->update();
}

void ChangeImageCommand::undo()
{
    // we need new here as setUserData deletes the old data
    m_shape->update();
    m_shape->setUserData( m_oldImageData ? new KoImageData( *m_oldImageData ): 0 );
    m_shape->setSize( m_oldSize );
    m_shape->update();
}
