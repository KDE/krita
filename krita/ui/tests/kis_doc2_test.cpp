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

#include "kis_doc2_test.h"

#include <kundo2stack.h>
#include <qtest_kde.h>
#include <kstandarddirs.h>
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_undo_store.h"
#include "kis_factory2.h"
#include <KoDocumentEntry.h>
#include <KoMainWindow.h>

class KisCommandHistoryListenerFake : public KisCommandHistoryListener
{
public:
    KisCommandHistoryListenerFake() {
        reset();
    }

    void reset() {
        m_command = 0;
        m_executed = false;
        m_added = false;
    }

    void notifyCommandAdded(const KUndo2Command * cmd) {
        if(!m_command) {
            m_command = cmd;
        }
        Q_ASSERT(cmd == m_command);

        m_added = true;
    }

    void notifyCommandExecuted(const KUndo2Command * cmd) {
        if(!m_command) {
            m_command = cmd;
        }
        Q_ASSERT(cmd == m_command);

        m_executed = true;
    }

    bool wasAdded() {
        return m_added;
    }

    bool wasExecuted() {
        return m_executed;
    }

    const KUndo2Command* command() {
        return m_command;
    }

private:
    const KUndo2Command* m_command;
    bool m_executed;
    bool m_added;
};


class TestCommand : public KUndo2Command
{
public:
    void undo() {}
    void redo() {}
};

void KisDoc2Test::testUndoRedoNotify()
{
    KisDoc2 doc;
    doc.initEmpty();

    KUndo2Command *testCommand1 = new TestCommand();
    KUndo2Command *testCommand2 = new TestCommand();

    KisUndoStore *undoStore = doc.image()->undoStore();
    KisCommandHistoryListenerFake listener;

    undoStore->setCommandHistoryListener(&listener);

    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Adding one command";
    listener.reset();
    undoStore->addCommand(testCommand1);
    QVERIFY(listener.wasAdded());
    QVERIFY(!listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand1);
    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Adding one more command";
    listener.reset();
    undoStore->addCommand(testCommand2);
    QVERIFY(listener.wasAdded());
    QVERIFY(!listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand2);
    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Undo";
    listener.reset();
    doc.undoStack()->undo();
    QVERIFY(!listener.wasAdded());
    QVERIFY(listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand2);
    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Undo";
    listener.reset();
    doc.undoStack()->undo();
    QVERIFY(!listener.wasAdded());
    QVERIFY(listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand1);
    qDebug() << "Undo index:" << doc.undoStack()->index();


    /**
     * FIXME: Here is a bug in undo listeners framework
     * notifyCommandExecuted works wrong with redo(),
     * because KUndo2Stack->index() returns "the command
     * that will be executed on the next redo()", but
     * not the undone command
     */

    qDebug() << "Redo";
    listener.reset();
    doc.undoStack()->redo();
    QVERIFY(!listener.wasAdded());
    QVERIFY(listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand2);
    qDebug() << "Undo index:" << doc.undoStack()->index();

}

void KisDoc2Test::testOpenImageTwiceInSameDoc()
{
    QString fname2 = QString(FILES_DATA_DIR) + QDir::separator() + "load_test.kra";
    QString fname = QString(FILES_DATA_DIR) + QDir::separator() + "load_test2.kra";


    Q_ASSERT(!fname.isEmpty());
    Q_ASSERT(!fname2.isEmpty());
    KisDoc2 doc;
    doc.loadNativeFormat(fname);
    doc.loadNativeFormat(fname2);
}

QTEST_KDEMAIN(KisDoc2Test, GUI)
#include "kis_doc2_test.moc"

