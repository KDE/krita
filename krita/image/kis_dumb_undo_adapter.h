/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_DUMB_UNDO_ADAPTER_H_
#define KIS_DUMB_UNDO_ADAPTER_H_

#include "kis_undo_adapter.h"

class KUndo2Command;

class KRITAIMAGE_EXPORT KisDumbUndoAdapter : public KisUndoAdapter
{
public:
    KisDumbUndoAdapter();
    virtual ~KisDumbUndoAdapter();

public:
    const KUndo2Command * presentCommand();

    void addCommand(KUndo2Command *command);
    void undoLastCommand();

    void beginMacro(const QString& macroName);
    void endMacro();

private:
    Q_DISABLE_COPY(KisDumbUndoAdapter);
};


#endif // KIS_DUMB_UNDO_ADAPTER_H_

