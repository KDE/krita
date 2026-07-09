/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_manager_test.h"

#include <simpletest.h>

// for QMutableEventPoint
#include <QtGui/private/qeventpoint_p.h>
#include <QMouseEvent>

#include "input/kis_single_action_shortcut.h"
#include "input/kis_stroke_shortcut.h"
#include "input/kis_touch_shortcut.h"
#include "input/kis_abstract_input_action.h"
#include "input/kis_shortcut_matcher.h"

#include <kis_debug.h>
#include <kis_config.h>

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
    enum State {
        Deactivated = 0,
        Activated,
        Running
    };

    TestingAction()
        : KisAbstractInputAction("TestingAction")
        , m_isHighResolution(false)
    {
        setName("testing touch action");
        reset();
    }
    ~TestingAction() {}

    void activate(int shortcut) override
    {
        KIS_ASSERT(m_state == Deactivated);
        m_state = Activated;

        m_activatedShortcut = shortcut;
    }

    void deactivate(int shortcut) override
    {
        KIS_ASSERT(m_state == Activated);
        KIS_ASSERT(m_activatedShortcut == shortcut);
        m_state = Deactivated;

        m_activatedShortcut = -1;
    }

    void begin(int shortcut, QEvent *event) override {
        KIS_ASSERT(m_state == Activated || m_state == Deactivated);
        // TODO: make sure that single action shortcuts first deactivate currently
        // active long actions
        // KIS_ASSERT(m_state == Deactivated || m_activatedShortcut == shortcut);
        m_stateBeforeRunning = m_state;
        m_state = Running;

        m_beginIndex = shortcut;
        m_begunIndexes.append(shortcut);
        m_beginNonNull = event;
    }
    void end(QEvent *event) override {
        KIS_ASSERT(m_state == Running);
        m_state = m_stateBeforeRunning;
        m_endedIndexes.append(m_beginIndex);

        m_ended = true; m_endNonNull = event;
    }
    void inputEvent(QEvent* event) override {
        KIS_ASSERT(m_state == Running);

        Q_UNUSED(event); m_gotInput = true;
    }

    void reset() {
        m_beginIndex = -1;
        m_begunIndexes.clear();
        m_ended = false;
        m_gotInput = false;
        m_beginNonNull = false;
        m_endNonNull = false;
    }

    bool supportsHiResInputEvents(int /*shortcut*/) const override {
        return m_isHighResolution;
    }

    void setHighResInputEvents(bool value) {
        m_isHighResolution = value;
    }

    int m_activatedShortcut = -1;
    int m_beginIndex;
    bool m_ended;
    bool m_gotInput;
    bool m_beginNonNull;
    bool m_endNonNull;

    bool m_isHighResolution;
    State m_state {Deactivated};
    State m_stateBeforeRunning {Deactivated};
    QList<int> m_begunIndexes;
    QList<int> m_endedIndexes;

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

