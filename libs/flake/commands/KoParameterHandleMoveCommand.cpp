/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoParameterHandleMoveCommand.h"
#include "KoParameterShape.h"
#include <klocalizedstring.h>
#include "kis_command_ids.h"

KoParameterHandleMoveCommand::KoParameterHandleMoveCommand(KoParameterShape *shape, int handleId, const QPointF &startPoint, const QPointF &endPoint, Qt::KeyboardModifiers keyModifiers, KUndo2Command *parent)
        : KUndo2Command(parent)
        , m_shape(shape)
        , m_handleId(handleId)
        , m_startPoint(startPoint)
        , m_endPoint(endPoint)
        , m_keyModifiers(keyModifiers)
{
    setText(kundo2_i18n("Change parameter"));
}

KoParameterHandleMoveCommand::~KoParameterHandleMoveCommand()
{
}

/// redo the command
void KoParameterHandleMoveCommand::redo()
{
    KUndo2Command::redo();
    m_shape->update();
    m_shape->moveHandle(m_handleId, m_endPoint, m_keyModifiers);
    m_shape->update();
}

/// revert the actions done in redo
void KoParameterHandleMoveCommand::undo()
{
    KUndo2Command::undo();
    m_shape->update();
    m_shape->moveHandle(m_handleId, m_startPoint);
    m_shape->update();
}

int KoParameterHandleMoveCommand::id() const
{
    return KisCommandUtils::ChangeShapeParameterId;
}

bool KoParameterHandleMoveCommand::mergeWith(const KUndo2Command *command)
{
    const KoParameterHandleMoveCommand *other = dynamic_cast<const KoParameterHandleMoveCommand*>(command);

    if (!other ||
        other->m_shape != m_shape ||
        other->m_handleId != m_handleId ||
        other->m_keyModifiers != m_keyModifiers) {

        return false;
    }

    m_endPoint = other->m_endPoint;

    return true;
}

