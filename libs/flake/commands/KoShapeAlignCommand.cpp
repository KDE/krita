/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeAlignCommand.h"
#include "KoShape.h"
#include "KoShapeGroup.h"
#include "commands/KoShapeMoveCommand.h"

#include <klocalizedstring.h>
// #include <FlakeDebug.h>

class Q_DECL_HIDDEN KoShapeAlignCommand::Private
{
public:
    Private() : command(0) {}
    ~Private() {
        delete command;
    }
    KoShapeMoveCommand *command;
};

KoShapeAlignCommand::KoShapeAlignCommand(const QList<KoShape*> &shapes, Align align, const QRectF &boundingRect, KUndo2Command *parent)
        : KUndo2Command(parent),
        d(new Private())
{
    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    QPointF position;
    QPointF delta;
    QRectF bRect;
    Q_FOREACH (KoShape *shape, shapes) {
//   if (dynamic_cast<KoShapeGroup*> (shape))
//       debugFlake <<"Found Group";
//   else if (dynamic_cast<KoShapeContainer*> (shape))
//       debugFlake <<"Found Container";
//   else
//       debugFlake <<"Found shape";
        position = shape->absolutePosition();
        previousPositions  << position;
        bRect = shape->absoluteOutlineRect();
        switch (align) {
        case HorizontalLeftAlignment:
            delta = QPointF(boundingRect.left(), bRect.y()) - bRect.topLeft();
            break;
        case HorizontalCenterAlignment:
            delta = QPointF(boundingRect.center().x() - bRect.width() / 2, bRect.y()) - bRect.topLeft();
            break;
        case HorizontalRightAlignment:
            delta = QPointF(boundingRect.right() - bRect.width(), bRect.y()) - bRect.topLeft();
            break;
        case VerticalTopAlignment:
            delta = QPointF(bRect.x(), boundingRect.top()) - bRect.topLeft();
            break;
        case VerticalCenterAlignment:
            delta = QPointF(bRect.x(), boundingRect.center().y() - bRect.height() / 2) - bRect.topLeft();
            break;
        case VerticalBottomAlignment:
            delta = QPointF(bRect.x(), boundingRect.bottom() - bRect.height()) - bRect.topLeft();
            break;
        };
        newPositions  << position + delta;
//debugFlake <<"-> moving" <<  position.x() <<"," << position.y() <<" to" <<
//        (position + delta).x() << ", " << (position+delta).y() << endl;
    }
    d->command = new KoShapeMoveCommand(shapes, previousPositions, newPositions);

    setText(kundo2_i18n("Align shapes"));
}

KoShapeAlignCommand::~KoShapeAlignCommand()
{
    delete d;
}

void KoShapeAlignCommand::redo()
{
    KUndo2Command::redo();
    d->command->redo();
}

void KoShapeAlignCommand::undo()
{
    KUndo2Command::undo();
    d->command->undo();
}
