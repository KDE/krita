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
#include "kritacommand_export.h"

namespace KisCommandUtils
{
    struct KRITACOMMAND_EXPORT AggregateCommand : public KUndo2Command {
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

    struct KRITACOMMAND_EXPORT SkipFirstRedoWrapper : public KUndo2Command {
        SkipFirstRedoWrapper(KUndo2Command *child = 0, KUndo2Command *parent = 0);
        void redo() override;
        void undo() override;

    private:
        bool m_firstRedo;
        QScopedPointer<KUndo2Command> m_child;
    };

    struct KRITACOMMAND_EXPORT SkipFirstRedoBase : public KUndo2Command {
        SkipFirstRedoBase(bool skipFirstRedo, KUndo2Command *parent = 0);
        SkipFirstRedoBase(bool skipFirstRedo, const KUndo2MagicString &text, KUndo2Command *parent = 0);

        void redo() final;
        void undo() final;

        void setSkipOneRedo(bool value);

    protected:
        virtual void redoImpl() = 0;
        virtual void undoImpl() = 0;

    private:
        bool m_firstRedo;
    };


    struct KRITACOMMAND_EXPORT FlipFlopCommand : public KUndo2Command {
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
    };

    struct KRITACOMMAND_EXPORT CompositeCommand : public KUndo2Command {
        CompositeCommand(KUndo2Command *parent = 0);
        ~CompositeCommand();

        void addCommand(KUndo2Command *cmd);

        void redo();
        void undo();

    private:
        QVector<KUndo2Command*> m_commands;
    };
}

#endif /* __KIS_COMMAND_UTILS_H */
