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

#include <QObject>
#include <kundo2command.h>
#include "PictureShape.h"

class ChangeImageCommand : public QObject, public KUndo2Command
{
    Q_OBJECT

public:
    ChangeImageCommand(PictureShape *shape, KoImageData *newImageData, KUndo2Command *parent=0);
    ChangeImageCommand(PictureShape *shape, const QRectF &croppingRect, KUndo2Command *parent=0);
    ChangeImageCommand(PictureShape *shape, PictureShape::ColorMode colorMode, KUndo2Command *parent=0);
    virtual ~ChangeImageCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

signals:
    void sigExecuted();

private:
    bool m_imageChanged;
    PictureShape *m_shape;
    KoImageData *m_oldImageData;
    KoImageData *m_newImageData;
    QRectF m_oldCroppingRect;
    QRectF m_newCroppingRect;
    PictureShape::ColorMode m_oldColorMode;
    PictureShape::ColorMode m_newColorMode;
};

#endif /* CHANGEIMAGECOMMAND_H */
