/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_input_manager_test.h"

#include <QTest>

#include <QMouseEvent>

#include "input/kis_single_action_shortcut.h"
#include "input/kis_stroke_shortcut.h"
#include "input/kis_abstract_input_action.h"
#include "input/kis_shortcut_matcher.h"


void KisInputManagerTest::testSingleActionShortcut()
{
    KisSingleActionShortcut s(0,0);
    s.setKey(QSet<Qt::Key>() << Qt::Key_Shift, Qt::Key_Space);

    QVERIFY(s.match(QSet<Qt::Key>() << Qt::Key_Shift, Qt::Key_Space));
    QVERIFY(!s.match(QSet<Qt::Key>() << Qt::Key_Control, Qt::Key_Space));
    QVERIFY(!s.match(QSet<Qt::Key>(), Qt::Key_Space));
    QVERIFY(!s.match(QSet<Qt::Key>() << Qt::Key_Shift, Qt::Key_Escape));
    QVERIFY(!s.match(QSet<Qt::Key>() << Qt::Key_Shift, KisSingleActionShortcut::WheelUp));

    s.setWheel(QSet<Qt::Key>() << Qt::Key_Shift, KisSingleActionShortcut::WheelUp);

    QVERIFY(!s.match(QSet<Qt::Key>() << Qt::Key_Shift, Qt::Key_Space));
    QVERIFY(!s.match(QSet<Qt::Key>() << Qt::Key_Control, Qt::Key_Space));
    QVERIFY(!s.match(QSet<Qt::Key>(), Qt::Key_Space));
    QVERIFY(!s.match(QSet<Qt::Key>() << Qt::Key_Shift, Qt::Key_Escape));
    QVERIFY(s.match(QSet<Qt::Key>() << Qt::Key_Shift, KisSingleActionShortcut::WheelUp));
}

void KisInputManagerTest::testStrokeShortcut()
{
    KisStrokeShortcut s(0,0);
    s.setButtons(QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_Control,
                 QSet<Qt::MouseButton>() << Qt::LeftButton);

    QVERIFY(s.matchReady(QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_Control,
                         QSet<Qt::MouseButton>() << Qt::LeftButton));

    QVERIFY(s.matchReady(QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_Control,
                         QSet<Qt::MouseButton>()));

    QVERIFY(!s.matchReady(QSet<Qt::Key>() << Qt::Key_Control << Qt::Key_Alt,
                         QSet<Qt::MouseButton>()));

    QVERIFY(!s.matchReady(QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_Control,
                         QSet<Qt::MouseButton>() << Qt::RightButton));

    QVERIFY(s.matchBegin(Qt::LeftButton));
    QVERIFY(!s.matchBegin(Qt::RightButton));
}

struct TestingAction : public KisAbstractInputAction
{
    TestingAction() : KisAbstractInputAction("TestingAction"), m_isHighResolution(false) { reset(); }
    ~TestingAction() {}

    void begin(int shortcut, QEvent *event) { m_beginIndex = shortcut; m_beginNonNull = event;}
    void end(QEvent *event) { m_ended = true; m_endNonNull = event; }
    void inputEvent(QEvent* event) { Q_UNUSED(event); m_gotInput = true; }

    void reset() {
        m_beginIndex = -1;
        m_ended = false;
        m_gotInput = false;
        m_beginNonNull = false;
        m_endNonNull = false;
    }

    bool supportsHiResInputEvents() const {
        return m_isHighResolution;
    }

    void setHighResInputEvents(bool value) {
        m_isHighResolution = value;
    }

    int m_beginIndex;
    bool m_ended;
    bool m_gotInput;
    bool m_beginNonNull;
    bool m_endNonNull;

    bool m_isHighResolution;
};

KisSingleActionShortcut* createKeyShortcut(KisAbstractInputAction *action,
                                  int shortcutIndex,
                                  const QSet<Qt::Key> &modifiers,
                                  Qt::Key key)
{
    KisSingleActionShortcut *s = new KisSingleActionShortcut(action, shortcutIndex);
    s->setKey(modifiers, key);
    return s;
}

KisStrokeShortcut* createStrokeShortcut(KisAbstractInputAction *action,
                                     int shortcutIndex,
                                     const QSet<Qt::Key> &modifiers,
                                     Qt::MouseButton button)
{
    KisStrokeShortcut *s = new KisStrokeShortcut(action, shortcutIndex);
    s->setButtons(modifiers, QSet<Qt::MouseButton>() << button);
    return s;
}