KisTouchShortcut* createTouchShortcut(KisAbstractInputAction *action,
                                     int shortcutIndex,
                                     KisShortcutConfiguration::GestureAction gesture)
{
    KisTouchShortcut *s = new KisTouchShortcut(action, shortcutIndex, gesture);

    // TODO: move the initialization to the shortcut itself
    switch(gesture) {
#ifndef Q_OS_MACOS
    case KisShortcutConfiguration::OneFingerTap:
    case KisShortcutConfiguration::OneFingerDrag:
        // Touch painting takes precedence over one-finger touch shortcuts, so
        // disable this type of shortcut when touch painting is active. Except
        // touch hold shortcuts, since touching and holding in one spot does
        // nothing otherwise and is therefore unambiguous.
        s->setDisableOnTouchPainting(true);
        Q_FALLTHROUGH();
    case KisShortcutConfiguration::OneFingerHold:
        s->setMinimumTouchPoints(1);
        s->setMaximumTouchPoints(1);
        break;
    case KisShortcutConfiguration::TwoFingerTap:
    case KisShortcutConfiguration::TwoFingerDrag:
        s->setMinimumTouchPoints(2);
        s->setMaximumTouchPoints(2);
        break;
    case KisShortcutConfiguration::ThreeFingerTap:
    case KisShortcutConfiguration::ThreeFingerDrag:
        s->setMinimumTouchPoints(3);
        s->setMaximumTouchPoints(3);
        break;
    case KisShortcutConfiguration::FourFingerTap:
    case KisShortcutConfiguration::FourFingerDrag:
        s->setMinimumTouchPoints(4);
        s->setMaximumTouchPoints(4);
        break;
    case KisShortcutConfiguration::FiveFingerTap:
    case KisShortcutConfiguration::FiveFingerDrag:
        s->setMinimumTouchPoints(5);
        s->setMaximumTouchPoints(5);
#endif
    default:
        break;
    }
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

/**********************************************************************/
/*      A framework for testing touch processing state automata       */
/**********************************************************************/

struct TouchSequenceGeneratorBase
{
    TouchSequenceGeneratorBase(int fingerCount, int maxEventIndex, const QPointF pointOffset)
        : m_fingerCount(fingerCount)
        , m_maxEventIndex(maxEventIndex)
        , m_pointOffset(pointOffset)
    {
    }

    virtual ~TouchSequenceGeneratorBase() {}

    void updatePoints(int eventIndex, QList<QEventPoint> &touchPoints)
    {
        /**
         * First, remove all the points marked as "Release" at the previous step
         */
        for (auto it = touchPoints.begin(); it != touchPoints.end();) {
            if (it->state() == QEventPoint::Released) {
                it = touchPoints.erase(it);
            } else {
                ++it;
            }
        }

        updatePointsSet(eventIndex, touchPoints);

        for (auto it = touchPoints.begin(); it != touchPoints.end(); ++it) {
            /**
             * Now update all points positions
             */
            QMutableEventPoint::setGlobalLastPosition(*it, it->globalPosition());
            QMutableEventPoint::setGlobalPosition(*it, globalPositionForPoint(it->id(), eventIndex));
            QMutableEventPoint::setPosition(*it, it->globalPosition());

            if (it->state() == QEventPoint::Pressed && it->globalLastPosition() != it->globalPosition()) {
                /**
                 * If a "Pressed" point has been offset, then we should change
                 * its state to "Updated"
                 */
                QMutableEventPoint::setState(*it, QEventPoint::Updated);
            } else if (eventIndex == m_maxEventIndex) {
                /**
                 * At the very last iteration, mark all the points as
                 * "Released".
                 */
                QMutableEventPoint::setState(*it, QEventPoint::Released);
            }

            // ENTER_FUNCTION() << ppVar(eventIndex) << ppVar(it->id()) << ppVar(it->state()) <<
            // ppVar(it->globalPosition()) << ppVar(it->globalLastPosition());
        }
    }

protected:
    QPointF globalPositionForPoint(int id, int eventIndex)
    {
        return QPointF(10 * id + eventIndex * m_pointOffset.x(), eventIndex * m_pointOffset.y());
    };

    void appendNewPoint(QList<QEventPoint> &touchPoints, int eventIndex)
    {
        const int newIndex = touchPoints.size();
        const QPointF pos = globalPositionForPoint(10 * newIndex, eventIndex);
        touchPoints.emplace_back(10 * newIndex, QEventPoint::Pressed, pos, pos);
        QMutableEventPoint::setGlobalPressPosition(touchPoints.back(), pos);
    };

    /**
     * Modify the points set in \p touchPoints accorting to \p eventIndex
     *
     * 1) To add a point call `appendNewPoint(touchPoints, eventIndex)`, the state
     *    will automatically initialized correctly.
     *
     * 2) To remove a point just change its state to "Released" and
     *    it will be removed at the next iteration automatically.
     */
    virtual void updatePointsSet(int eventIndex, QList<QEventPoint> &touchPoints) = 0;

protected:
    int m_fingerCount = 0;
    int m_maxEventIndex = 0;
    const QPointF m_pointOffset;
};

struct TouchCleanDrag : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<QEventPoint> &touchPoints) override
    {
        if (eventIndex == 0) {
            for (int i = 0; i < m_fingerCount; i++) {
                appendNewPoint(touchPoints, eventIndex);
            }
        }
    }
};

struct TouchDragDirtyStart : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<QEventPoint> &touchPoints) override
    {
        KIS_ASSERT(m_fingerCount > 1);
        KIS_ASSERT(m_maxEventIndex > 6);

        if (eventIndex == 0) {
            appendNewPoint(touchPoints, eventIndex);
        }

        if (eventIndex == 1) {
            // noop
        }

        if (eventIndex == 2) {
            appendNewPoint(touchPoints, eventIndex);
        }

        if (eventIndex == 3) {
            // noop
        }

        if (eventIndex == 4 && m_fingerCount > 2) {
            appendNewPoint(touchPoints, eventIndex);
        }
    }
};

struct TouchDragDirtyEnd : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<QEventPoint> &touchPoints) override
    {
        if (eventIndex == 0) {
            for (int i = 0; i < m_fingerCount; ++i) {
                appendNewPoint(touchPoints, eventIndex);
            }
        }

        for (int i = 0; i < m_fingerCount; i++) {
            if (eventIndex == m_maxEventIndex - i * 2) {
                QMutableEventPoint::setState(touchPoints.back(), QEventPoint::Released);
            }
        }
    }
};

