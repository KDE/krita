/* This file is part of the KDE project
   Copyright 2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright 2011 Boudewijn Rempt <boud@valdyas.org>

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
#ifndef CHANGEVECTORDATACOMMAND_H
#define CHANGEVECTORDATACOMMAND_H

#include <kundo2command.h>
#include <QByteArray>

#include "VectorShape.h"

class ChangeVectorDataCommand : public KUndo2Command
{
public:
    ChangeVectorDataCommand(VectorShape *shape, const QByteArray &newImageData, VectorShape::VectorType newVectorType,
                            KUndo2Command *parent = 0);
    virtual ~ChangeVectorDataCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    VectorShape *m_shape;
    QByteArray m_oldImageData;
    VectorShape::VectorType m_oldVectorType;
    QByteArray m_newImageData;
    VectorShape::VectorType m_newVectorType;
};

#endif /* CHANGEVECTORDATACOMMAND_H */
