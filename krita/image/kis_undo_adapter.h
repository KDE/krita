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

#include <qglobal.h>

class QString;
class KCommand;

/**
 * Undo listeners want to be notified of undo and redo actions.
 * add notification is given _before_ the command is added to the
 * stack.
 * execute notification is given on undo and redo
 */
class KisCommandHistoryListener {

public:

    KisCommandHistoryListener(){};
   	virtual ~KisCommandHistoryListener() {} 
    virtual void notifyCommandAdded(KCommand * cmd) = 0;
    virtual void notifyCommandExecuted(KCommand * cmd) = 0;
};

class KisUndoAdapter {
public:
    KisUndoAdapter() {};
    virtual ~KisUndoAdapter() {};

public:
    
    virtual void setCommandHistoryListener(const KisCommandHistoryListener *) = 0;
    virtual void removeCommandHistoryListener(const KisCommandHistoryListener *) = 0;
    
    virtual KCommand * presentCommand() = 0;
    virtual void addCommand(KCommand *cmd) = 0;
    virtual void setUndo(bool undo) = 0;
    virtual bool undo() const = 0;
    virtual void beginMacro(const QString& macroName) = 0;
    virtual void endMacro() = 0;

private:
    KisUndoAdapter(const KisUndoAdapter&);
    KisUndoAdapter& operator=(const KisUndoAdapter&);
};


#endif // KIS_UNDO_ADAPTER_H_