struct TouchDragTwoThenThree : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<QEventPoint> &touchPoints) override
    {
        KIS_ASSERT(m_fingerCount == 2);
        KIS_ASSERT(m_maxEventIndex > 10);

        if (eventIndex == 0) {
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
        }

        if (eventIndex == m_maxEventIndex / 2) {
            appendNewPoint(touchPoints, eventIndex);
        }
    }
};

struct TouchDragThreeThenTwo : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<QEventPoint> &touchPoints) override
    {
        KIS_ASSERT(m_fingerCount == 3);
        KIS_ASSERT(m_maxEventIndex > 10);

        if (eventIndex == 0) {
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
        }

        if (eventIndex == m_maxEventIndex / 2) {
            QMutableEventPoint::setState(touchPoints.back(), QEventPoint::Released);
        }
    }
};

struct TouchDragThreeThenTwoThenThree : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<QEventPoint> &touchPoints) override
    {
        KIS_ASSERT(m_fingerCount == 3);
        KIS_ASSERT(m_maxEventIndex > 10);

        if (eventIndex == 0) {
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
        }

        if (eventIndex == m_maxEventIndex / 3) {
            QMutableEventPoint::setState(touchPoints.back(), QEventPoint::Released);
        }

        if (eventIndex == 2 * m_maxEventIndex / 3) {
            appendNewPoint(touchPoints, eventIndex);
        }
    }
};

TouchSequenceGeneratorBase* createTouchSequenceGenerator(const QString sequenceName, int fingerCount, int maxEventIndex, const QPointF pointOffset)
{
    if (sequenceName == "touchCleanDrag") {
        return new TouchCleanDrag(fingerCount, maxEventIndex, pointOffset);
    } else if (sequenceName == "touchDragDirtyStart") {
        return new TouchDragDirtyStart(fingerCount, maxEventIndex, pointOffset);
    } else if (sequenceName == "touchDragDirtyEnd") {
        return new TouchDragDirtyEnd(fingerCount, maxEventIndex, pointOffset);
    } else if (sequenceName == "touchDragTwoThenThree") {
        return new TouchDragTwoThenThree(fingerCount, maxEventIndex, pointOffset);
    } else if (sequenceName == "touchDragThreeThenTwo") {
        return new TouchDragThreeThenTwo(fingerCount, maxEventIndex, pointOffset);
    } else if (sequenceName == "touchDragThreeThenTwoThenThree") {
        return new TouchDragThreeThenTwoThenThree(fingerCount, maxEventIndex, pointOffset);
    } else {
        qFatal("Unknown touch sequence name");
    }

    Q_UNREACHABLE_RETURN(nullptr);
}

void KisInputManagerTest::testTouchMoves_data()
{
    QTest::addColumn<QString>("sequenceName");
    QTest::addColumn<int>("fingerCount");
    QTest::addColumn<int>("eventsCount");
    QTest::addColumn<QPointF>("pointOffset");
    QTest::addColumn<QList<int>>("triggeredShortcuts");

    const QPointF dragOffset(10,10);
    const QPointF dirtyDragOffset(1,1);
    const QPointF tapOffset(0.5,0.5);

    QTest::addRow("clean-1p-drag") << "touchCleanDrag" << 1 << 100 << dragOffset << QList<int>{15};
    QTest::addRow("clean-2p-drag") << "touchCleanDrag" << 2 << 100 << dragOffset << QList<int>{16};
    QTest::addRow("clean-3p-drag") << "touchCleanDrag" << 3 << 100 << dragOffset << QList<int>{17};

    QTest::addRow("clean-1p-tap") << "touchCleanDrag" << 1 << 10 << tapOffset << QList<int>{20};
    QTest::addRow("clean-2p-tap") << "touchCleanDrag" << 2 << 10 << tapOffset << QList<int>{21};
    QTest::addRow("clean-3p-tap") << "touchCleanDrag" << 3 << 10 << tapOffset << QList<int>{22};

    // 16px is the threshold for distinguishing taps and drags (touch-begin + 16 steps + touch-end)
    QTest::addRow("clean-1p-tap-upper-bound") << "touchCleanDrag" << 1 << 18 << QPointF(1, 0) << QList<int>{20};
    QTest::addRow("clean-1p-drag-lower-bound") << "touchCleanDrag" << 1 << 19 << QPointF(1, 0) << QList<int>{15};

    QTest::addRow("dirty-start-2p-drag") << "touchDragDirtyStart" << 2 << 100 << dirtyDragOffset << QList<int>{16};
    QTest::addRow("dirty-start-3p-drag") << "touchDragDirtyStart" << 3 << 100 << dirtyDragOffset << QList<int>{17};

    QTest::addRow("dirty-end-2p-drag") << "touchDragDirtyEnd" << 2 << 100 << dirtyDragOffset << QList<int>{16};
    QTest::addRow("dirty-end-3p-drag") << "touchDragDirtyEnd" << 3 << 100 << dirtyDragOffset << QList<int>{17};

    QTest::addRow("dirty-end-2p-tap") << "touchDragDirtyEnd" << 2 << 10 << tapOffset << QList<int>{21};
    QTest::addRow("dirty-end-3p-tap") << "touchDragDirtyEnd" << 3 << 10 << tapOffset << QList<int>{22};

    // the current implementation is expected to switch to a "more-fingers-rich" shortcut immetiately
    QTest::addRow("clean-2p-then-3p-drag") << "touchDragTwoThenThree" << 2 << 100 << dragOffset << QList<int>{16, 17};

    // the current implementation is expected to stay on the "finger-richest" shortcut and,
    // not to switch to a "finger-poorer" shortcut
    QTest::addRow("clean-3p-then-2p-drag") << "touchDragThreeThenTwo" << 3 << 100 << dragOffset << QList<int>{17};

    // the 3-point gesture should be run twice during the same run
    QTest::addRow("clean-3p-then-2p-then-3p-drag") << "touchDragThreeThenTwoThenThree" << 3 << 100 << dragOffset << QList<int>{17, 17};
}

