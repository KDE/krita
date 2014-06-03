/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LEGACY_UNDO_ADAPTER_H
#define __KIS_LEGACY_UNDO_ADAPTER_H

#include "kis_undo_adapter.h"


/**
 * KisLegacyUndoAdapter -- blocks the strokes and updates queue,
 * and then adds the command to a store
 */
class KRITAIMAGE_EXPORT KisLegacyUndoAdapter : public KisUndoAdapter
{
public:
    KisLegacyUndoAdapter(KisUndoStore *undoStore, KisImageWSP image);

    const KUndo2Command* presentCommand();
    void undoLastCommand();
    void addCommand(KUndo2Command *cmd);
    void beginMacro(const QString& macroName);
    void endMacro();
private:
    KisImageWSP m_image;
    qint32 m_macroCounter;
};

#endif /* __KIS_LEGACY_UNDO_ADAPTER_H */
