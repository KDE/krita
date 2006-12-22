/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoShapeAlignCommand.h"
#include "KoShape.h"
#include "KoShapeGroup.h"
#include "commands/KoShapeMoveCommand.h"

#include <klocale.h>
// #include <kdebug.h>

KoShapeAlignCommand::KoShapeAlignCommand( const KoSelectionSet &shapes, Align align, QRectF boundingRect,
                                          QUndoCommand *parent )
: QUndoCommand( parent )
{
    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    QPointF position;
    QPointF delta;
    QRectF bRect;
    foreach( KoShape *shape, shapes ) {
//   if(dynamic_cast<KoShapeGroup*> (shape))
//       kDebug() << "Found Group\n";
//   else if(dynamic_cast<KoShapeContainer*> (shape))
//       kDebug() << "Found Container\n";
//   else
//       kDebug() << "Found shape\n";
        position = shape->position();
        previousPositions  << position;
        bRect = shape->boundingRect();
        switch( align )
        {
            case HorizontalLeftAlignment:
                delta = QPointF( boundingRect.left(), bRect.y()) - bRect.topLeft();
                break;
            case HorizontalCenterAlignment:
                delta = QPointF( boundingRect.center().x() - bRect.width()/2, bRect.y()) - bRect.topLeft();
                break;
            case HorizontalRightAlignment:
                delta = QPointF( boundingRect.right() - bRect.width(), bRect.y()) - bRect.topLeft();
                break;
            case VerticalTopAlignment:
                delta = QPointF( bRect.x(), boundingRect.top()) - bRect.topLeft();
                break;
            case VerticalCenterAlignment:
                delta = QPointF(  bRect.x(), boundingRect.center().y() - bRect.height()/2) - bRect.topLeft();
                break;
            case VerticalBottomAlignment:
                delta = QPointF(  bRect.x(), boundingRect.bottom() - bRect.height()) - bRect.topLeft();
                break;
        };
        newPositions  << position + delta;
//kDebug() << "-> moving " <<  position.x() << "," << position.y() << " to " <<
//        (position + delta).x() << ", " << (position+delta).y() << endl;
    }
    m_command = new KoShapeMoveCommand(shapes, previousPositions, newPositions);

    setText( i18n( "Align shapes" ) );
}

KoShapeAlignCommand::~KoShapeAlignCommand()
{
    delete m_command;
}

void KoShapeAlignCommand::redo()
{
    m_command->redo();
}

void KoShapeAlignCommand::undo()
{
    m_command->undo();
}