void KisInputManagerTest::testTouchMoves()
{
    KisShortcutMatcher m;
    m.enterEvent();

    TestingAction *a = new TestingAction();

    m.addShortcut(
        createKeyShortcut(a, 10,
                          QSet<Qt::Key>() << Qt::Key_Shift,
                          Qt::Key_Enter));

    m.addShortcut(
        createStrokeShortcut(a, 13,
                             QSet<Qt::Key>(),
                             Qt::LeftButton));

    // TODO: add a separate test for that!
    // let single finger gestures to work
    KisConfig(false).setTouchPainting(KisConfig::TOUCH_PAINTING_DISABLED);

    m.addShortcut(
        createTouchShortcut(a, 15, KisShortcutConfiguration::OneFingerDrag));
    m.addShortcut(
        createTouchShortcut(a, 16, KisShortcutConfiguration::TwoFingerDrag));
    m.addShortcut(
        createTouchShortcut(a, 17, KisShortcutConfiguration::ThreeFingerDrag));

    m.addShortcut(
        createTouchShortcut(a, 20, KisShortcutConfiguration::OneFingerTap));
    m.addShortcut(
        createTouchShortcut(a, 21, KisShortcutConfiguration::TwoFingerTap));
    m.addShortcut(
        createTouchShortcut(a, 22, KisShortcutConfiguration::ThreeFingerTap));


    QFETCH(QString, sequenceName);
    QFETCH(int, fingerCount);
    QFETCH(int, eventsCount);
    QFETCH(QPointF, pointOffset);
    QFETCH(QList<int>, triggeredShortcuts);

    std::unique_ptr<TouchSequenceGeneratorBase> strokeGenerator(
        createTouchSequenceGenerator(sequenceName, fingerCount, eventsCount - 1, pointOffset));

    QPointingDevice device;
    QList<QEventPoint> touchPoints;

    for (int eventIndex = 0; eventIndex < eventsCount; eventIndex++) {
        strokeGenerator->updatePoints(eventIndex, touchPoints);
        if (eventIndex == 0) {
            QTouchEvent e(QEvent::TouchBegin, &device, Qt::NoModifier, touchPoints);
            m.touchBeginEvent(&e);
        } else if (eventIndex > 0 && eventIndex < eventsCount - 1) {
            QTouchEvent e(QEvent::TouchUpdate, &device, Qt::NoModifier, touchPoints);
            m.touchUpdateEvent(&e);
        } else if (eventIndex == eventsCount - 1) {
            QTouchEvent e(QEvent::TouchEnd, &device, Qt::NoModifier, touchPoints);
            m.touchEndEvent(&e);
        } else {
            qFatal("invalid step value");
        }
    }

    QCOMPARE(a->m_begunIndexes, triggeredShortcuts);
    QCOMPARE(a->m_endedIndexes, triggeredShortcuts);
    QCOMPARE(a->m_ended, true);
    //QCOMPARE(a->m_gotInput, true); // TODO!
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

SIMPLE_TEST_MAIN(KisInputManagerTest)
