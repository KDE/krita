/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef STARSHAPECONFIGCOMMAND_H
#define STARSHAPECONFIGCOMMAND_H

#include <kundo2command.h>

class StarShape;

/// The undo / redo command for configuring a star shape
class StarShapeConfigCommand : public KUndo2Command
{
public:
    /**
     * Configures a star shape
     * @param star the star shape to configure
     * @param cornerCount the number of corners to set
     * @param innerRadius the inner radius
     * @param outerRadius the outer radius
     * @param convex indicates whether the star is convex or not
     * @param parent the optional parent command
     */
    StarShapeConfigCommand(StarShape *star, uint cornerCount, qreal innerRadius, qreal outerRadius, bool convex, KUndo2Command *parent = 0);
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;
private:
    StarShape *m_star;
    uint m_oldCornerCount;
    qreal m_oldInnerRadius;
    qreal m_oldOuterRadius;
    bool m_oldConvex;
    uint m_newCornerCount;
    qreal m_newInnerRadius;
    qreal m_newOuterRadius;
    bool m_newConvex;
};

#endif // STARSHAPECONFIGCOMMAND_H

