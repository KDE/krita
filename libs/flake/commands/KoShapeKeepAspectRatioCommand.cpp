/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeKeepAspectRatioCommand.h"

#include <klocalizedstring.h>

#include <KoShape.h>

KoShapeKeepAspectRatioCommand::KoShapeKeepAspectRatioCommand(const QList<KoShape *> &shapes, bool newKeepAspectRatio, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Keep Aspect Ratio"), parent)
    , m_shapes(shapes)
{
    Q_FOREACH (KoShape *shape, shapes) {
            m_oldKeepAspectRatio << shape->keepAspectRatio();
            m_newKeepAspectRatio << newKeepAspectRatio;
    }
}

KoShapeKeepAspectRatioCommand::~KoShapeKeepAspectRatioCommand()
{
}

void KoShapeKeepAspectRatioCommand::redo()
{
    KUndo2Command::redo();
    for (int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setKeepAspectRatio(m_newKeepAspectRatio[i]);
    }
}

void KoShapeKeepAspectRatioCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setKeepAspectRatio(m_oldKeepAspectRatio[i]);
    }
}
