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
#ifndef CHANGEIMAGECOMMAND_H
#define CHANGEIMAGECOMMAND_H

#include <QUndoCommand>

#include <QSizeF>

class KoImageData;
class PictureShape;

class ChangeImageCommand : public QUndoCommand
{
public:
    ChangeImageCommand(PictureShape *shape, KoImageData *newImageData, QUndoCommand *parent = 0);
    virtual ~ChangeImageCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    PictureShape * m_shape;
    KoImageData * m_oldImageData;
    KoImageData * m_newImageData;
    QSizeF m_oldSize;
    QSizeF m_newSize;
};

#endif /* CHANGEIMAGECOMMAND_H */
