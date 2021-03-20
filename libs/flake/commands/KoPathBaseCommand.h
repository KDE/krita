/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHBASECOMMAND_H
#define KOPATHBASECOMMAND_H

#include <kundo2command.h>
#include <QSet>

class KoPathShape;

/// the base command for commands altering a path shape
class KoPathBaseCommand : public KUndo2Command
{
public:
    /**
     * @param parent the parent command used for macro commands
     */
    explicit KoPathBaseCommand(KUndo2Command *parent = 0);

    /** initialize the base command with a single shape
     * @param shape the shape
     * @param parent the parent command used for macro commands
     */
    explicit KoPathBaseCommand(KoPathShape *shape, KUndo2Command *parent = 0);

protected:
    /**
     * Schedules repainting of all shapes control point rects.
     * @param normalizeShapes controls if paths are normalized before painting
     */
    void repaint(bool normalizeShapes);

    QSet<KoPathShape*> m_shapes; ///< the shapes the command operates on
};

#endif // KOPATHBASECOMMAND_H
