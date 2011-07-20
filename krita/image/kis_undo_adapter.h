/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_UNDO_ADAPTER_H_
#define KIS_UNDO_ADAPTER_H_

#include <QString>
#include <QVector>
#include <QObject>

#include <krita_export.h>
#include "kis_types.h"

class KUndo2Command;


/**
 * Undo listeners want to be notified of undo and redo actions.
 * add notification is given _before_ the command is added to the
 * stack.
 * execute notification is given on undo and redo
 */
class KisCommandHistoryListener
{

public:

    KisCommandHistoryListener() {}
    virtual ~KisCommandHistoryListener() {}
    virtual void notifyCommandAdded(const KUndo2Command * cmd) = 0;
    virtual void notifyCommandExecuted(const KUndo2Command * cmd) = 0;
};

class KRITAIMAGE_EXPORT KisUndoAdapter : public QObject
{
    Q_OBJECT

public:
    KisUndoAdapter();
    virtual ~KisUndoAdapter();

public:
    void setCommandHistoryListener(KisCommandHistoryListener *listener);
    void removeCommandHistoryListener(KisCommandHistoryListener *listener);

    void notifyCommandAdded(const KUndo2Command *command);
    void notifyCommandExecuted(const KUndo2Command *command);

    void setImage(KisImageWSP image);
    KisImageWSP image();

    void emitSelectionChanged();
    virtual void addCommand(KUndo2Command *cmd);

    virtual const KUndo2Command* presentCommand() = 0;
    virtual void undoLastCommand() = 0;
    virtual void addCommand(KUndo2CommandSP cmd) = 0;

    /// XXX: is this actually threadsafe?
    /// AAA: No. Absolutely.
    virtual void beginMacro(const QString& macroName) = 0;
    virtual void endMacro() = 0;

signals:
    void selectionChanged();

private:
    Q_DISABLE_COPY(KisUndoAdapter);
    QVector<KisCommandHistoryListener*> m_undoListeners;
    KisImageWSP m_image;
};


#endif // KIS_UNDO_ADAPTER_H_

