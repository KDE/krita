/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_COMMAND_UTILS_H
#define __KIS_COMMAND_UTILS_H

#include "kundo2command.h"
#include "kis_undo_stores.h"
#include "kritaimage_export.h"

namespace KisCommandUtils
{
    struct KRITAIMAGE_EXPORT AggregateCommand : public KUndo2Command {
        AggregateCommand();

        void redo();
        void undo();

    protected:
        virtual void populateChildCommands() = 0;
        void addCommand(KUndo2Command *cmd);

    private:
        bool m_firstRedo;
        KisSurrogateUndoStore m_store;
    };

    struct KRITAIMAGE_EXPORT SkipFirstRedoWrapper : public KUndo2Command {
        SkipFirstRedoWrapper(KUndo2Command *child = 0, KUndo2Command *parent = 0);
        void redo();
        void undo();

    private:
        bool m_firstRedo;
        KUndo2Command *m_child;
    };

    struct KRITAIMAGE_EXPORT FlipFlopCommand : public KUndo2Command {
        FlipFlopCommand(bool finalize, KUndo2Command *parent = 0);

        void redo();
        void undo();

    protected:
        virtual void init();
        virtual void end();
        bool isFinalizing() const { return m_finalize; }
        bool isFirstRedo() const { return m_firstRedo; }

    private:
        bool m_finalize;
        bool m_firstRedo;
        KisSurrogateUndoStore m_store;
    };
}

#endif /* __KIS_COMMAND_UTILS_H */
