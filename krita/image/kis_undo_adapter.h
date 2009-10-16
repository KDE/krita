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

class QUndoCommand;
class KoDocument;

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
    virtual void notifyCommandAdded(const QUndoCommand * cmd) = 0;
    virtual void notifyCommandExecuted(const QUndoCommand * cmd) = 0;
};

class KRITAIMAGE_EXPORT KisUndoAdapter : public QObject
{
    Q_OBJECT

public:
    KisUndoAdapter(KoDocument* doc);
    virtual ~KisUndoAdapter();

public:

    virtual void setCommandHistoryListener(KisCommandHistoryListener * l);
    virtual void removeCommandHistoryListener(KisCommandHistoryListener * l);
    virtual void notifyCommandExecuted(const QUndoCommand *command);

    virtual const QUndoCommand * presentCommand();
    virtual void addCommand(QUndoCommand *cmd);
    virtual void undoLastCommand();
    virtual void setUndo(bool undo);
    virtual bool undo() const;

    /// XXX: is this actually threadsafe?
    virtual void beginMacro(const QString& macroName);

    /// XXX: is this actually threadsafe?
    virtual void endMacro();

    void emitSelectionChanged();

signals:
    void selectionChanged();

private:
    KisUndoAdapter(const KisUndoAdapter&);
    KisUndoAdapter& operator=(const KisUndoAdapter&);

    QVector<KisCommandHistoryListener*> m_undoListeners;
    KoDocument* m_doc;
    bool m_undo;
};


#endif // KIS_UNDO_ADAPTER_H_

