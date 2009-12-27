/* This file is part of the KDE project
 * Copyright (C) 2007 Peter Simonsson <peter.simonsson@gmail.com>
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

#ifndef KOSHAPEKEEPASPECTRATIOCOMMAND_H
#define KOSHAPEKEEPASPECTRATIOCOMMAND_H

#include <QUndoCommand>
#include <QList>

class KoShape;

/**
 * Command that changes the keepAspectRatio property of KoShape
 */
class KoShapeKeepAspectRatioCommand : public QUndoCommand
{
public:
    /**
     * Constructor
     * @param shapes the shapes affected by the command
     * @param oldKeepAspectRatio the old settings
     * @param newKeepAspectRatio the new settings
     * @param parent the parent command
     */
    KoShapeKeepAspectRatioCommand(const QList<KoShape*> &shapes, const QList<bool> &oldKeepAspectRatio, const QList<bool> &newKeepAspectRatio, QUndoCommand* parent = 0);
    ~KoShapeKeepAspectRatioCommand();

    /// Execute the command
    virtual void redo();
    /// Unexecute the command
    virtual void undo();

private:
    QList<KoShape*> m_shapes;
    QList<bool> m_oldKeepAspectRatio;
    QList<bool> m_newKeepAspectRatio;
};

#endif
