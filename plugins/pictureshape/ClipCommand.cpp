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
#include "ClipCommand.h"

#include <KDebug>
#include <KLocale>

ClipCommand::ClipCommand(PictureShape *shape, bool clip)
    : KUndo2Command(0)
    , m_pictureShape(shape)
    , m_clip(clip)
{
    if (clip) {
        setText(i18nc("(qtundo-format)", "Contour image (by image analysis)"));
    } else {
        setText(i18nc("(qtundo-format)", "Remove image contour"));
    }
}

ClipCommand::~ClipCommand()
{
}

void ClipCommand::redo()
{
    if (m_clip) {
        m_pictureShape->setClipPath(m_pictureShape->generateClipPath());
    } else {
        m_pictureShape->setClipPath(0);
    }
    m_pictureShape->update();
}

void ClipCommand::undo()
{
    if (m_clip) {
        m_pictureShape->setClipPath(0);
    } else {
        m_pictureShape->setClipPath(m_pictureShape->generateClipPath());
    }
    m_pictureShape->update();
}
