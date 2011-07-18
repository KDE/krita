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

class TestingUndoCommand : public KUndo2Command
{
public:
    TestingUndoCommand(const QString &name, QString &result)
        : KUndo2Command(name),
          m_result(result)
    {
    }

    void undo() {
        m_result += QString(" ") + text() + undoString(true);
    }

    void redo() {
        m_result += QString(" ") + text() + undoString(false);
    }

private:
    QString &m_result;
};


void KisStrokeStrategyUndoCommandBasedTest::testFinishedStroke()
{
    QString result;
    KUndo2CommandSP initCommand(new TestingUndoCommand("init", result));
    KUndo2CommandSP dabCommand(new TestingUndoCommand("dab", result));
    KUndo2CommandSP finishCommand(new TestingUndoCommand("finish", result));

    KisStrokeStrategy *strategy =
        new KisStrokeStrategyUndoCommandBased("test", false, 0,
                                              initCommand, finishCommand);

    KisStroke stroke(strategy);
    stroke.addJob(
        new KisStrokeStrategyUndoCommandBased::Data(dabCommand));
    stroke.endStroke();

    executeStrokeJobs(&stroke);

    SCOMPARE(result.trimmed(), "init_redo dab_redo finish_redo");
}

void KisStrokeStrategyUndoCommandBasedTest::testCancelledStroke()
{
    QString result;
    KUndo2CommandSP initCommand(new TestingUndoCommand("init", result));
    KUndo2CommandSP dabCommand(new TestingUndoCommand("dab", result));
    KUndo2CommandSP finishCommand(new TestingUndoCommand("finish", result));

    KisStrokeStrategy *strategy =
        new KisStrokeStrategyUndoCommandBased("test", false, 0,
                                              initCommand, finishCommand);

    KisStroke stroke(strategy);
    stroke.addJob(
        new KisStrokeStrategyUndoCommandBased::Data(dabCommand));


    KisStrokeJob *job;
    job = stroke.popOneJob(); // init
    job->run();
    delete job;

    job = stroke.popOneJob(); // dab
    job->run();
    delete job;

    stroke.cancelStroke();

    executeStrokeJobs(&stroke);

    SCOMPARE(result.trimmed(), "init_redo dab_redo dab_undo init_undo");
}

QTEST_KDEMAIN(KisStrokeStrategyUndoCommandBasedTest, NoGUI)
#include "kis_stroke_strategy_undo_command_based_test.moc"
