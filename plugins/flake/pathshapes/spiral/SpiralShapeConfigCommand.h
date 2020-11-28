/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Rob Buis <buis@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SPIRALSHAPECONFIGCOMMAND_H
#define SPIRALSHAPECONFIGCOMMAND_H

#include "SpiralShape.h"
#include <kundo2command.h>

/// The undo / redo command for configuring a spiral shape
class SpiralShapeConfigCommand : public KUndo2Command
{
public:
    /**
     * Configures an spiral shape
     * @param spiral the spiral shape to configure
     * @param type the spiral type
     * @param fade the fade parameter
     * @param parent the optional parent command
     */
    SpiralShapeConfigCommand(SpiralShape *spiral, SpiralShape::SpiralType type, bool clockWise, qreal fade, KUndo2Command *parent = 0);
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;
private:
    SpiralShape *m_spiral;
    SpiralShape::SpiralType m_oldType;
    bool m_oldClockWise;
    qreal m_oldFade;
    SpiralShape::SpiralType m_newType;
    bool m_newClockWise;
    qreal m_newFade;
};

#endif // SPIRALSHAPECONFIGCOMMAND_H

