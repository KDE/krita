/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef RECTANGLESHAPECONFIGCOMMAND_H
#define RECTANGLESHAPECONFIGCOMMAND_H

#include <kundo2command.h>

class RectangleShape;

/// The undo / redo command for configuring a rectangle shape
class RectangleShapeConfigCommand : public KUndo2Command
{
public:
    /**
     * Configures a rectangle shape
     * @param Rectangle the rectangle shape to configure
     * @param cornerRadiusX the x corner radius
     * @param cornerRadiusY the y corner radius
     * @param parent the optional parent command
     */
    RectangleShapeConfigCommand(RectangleShape *rectangle, qreal cornerRadiusX, qreal cornerRadiusY, KUndo2Command *parent = 0);
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;

private:
    RectangleShape *m_rectangle;
    qreal m_oldCornerRadiusX;
    qreal m_oldCornerRadiusY;
    qreal m_newCornerRadiusX;
    qreal m_newCornerRadiusY;
};

#endif // RECTANGLESHAPECONFIGCOMMAND_H

