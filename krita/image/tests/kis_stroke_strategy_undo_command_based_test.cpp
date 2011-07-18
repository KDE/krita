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

#include "kis_stroke_strategy_undo_command_based_test.h"
#include <qtest_kde.h>

#include "kis_stroke.h"
#include "kis_stroke_strategy_undo_command_based.h"
#include "scheduler_utils.h"

inline QString undoString(bool undo) {
    return undo ? "_undo" : "_redo";
}

inline QString getCommandName(KisStrokeJob *job) {
    KisStrokeJobStrategyUndoCommandBased *jobStrategy =
        dynamic_cast<KisStrokeJobStrategyUndoCommandBased*>(
            job->testingGetDabStrategy());

    if(jobStrategy) {
        KisStrokeJobStrategyUndoCommandBased::Data *data =
            dynamic_cast<KisStrokeJobStrategyUndoCommandBased::Data*>(
                job->testingGetDabData());

        return data->command->text() + undoString(jobStrategy->m_parentStroke->undo());
    }
    else {
        KisStrokeJobStrategyCancelUndoCommandBased *jobStrategy =
            dynamic_cast<KisStrokeJobStrategyCancelUndoCommandBased*>(
                job->testingGetDabStrategy());

        QString result;
        QVector<KUndo2CommandSP> commands =
            jobStrategy->m_parentStroke->takeFinishedCommands();

        foreach(KUndo2CommandSP cmd, commands) {
            result += cmd->text() + undoString(jobStrategy->m_parentStroke) + QString(" ");
        }
        return result.trimmed();
    }
}


void KisStrokeStrategyUndoCommandBasedTest::testFinishedStroke()
{
    KUndo2CommandSP initCommand(new KUndo2Command("init"));
    KUndo2CommandSP dabCommand(new KUndo2Command("dab"));
    KUndo2CommandSP finishCommand(new KUndo2Command("finish"));

    KisStrokeStrategy *strategy =
        new KisStrokeStrategyUndoCommandBased("test", false, 0,
                                              initCommand, finishCommand);

    KisStroke stroke(strategy);
    stroke.addJob(
        new KisStrokeJobStrategyUndoCommandBased::Data(dabCommand, false));
    stroke.endStroke();


    QQueue<KisStrokeJob*> &queue = stroke.testingGetQueue();
    QCOMPARE(queue.size(), 3);

    SCOMPARE(getCommandName(queue[0]), "init_redo");
    SCOMPARE(getCommandName(queue[1]), "dab_redo");
    SCOMPARE(getCommandName(queue[2]), "finish_redo");

    delete stroke.popOneJob(); // init
    delete stroke.popOneJob(); // dab
    delete stroke.popOneJob(); // finish
}

void KisStrokeStrategyUndoCommandBasedTest::testCancelledStroke()
{
    KUndo2CommandSP initCommand(new KUndo2Command("init"));
    KUndo2CommandSP dabCommand(new KUndo2Command("dab"));
    KUndo2CommandSP finishCommand(new KUndo2Command("finish"));

    KisStrokeStrategy *strategy =
        new KisStrokeStrategyUndoCommandBased("test", false, 0,
                                              initCommand, finishCommand);

    KisStroke stroke(strategy);
    stroke.addJob(
        new KisStrokeJobStrategyUndoCommandBased::Data(dabCommand, false));


    KisStrokeJob *job;
    job = stroke.popOneJob(); // init
    job->run();
    delete job;

    job = stroke.popOneJob(); // dab
    job->run();
    delete job;

    stroke.cancelStroke();

    QQueue<KisStrokeJob*> &queue = stroke.testingGetQueue();
    QCOMPARE(queue.size(), 1);

    SCOMPARE(getCommandName(queue[0]), "init_undo dab_undo");

    delete stroke.popOneJob(); // cancel
}

QTEST_KDEMAIN(KisStrokeStrategyUndoCommandBasedTest, NoGUI)
#include "kis_stroke_strategy_undo_command_based_test.moc"
