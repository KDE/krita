/* This file is part of the KDE project

   Copyright 2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright 2012 Inge Wallin <inge@lysator.liu.se>

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

// Own
#include "ChangeSomethingCommand.h"

// KDE
#include <klocale.h>
#include <KDebug>

// Calligra
#include <KoSomethingData.h>

#include "TemplateShape.h"


ChangeSomethingCommand::ChangeSomethingCommand(TemplateShape *shape, KoSomethingData *newSomethingData,
                                               KUndo2Command *parent)
  : KUndo2Command(parent)
  , m_shape(shape)
  , m_oldSomethingData(0)
  , m_newSomethingData(newSomethingData)
{
    Q_ASSERT(shape);
    KoSomethingData *oldSomethingData = qobject_cast<KoSomethingData*>(m_shape->userData());

    // We need new here as setUserData deletes the old data.
    m_oldSomethingData = oldSomethingData ? new KoSomethingData(*oldSomethingData): 0;
    setText(i18nc("(qtundo-format)", "Change something"));

    m_oldSize = shape->size();
    m_newSize = newSomethingData->somethingSize();
    qreal oldarea = m_oldSize.width() * m_oldSize.height();
    qreal newarea = m_newSize.width() * m_newSize.height();
    m_newSize *= sqrt(oldarea / newarea);
}

ChangeSomethingCommand::~ChangeSomethingCommand()
{
    delete m_oldSomethingData;
    delete m_newSomethingData;
}

void ChangeSomethingCommand::redo()
{
    m_shape->update();

    // We need new here as setUserData deletes the old data
    m_shape->setUserData(m_newSomethingData ? new KoSomethingData(*m_newSomethingData): 0);
    m_shape->setSize(m_newSize);
    m_shape->update();
}

void ChangeSomethingCommand::undo()
{
    // We need new here as setUserData deletes the old data
    m_shape->update();
    m_shape->setUserData(m_oldSomethingData ? new KoSomethingData(*m_oldSomethingData): 0);
    m_shape->setSize(m_oldSize);
    m_shape->update();
}
