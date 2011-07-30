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

#include <KoColorSpaceRegistry.h>
#include "kis_image.h"

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

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 300, 300, cs, "test", true);

    KisStrokeStrategy *strategy =
        new KisStrokeStrategyUndoCommandBased("test", false,
                                              image->postExecutionUndoAdapter(),
                                              initCommand, finishCommand);

    KisStrokeId id = image->startStroke(strategy);
    image->addJob(id, new KisStrokeStrategyUndoCommandBased::Data(dabCommand));
    QTest::qSleep(500);
    image->cancelStroke(id);
    image->waitForDone();

    SCOMPARE(result.trimmed(), "init_redo dab_redo dab_undo init_undo");
}

#define NUM_JOBS 1000
#define SEQUENTIAL_NTH 12
#define NUM_CHECKS 10
#define CHECK_DELAY 2 // ms

class ExclusivenessCheckerCommand : public KUndo2Command
{
public:
    ExclusivenessCheckerCommand(QAtomicInt &counter,
                                QAtomicInt &hadConcurrency,
                                bool exclusive)
        : m_counter(counter),
          m_hadConcurrency(hadConcurrency),
          m_exclusive(exclusive)
    {
    }
    void redo() { checkState(); }
    void undo() { checkState(); }

private:
    void checkState() {
        m_counter.ref();
        for(int i = 0; i < NUM_CHECKS; i++) {

            if(m_exclusive) {
                Q_ASSERT(m_counter == 1);
            }
            else {
                m_hadConcurrency.ref();
            }

            QTest::qSleep(CHECK_DELAY);
        }
        m_counter.deref();
    }

private:
    QAtomicInt &m_counter;
    QAtomicInt &m_hadConcurrency;
    bool m_exclusive;
};

void KisStrokeStrategyUndoCommandBasedTest::stressTestSequentialCommands()
{

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 300, 300, cs, "test", true);

    QAtomicInt counter;
    QAtomicInt hadConcurrency;

    KisStrokeStrategy *strategy =
        new KisStrokeStrategyUndoCommandBased("test", false, 0);

    KisStrokeId id = image->startStroke(strategy);

    for(int i = 0; i < NUM_JOBS; i++) {
        bool isSequential = i % SEQUENTIAL_NTH == 0;

        KisStrokeJobData::Sequentiality seq = isSequential ?
            KisStrokeJobData::SEQUENTIAL : KisStrokeJobData::CONCURRENT;

        KUndo2CommandSP command(new ExclusivenessCheckerCommand(counter,
                                                                hadConcurrency,
                                                                isSequential));

        image->addJob(id,
            new KisStrokeStrategyUndoCommandBased::Data(command, seq));

    }

    image->endStroke(id);
    image->waitForDone();

    QVERIFY(!counter);
    qDebug() << "Concurrency observed:" << hadConcurrency
             << "/" << NUM_CHECKS * NUM_JOBS;
}

QTEST_KDEMAIN(KisStrokeStrategyUndoCommandBasedTest, NoGUI)
#include "kis_stroke_strategy_undo_command_based_test.moc"
