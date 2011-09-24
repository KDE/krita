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

#ifndef __KIS_SAVED_COMMANDS_H
#define __KIS_SAVED_COMMANDS_H

#include <kundo2command.h>
#include "kis_types.h"
#include "krita_export.h"
#include "kis_stroke_job_strategy.h"

class KisStrokesFacade;


class KisSavedCommandBase : public KUndo2Command
{
public:
    KisSavedCommandBase(const QString &name, KisStrokesFacade *strokesFacade);
    virtual ~KisSavedCommandBase();

    void undo();
    void redo();

protected:
    virtual void addCommands(KisStrokeId id, bool undo) = 0;
    KisStrokesFacade* strokesFacade();

private:
    void runStroke(bool undo);

private:
    KisStrokesFacade *m_strokesFacade;
    bool m_skipOneRedo;
};

class KisSavedCommand : public KisSavedCommandBase
{
public:
    KisSavedCommand(KUndo2CommandSP command, KisStrokesFacade *strokesFacade);

protected:
    void addCommands(KisStrokeId id, bool undo);

private:
    KUndo2CommandSP m_command;
};

class KisSavedMacroCommand : public KisSavedCommandBase
{
public:
    KisSavedMacroCommand(const QString &name, KisStrokesFacade *strokesFacade);
    ~KisSavedMacroCommand();

    void addCommand(KUndo2CommandSP command,
                    KisStrokeJobData::Sequentiality sequentiality,
                    KisStrokeJobData::Exclusivity exclusivity);

    void performCancel(KisStrokeId id, bool strokeUndo);

protected:
    void addCommands(KisStrokeId id, bool undo);

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_SAVED_COMMANDS_H */
