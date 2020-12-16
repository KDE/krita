/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
