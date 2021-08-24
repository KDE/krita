/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    bool canAnnihilateWith(const KUndo2Command *command) const override;

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
    bool canAnnihilateWith(const KUndo2Command *command) const override;

    void setMacroId(int value);

    void addCommand(KUndo2CommandSP command,
                    KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                    KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    void getCommandExecutionJobs(QVector<KisStrokeJobData*> *jobs, bool undo, bool shouldGoToHistory = true) const;

    void setOverrideInfo(const KisSavedMacroCommand *overriddenCommand, const QVector<const KUndo2Command *> &skipWhileOverride);
protected:
    void addCommands(KisStrokeId id, bool undo) override;

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_SAVED_COMMANDS_H */
