/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEKEEPASPECTRATIOCOMMAND_H
#define KOSHAPEKEEPASPECTRATIOCOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>
#include <QList>

class KoShape;

/**
 * Command that changes the keepAspectRatio property of KoShape
 */
class KRITAFLAKE_EXPORT KoShapeKeepAspectRatioCommand : public KUndo2Command
{
public:
    /**
     * Constructor
     * @param shapes the shapes affected by the command
     * @param newKeepAspectRatio the new setting
     * @param parent the parent command
     */
    KoShapeKeepAspectRatioCommand(const QList<KoShape*> &shapes, bool newKeepAspectRatio, KUndo2Command* parent = 0);
    ~KoShapeKeepAspectRatioCommand() override;

    /// Execute the command
    void redo() override;
    /// Unexecute the command
    void undo() override;

private:
    QList<KoShape*> m_shapes;
    QList<bool> m_oldKeepAspectRatio;
    QList<bool> m_newKeepAspectRatio;
};

#endif
