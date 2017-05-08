/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_GUI_CONTEXT_COMMAND_H
#define __KIS_GUI_CONTEXT_COMMAND_H

#include <QObject>
#include "kundo2command.h"

class KisGuiContextCommandDelegate;

/**
 * KisGuiContextCommand is a special command-wrapper which ensures
 * that the holding command is executed in the GUI thread only. Please
 * note that any activity done by the containing command must *not*
 * lead to the blocking on the image, otherwise you'll get a deadlock!
 */
class KisGuiContextCommand : public QObject, public KUndo2Command
{
    Q_OBJECT
public:
    KisGuiContextCommand(KUndo2Command *command, QObject *guiObject);
    ~KisGuiContextCommand() override;

    void undo() override;
    void redo() override;

Q_SIGNALS:
    void sigExecuteCommand(KUndo2Command *command, bool undo);

private:
    QScopedPointer<KUndo2Command> m_command;
    QScopedPointer<KisGuiContextCommandDelegate> m_delegate;
};

#endif /* __KIS_GUI_CONTEXT_COMMAND_H */