void KisInputManagerTest::testKeyEvents()
{
    KisShortcutMatcher m;
    m.enterEvent();

    TestingAction *a = new TestingAction();


    m.addShortcut(
        createKeyShortcut(a, 10,
                          QSet<Qt::Key>() << Qt::Key_Shift,
                          Qt::Key_Enter));

    m.addShortcut(
        createKeyShortcut(a, 11,
                          QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_Control,
                          Qt::Key_Enter));

    m.addShortcut(
        createStrokeShortcut(a, 12,
                             QSet<Qt::Key>() << Qt::Key_Shift,
                             Qt::RightButton));

    m.addShortcut(
        createStrokeShortcut(a, 13,
                             QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_Control,
                             Qt::LeftButton));

    QCOMPARE(a->m_beginIndex, -1);

    // Test event with random values
    QMouseEvent mouseEvent(QEvent::MouseMove, QPoint(),
                           Qt::LeftButton, Qt::NoButton, Qt::NoModifier);

    // Press Ctrl+Shift
    QVERIFY(!m.keyPressed(Qt::Key_Shift));
    QCOMPARE(a->m_beginIndex, -1);

    QVERIFY(!m.keyPressed(Qt::Key_Control));
    QCOMPARE(a->m_beginIndex, -1);


    // Complete Ctrl+Shift+Enter shortcut
    QVERIFY(m.keyPressed(Qt::Key_Enter));
    QCOMPARE(a->m_beginIndex, 11);
    QCOMPARE(a->m_ended, true);
    QCOMPARE(a->m_beginNonNull, false);
    QCOMPARE(a->m_endNonNull, false);
    a->reset();


    // Pressing mouse buttons is disabled since Enter is pressed
    QVERIFY(!m.buttonPressed(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);
    QVERIFY(!m.buttonReleased(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);


    // Release Enter, so the system should be ready for new shortcuts
    QVERIFY(!m.keyReleased(Qt::Key_Enter));
    QCOMPARE(a->m_beginIndex, -1);


    // Complete Ctrl+Shift+LB shortcut
    QVERIFY(m.buttonPressed(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, 13);
    QCOMPARE(a->m_ended, false);
    QCOMPARE(a->m_beginNonNull, true);
    QCOMPARE(a->m_endNonNull, false);
    a->reset();

    QVERIFY(m.buttonReleased(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);
    QCOMPARE(a->m_ended, true);
    QCOMPARE(a->m_beginNonNull, false);
    QCOMPARE(a->m_endNonNull, true);
    a->reset();


    // There is no Ctrl+Shift+RB shortcut
    QVERIFY(!m.buttonPressed(Qt::RightButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);

    QVERIFY(!m.buttonReleased(Qt::RightButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);


    // Check that Ctrl+Shift+Enter is still enabled
    QVERIFY(m.keyPressed(Qt::Key_Enter));
    QCOMPARE(a->m_beginIndex, 11);
    QCOMPARE(a->m_ended, true);
    QCOMPARE(a->m_beginNonNull, false);
    QCOMPARE(a->m_endNonNull, false);
    a->reset();

    // Check autorepeat
    QVERIFY(m.autoRepeatedKeyPressed(Qt::Key_Enter));
    QCOMPARE(a->m_beginIndex, 11);
    QCOMPARE(a->m_ended, true);
    QCOMPARE(a->m_beginNonNull, false);
    QCOMPARE(a->m_endNonNull, false);
    a->reset();

    QVERIFY(!m.keyReleased(Qt::Key_Enter));
    QCOMPARE(a->m_beginIndex, -1);


    // Release Ctrl
    QVERIFY(!m.keyReleased(Qt::Key_Control));
    QCOMPARE(a->m_beginIndex, -1);


    // There is no Shift+LB shortcut
    QVERIFY(!m.buttonPressed(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);

    QVERIFY(!m.buttonReleased(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);


    // But there *is* Shift+RB shortcut
    QVERIFY(m.buttonPressed(Qt::RightButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, 12);
    QCOMPARE(a->m_ended, false);
    QCOMPARE(a->m_beginNonNull, true);
    QCOMPARE(a->m_endNonNull, false);
    a->reset();

    QVERIFY(m.buttonReleased(Qt::RightButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);
    QCOMPARE(a->m_ended, true);
    QCOMPARE(a->m_beginNonNull, false);
    QCOMPARE(a->m_endNonNull, true);
    a->reset();


    // Check that Shift+Enter still works
    QVERIFY(m.keyPressed(Qt::Key_Enter));
    QCOMPARE(a->m_beginIndex, 10);
    QCOMPARE(a->m_ended, true);
    QCOMPARE(a->m_beginNonNull, false);
    QCOMPARE(a->m_endNonNull, false);
    a->reset();

    m.leaveEvent();
}

void KisInputManagerTest::testReleaseUnnecessaryModifiers()
{
    KisShortcutMatcher m;
    m.enterEvent();

    TestingAction *a = new TestingAction();

    m.addShortcut(
        createStrokeShortcut(a, 13,
                             QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_Control,
                             Qt::LeftButton));

    // Test event with random values
    QMouseEvent mouseEvent(QEvent::MouseMove, QPoint(),
                           Qt::LeftButton, Qt::NoButton, Qt::NoModifier);

    // Press Ctrl+Shift
    QVERIFY(!m.keyPressed(Qt::Key_Shift));
    QCOMPARE(a->m_beginIndex, -1);

    QVERIFY(!m.keyPressed(Qt::Key_Control));
    QCOMPARE(a->m_beginIndex, -1);

    // Complete Ctrl+Shift+LB shortcut
    QVERIFY(m.buttonPressed(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, 13);
    QCOMPARE(a->m_ended, false);
    a->reset();

    // Release Ctrl
    QVERIFY(!m.keyReleased(Qt::Key_Control));
    QCOMPARE(a->m_beginIndex, -1);
    QCOMPARE(a->m_ended, false);

    // Release Shift
    QVERIFY(!m.keyReleased(Qt::Key_Shift));
    QCOMPARE(a->m_beginIndex, -1);
    QCOMPARE(a->m_ended, false);

    // Release LB, now it should end
    QVERIFY(m.buttonReleased(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);
    QCOMPARE(a->m_ended, true);
    a->reset();

    m.leaveEvent();
}

void KisInputManagerTest::testMouseMoves()
{
    KisShortcutMatcher m;
    m.enterEvent();

    TestingAction *a = new TestingAction();

    m.addShortcut(
        createStrokeShortcut(a, 13,
                             QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_Control,
                             Qt::LeftButton));

    // Test event with random values
    QMouseEvent mouseEvent(QEvent::MouseMove, QPoint(),
                           Qt::LeftButton, Qt::NoButton, Qt::NoModifier);


    // Press Ctrl+Shift
    QVERIFY(!m.keyPressed(Qt::Key_Shift));
    QCOMPARE(a->m_beginIndex, -1);

    QVERIFY(!m.keyPressed(Qt::Key_Control));
    QCOMPARE(a->m_beginIndex, -1);

    QVERIFY(!m.pointerMoved(&mouseEvent));
    QCOMPARE(a->m_gotInput, false);

    // Complete Ctrl+Shift+LB shortcut
    QVERIFY(m.buttonPressed(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, 13);
    QCOMPARE(a->m_ended, false);
    QCOMPARE(a->m_gotInput, false);
    a->reset();

    QVERIFY(m.pointerMoved(&mouseEvent));
    QCOMPARE(a->m_gotInput, true);
    a->reset();

    // Release Ctrl
    QVERIFY(!m.keyReleased(Qt::Key_Control));
    QCOMPARE(a->m_beginIndex, -1);
    QCOMPARE(a->m_ended, false);
    QCOMPARE(a->m_gotInput, false);

    // Release Shift
    QVERIFY(!m.keyReleased(Qt::Key_Shift));
    QCOMPARE(a->m_beginIndex, -1);
    QCOMPARE(a->m_ended, false);

    // Release LB, now it should end
    QVERIFY(m.buttonReleased(Qt::LeftButton, &mouseEvent));
    QCOMPARE(a->m_beginIndex, -1);
    QCOMPARE(a->m_ended, true);
    a->reset();

    m.leaveEvent();
}

#include "../input/wintab/kis_incremental_average.h"

void KisInputManagerTest::testIncrementalAverage()
{
    KisIncrementalAverage avg(3);

    QCOMPARE(avg.pushThrough(10), 10);
    QCOMPARE(avg.pushThrough(20), 13);
    QCOMPARE(avg.pushThrough(30), 20);
    QCOMPARE(avg.pushThrough(30), 26);
    QCOMPARE(avg.pushThrough(30), 30);

}

QTEST_MAIN(KisInputManagerTest)
