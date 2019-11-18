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
#include "kis_stroke_job_strategy.h"

class KisStrokesFacade;


class KRITAIMAGE_EXPORT KisSavedCommandBase : public KUndo2Command
{
public:
    KisSavedCommandBase(const KUndo2MagicString &name, KisStrokesFacade *strokesFacade);
    ~KisSavedCommandBase() override;


    void undo() override;
    void redo() override;

protected:
    virtual void addCommands(KisStrokeId id, bool undo) = 0;
    KisStrokesFacade* strokesFacade();

private:
    void runStroke(bool undo);

private:
    KisStrokesFacade *m_strokesFacade;
    bool m_skipOneRedo;
};

class KRITAIMAGE_EXPORT KisSavedCommand : public KisSavedCommandBase
{
public:
    KisSavedCommand(KUndo2CommandSP command, KisStrokesFacade *strokesFacade);
    int timedId() override;
    void setTimedID(int timedID) override;

    int id() const override;
    bool mergeWith(const KUndo2Command* command) override;

    bool timedMergeWith(KUndo2Command *other) override;
    QVector<KUndo2Command*> mergeCommandsVector() override;
    void setTime() override;
    QTime time() override;
    void setEndTime() override;
    QTime endTime() override;
    bool isMerged() override;

protected:
    void addCommands(KisStrokeId id, bool undo) override;

private:
    KUndo2CommandSP m_command;
};

class KRITAIMAGE_EXPORT KisSavedMacroCommand : public KisSavedCommandBase
{
public:
    KisSavedMacroCommand(const KUndo2MagicString &name, KisStrokesFacade *strokesFacade);
    ~KisSavedMacroCommand() override;

    int id() const override;
    bool mergeWith(const KUndo2Command* command) override;

    void setMacroId(int value);

    void addCommand(KUndo2CommandSP command,
                    KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                    KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    void performCancel(KisStrokeId id, bool strokeUndo);

    void getCommandExecutionJobs(QVector<KisStrokeJobData*> *jobs, bool undo, bool shouldGoToHistory = true) const;

    void setOverrideInfo(const KisSavedMacroCommand *overriddenCommand, const QVector<const KUndo2Command *> &skipWhileOverride);

protected:
    void addCommands(KisStrokeId id, bool undo) override;

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_SAVED_COMMANDS_H */
