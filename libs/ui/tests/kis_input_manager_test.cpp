/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2026 Ayanami Kaine <personal@ayanamikaine.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_manager_test.h"

#include <simpletest.h>

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
// for QMutableEventPoint
#include <QtGui/private/qeventpoint_p.h>
#endif

#include <QMouseEvent>

#include "input/kis_single_action_shortcut.h"
#include "input/kis_stroke_shortcut.h"
#include "input/kis_touch_shortcut.h"
#include "input/kis_abstract_input_action.h"
#include "input/kis_shortcut_matcher.h"

#include "input/KisTouchHoldEventsPostponer.h"

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

    TestingAction(const QString &id = "test")
        : KisAbstractInputAction(id)
        , m_isHighResolution(false)
    {
        setName(id);
        reset();
    }
    ~TestingAction() {}

    void activate(int shortcut) override
    {
        KIS_ASSERT(m_state == Deactivated);
        m_state = Activated;

        m_activatedShortcut = shortcut;
        m_activateIndex = shortcut;
        m_activateCount++;
    }

    void deactivate(int shortcut) override
    {
        KIS_ASSERT(m_state == Activated);
        KIS_ASSERT(m_activatedShortcut == shortcut);
        m_state = Deactivated;

        m_activatedShortcut = -1;
        m_deactivateIndex = shortcut;
        m_deactivateCount++;
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
        m_beginCount++;
        m_beginNonNull = event;

        m_inputEventCount++;
    }
    void end(QEvent *event) override {
        KIS_ASSERT(m_state == Running);
        m_state = m_stateBeforeRunning;
        m_endedIndexes.append(m_beginIndex);
        m_endCount++;

        m_ended = true; m_endNonNull = event;

        m_inputEventCount++;
    }
    void inputEvent(QEvent* event) override {
        KIS_ASSERT(m_state == Running);

        Q_UNUSED(event);
        m_gotInput = true;
        m_inputCount++;

        m_inputEventCount++;
    }
    int priority() const override { return m_priority; }
    bool canIgnoreModifiers() const override { return m_canIgnoreModifiers; }
    KisInputActionGroup inputActionGroup(int shortcut) const override { Q_UNUSED(shortcut); return m_inputActionGroup; }

    void reset() {
        m_activateIndex = -1;
        m_deactivateIndex = -1;
        m_beginIndex = -1;
        m_begunIndexes.clear();
        m_activateCount = 0;
        m_deactivateCount = 0;
        m_beginCount = 0;
        m_endCount = 0;
        m_inputCount = 0;
        m_ended = false;
        m_gotInput = false;
        m_inputEventCount = 0;
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
    int m_activateIndex;
    int m_deactivateIndex;
    int m_beginIndex;
    int m_activateCount;
    int m_deactivateCount;
    int m_beginCount;
    int m_endCount;
    int m_inputCount;
    bool m_ended;
    bool m_gotInput;
    bool m_beginNonNull;
    bool m_endNonNull;

    bool m_isHighResolution;
    State m_state {Deactivated};
    State m_stateBeforeRunning {Deactivated};
    QList<int> m_begunIndexes;
    QList<int> m_endedIndexes;
    int m_inputEventCount = 0;
    bool m_canIgnoreModifiers = false;
    int m_priority = 0;
    KisInputActionGroup m_inputActionGroup = ModifyingActionGroup;
};

struct SwitchableTestingAction : public TestingAction
{
    void activate(int shortcut) override
    {
        TestingAction::activate(shortcut);
        m_mode = shortcut;
        m_interactionState = 1;
    }

    void deactivate(int shortcut) override
    {
        TestingAction::deactivate(shortcut);
        m_interactionState = 0;
    }

    bool trySwitchShortcut(int oldShortcut, int newShortcut) override
    {
        m_switchCount++;
        m_oldShortcut = oldShortcut;
        m_newShortcut = newShortcut;

        if (!m_acceptSwitches) {
            return false;
        }

        KIS_ASSERT(m_state == Activated);
        KIS_ASSERT(m_activatedShortcut == oldShortcut);
        m_activatedShortcut = newShortcut;
        m_mode = newShortcut;
        return true;
    }

    int m_switchCount = 0;
    int m_oldShortcut = -1;
    int m_newShortcut = -1;
    bool m_acceptSwitches = true;
    int m_mode = -1;
    int m_interactionState = 0;
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
                                     KisShortcutConfiguration::TouchGestureAction gesture)
{
    KisTouchShortcut *s = new KisTouchShortcut(action, shortcutIndex, gesture);

    // TODO: move the initialization to the shortcut itself
    switch(gesture) {
#ifndef Q_OS_MACOS
    case KisShortcutConfiguration::OneFingerTap:
    case KisShortcutConfiguration::OneFingerDrag:
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    using TouchPoint = QTouchEvent::TouchPoint;

    struct TouchPointWrapper {
        enum State : quint8 {
            Unknown = 0,
            Stationary = Qt::TouchPointStationary,
            Pressed = Qt::TouchPointPressed,
            Updated = Qt::TouchPointMoved,
            Released = Qt::TouchPointReleased
        };

        QPointF globalPosition() const {
            return point.screenPos();
        }

        QPointF globalLastPosition() const {
            return point.lastScreenPos();
        }

        State state() const {
            return static_cast<State>(point.state());
        }

        // TODO: can we use a const-ref here as per c++?
        TouchPoint point;
    };

    TouchPointWrapper tpw(const TouchPoint &point) {
        return {point};
    }

    struct QMutableEventPoint {
        static void setGlobalLastPosition(TouchPoint &point, const QPointF &value) {
            point.setLastScreenPos(value);

            // mimic the behavior of Qt6
            point.setLastPos(value + point.pos() - point.screenPos());
        }

        static void setGlobalPressPosition(TouchPoint &point, const QPointF &value) {
            point.setStartScreenPos(value);

            // mimic the behavior of Qt6
            point.setStartPos(value + point.pos() - point.screenPos());
        }

        static void setGlobalPosition(TouchPoint &point, const QPointF &value) {
            point.setScreenPos(value);
        }

        static void setPosition(TouchPoint &point, const QPointF &value) {
            point.setPos(value);
        }

        static void setState(TouchPoint &point, TouchPointWrapper::State value) {
            point.setState(static_cast<Qt::TouchPointState>(value));
        }
    };

#else
    using TouchPoint = QEventPoint;
    using TouchPointWrapper = QEventPoint;

    TouchPointWrapper tpw(const TouchPoint &point) {
        return point;
    }
#endif

    TouchSequenceGeneratorBase(int fingerCount, int maxEventIndex, const QPointF pointOffset)
        : m_fingerCount(fingerCount)
        , m_maxEventIndex(maxEventIndex)
        , m_pointOffset(pointOffset)
    {
    }

    virtual ~TouchSequenceGeneratorBase() {}

    void updatePoints(int eventIndex, QList<TouchPoint> &touchPoints)
    {
        /**
         * First, remove all the points marked as "Release" at the previous step
         */
        for (auto it = touchPoints.begin(); it != touchPoints.end();) {
            if (tpw(*it).state() == TouchPointWrapper::Released) {
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
            QMutableEventPoint::setGlobalLastPosition(*it, tpw(*it).globalPosition());
            QMutableEventPoint::setGlobalPosition(*it, globalPositionForPoint(it->id(), eventIndex));
            QMutableEventPoint::setPosition(*it, tpw(*it).globalPosition());

            if (tpw(*it).state() == TouchPointWrapper::Pressed && tpw(*it).globalLastPosition() != tpw(*it).globalPosition()) {
                /**
                 * If a "Pressed" point has been offset, then we should change
                 * its state to "Updated"
                 */
                QMutableEventPoint::setState(*it, TouchPointWrapper::Updated);
            } else if (eventIndex == m_maxEventIndex) {
                /**
                 * At the very last iteration, mark all the points as
                 * "Released".
                 */
                QMutableEventPoint::setState(*it, TouchPointWrapper::Released);
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

    void appendNewPoint(QList<TouchPoint> &touchPoints, int eventIndex)
    {
        const int newIndex = touchPoints.size();
        const QPointF pos = globalPositionForPoint(10 * newIndex, eventIndex);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        TouchPoint point;
        point.setId(10 * newIndex);
        QMutableEventPoint::setState(point, TouchPointWrapper::Pressed);
        QMutableEventPoint::setGlobalPosition(point, pos);
        QMutableEventPoint::setPosition(point, pos);
        touchPoints.append(point);
#else
        touchPoints.emplace_back(10 * newIndex, QEventPoint::Pressed, pos, pos);
#endif
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
    virtual void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) = 0;

protected:
    int m_fingerCount = 0;
    int m_maxEventIndex = 0;
    const QPointF m_pointOffset;
};

struct TouchCleanDrag : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) override
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
    void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) override
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
    void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) override
    {
        if (eventIndex == 0) {
            for (int i = 0; i < m_fingerCount; ++i) {
                appendNewPoint(touchPoints, eventIndex);
            }
        }

        for (int i = 0; i < m_fingerCount; i++) {
            if (eventIndex == m_maxEventIndex - i * 2) {
                QMutableEventPoint::setState(touchPoints.back(), TouchPointWrapper::Released);
            }
        }
    }
};

struct TouchDragTwoThenThree : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) override
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
    void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) override
    {
        KIS_ASSERT(m_fingerCount == 3);
        KIS_ASSERT(m_maxEventIndex > 10);

        if (eventIndex == 0) {
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
        }

        if (eventIndex == m_maxEventIndex / 2) {
            QMutableEventPoint::setState(touchPoints.back(), TouchPointWrapper::Released);
        }
    }
};

struct TouchDragThreeThenTwoThenThree : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) override
    {
        KIS_ASSERT(m_fingerCount == 3);
        KIS_ASSERT(m_maxEventIndex > 10);

        if (eventIndex == 0) {
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
            appendNewPoint(touchPoints, eventIndex);
        }

        if (eventIndex == m_maxEventIndex / 3) {
            QMutableEventPoint::setState(touchPoints.back(), TouchPointWrapper::Released);
        }

        if (eventIndex == 2 * m_maxEventIndex / 3) {
            appendNewPoint(touchPoints, eventIndex);
        }
    }
};

struct TouchSlowDrag50ms : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) override
    {
        if (eventIndex == 0) {
            for (int i = 0; i < m_fingerCount; i++) {
                appendNewPoint(touchPoints, eventIndex);
            }
        }

        if (eventIndex > 0) {
            QTest::qWait(50);
        }
    }
};

struct TouchHoldFirstFourUpdateEventsFor50ms : TouchSequenceGeneratorBase
{
    using TouchSequenceGeneratorBase::TouchSequenceGeneratorBase;

protected:
    void updatePointsSet(int eventIndex, QList<TouchPoint> &touchPoints) override
    {
        if (eventIndex == 0) {
            for (int i = 0; i < m_fingerCount; i++) {
                appendNewPoint(touchPoints, eventIndex);
            }
        }

        if (eventIndex > 0 && eventIndex < 5) {
            QTest::qWait(50);
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
    } else if (sequenceName == "touchSlowDrag50ms") {
        return new TouchSlowDrag50ms(fingerCount, maxEventIndex, pointOffset);
    } else if (sequenceName == "touchHoldFirstFourUpdateEventsFor50ms") {
        return new TouchHoldFirstFourUpdateEventsFor50ms(fingerCount, maxEventIndex, pointOffset);
    } else {
        qFatal("Unknown touch sequence name");
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return nullptr;
#else
    Q_UNREACHABLE_RETURN(nullptr);
#endif
}



void KisInputManagerTest::testTouchMoves_data()
{
    QTest::addColumn<QString>("sequenceName");
    QTest::addColumn<int>("fingerCount");
    QTest::addColumn<int>("eventsCount");
    QTest::addColumn<QPointF>("pointOffset");
    QTest::addColumn<bool>("enableTouchPainting");
    QTest::addColumn<bool>("enableHoldShortcuts");
    QTest::addColumn<bool>("cancelShortcut");
    QTest::addColumn<QList<int>>("triggeredTouchShortcuts");
    QTest::addColumn<QList<int>>("triggeredPaintShortcuts");
    QTest::addColumn<int>("actionEventsCount"); // `-1` to ignore this test
    QTest::addColumn<int>("paintEventsCount"); // `-1` to ignore this test

    const QPointF dragOffset(10,10);
    const QPointF dirtyDragOffset(1,1);
    const QPointF tapOffset(0.5,0.5);
    const QPointF smallTapOffset(0.1,0.1);

    const bool touchPaintingOn = true;
    const bool touchPaintingOff = false;
    const bool cancelShortcutOn = true;
    const bool cancelShortcutOff = false;
    const int ignoreActionEventCount = -1;
    const int ignorePaintEventCount = -1;

    for (bool enableHoldShortcuts : {false, true}) {
        // clang-format off

        const char *holdPrefix = enableHoldShortcuts ? "hold_enabled-" : "";

        QTest::addRow("%sclean-1p-drag", holdPrefix) << "touchCleanDrag" << 1 << 100 << dragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{15} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-2p-drag", holdPrefix) << "touchCleanDrag" << 2 << 100 << dragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{16} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-3p-drag", holdPrefix) << "touchCleanDrag" << 3 << 100 << dragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        QTest::addRow("%sclean-1p-drag-paint", holdPrefix) << "touchCleanDrag" << 1 << 100 << dragOffset << touchPaintingOn << enableHoldShortcuts << cancelShortcutOff << QList<int>{} << QList<int>{30} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-2p-drag-paint", holdPrefix) << "touchCleanDrag" << 2 << 100 << dragOffset << touchPaintingOn << enableHoldShortcuts << cancelShortcutOff << QList<int>{16} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-3p-drag-paint", holdPrefix) << "touchCleanDrag" << 3 << 100 << dragOffset << touchPaintingOn << enableHoldShortcuts << cancelShortcutOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        // painting skips only the first two events
        QTest::addRow("%sclean-1p-drag-paint-delayed-start", holdPrefix) << "touchCleanDrag" << 1 << 100 << QPointF(1.0, 0) << touchPaintingOn << enableHoldShortcuts << cancelShortcutOff << QList<int>{} << QList<int>{30} << ignoreActionEventCount << 98;
        // action has higher threshold, so it skips the first 17 events
        QTest::addRow("%sclean-1p-drag-delayed-start", holdPrefix) << "touchCleanDrag" << 1 << 100 << QPointF(1.0, 0) << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{15} << QList<int>{} << 83 << ignorePaintEventCount;

        QTest::addRow("%sclean-1p-tap", holdPrefix) << "touchCleanDrag" << 1 << 10 << tapOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{20} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-2p-tap", holdPrefix) << "touchCleanDrag" << 2 << 10 << tapOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{21} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-3p-tap", holdPrefix) << "touchCleanDrag" << 3 << 10 << tapOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{22} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        // touch painting stroke has a lower drag threshold, so we should use smaller offset value for it
        QTest::addRow("%sclean-1p-tap-paint", holdPrefix) << "touchCleanDrag" << 1 << 10 << smallTapOffset << touchPaintingOn << enableHoldShortcuts << cancelShortcutOff << QList<int>{} << QList<int>{31} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-2p-tap-paint", holdPrefix) << "touchCleanDrag" << 2 << 10 << tapOffset << touchPaintingOn << enableHoldShortcuts << cancelShortcutOff << QList<int>{21} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-3p-tap-paint", holdPrefix) << "touchCleanDrag" << 3 << 10 << tapOffset << touchPaintingOn << enableHoldShortcuts << cancelShortcutOff << QList<int>{22} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        // 16px is the threshold for distinguishing taps and drags (touch-begin + 16 steps + touch-end)
        QTest::addRow("%sclean-1p-tap-upper-bound", holdPrefix) << "touchCleanDrag" << 1 << 18 << QPointF(1, 0) << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{20} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sclean-1p-drag-lower-bound", holdPrefix) << "touchCleanDrag" << 1 << 19 << QPointF(1, 0) << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{15} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        QTest::addRow("%sdirty-start-2p-drag", holdPrefix) << "touchDragDirtyStart" << 2 << 100 << dirtyDragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{16} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sdirty-start-3p-drag", holdPrefix) << "touchDragDirtyStart" << 3 << 100 << dirtyDragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        QTest::addRow("%sdirty-end-2p-drag", holdPrefix) << "touchDragDirtyEnd" << 2 << 100 << dirtyDragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{16} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sdirty-end-3p-drag", holdPrefix) << "touchDragDirtyEnd" << 3 << 100 << dirtyDragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        QTest::addRow("%sdirty-end-2p-tap", holdPrefix) << "touchDragDirtyEnd" << 2 << 10 << tapOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{21} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
        QTest::addRow("%sdirty-end-3p-tap", holdPrefix) << "touchDragDirtyEnd" << 3 << 10 << tapOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{22} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        // the current implementation is expected to switch to a "more-fingers-rich" shortcut immetiately
        QTest::addRow("%sclean-2p-then-3p-drag", holdPrefix) << "touchDragTwoThenThree" << 2 << 100 << dragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{16, 17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        // the current implementation is expected to stay on the "finger-richest" shortcut and,
        // not to switch to a "finger-poorer" shortcut
        QTest::addRow("%sclean-3p-then-2p-drag", holdPrefix) << "touchDragThreeThenTwo" << 3 << 100 << dragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        // the 3-point gesture should be run twice during the same run
        QTest::addRow("%sclean-3p-then-2p-then-3p-drag", holdPrefix) << "touchDragThreeThenTwoThenThree" << 3 << 100 << dragOffset << touchPaintingOff << enableHoldShortcuts << cancelShortcutOff << QList<int>{17, 17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

        // the amount of paint events should **not** depend on whether we have hold enabled or not
        QTest::addRow("%sclean-1p-drag-paint-very-short", holdPrefix) << "touchCleanDrag" << 1 << 15 << QPointF(1.0, 0) << touchPaintingOn << enableHoldShortcuts << cancelShortcutOff << QList<int>{} << QList<int>{30} << ignoreActionEventCount << 13;

        // clang-format on
    }

    // clang-format off

    const bool holdShortcutsOn = true;
    const bool holdShortcutsOff = false;

    QTest::addRow("hold-drag-1p") << "touchHoldFirstFourUpdateEventsFor50ms" << 1 << 100 << QPointF(1.0, 0) << touchPaintingOn << holdShortcutsOn << cancelShortcutOff<< QList<int>{40} << QList<int>{} << 97 << ignorePaintEventCount;

    /// the cancellation test cases

    // normal ending shortcuts, i.e. the last event is "TouchEnd"
    QTest::addRow("normal_ending-clean-2p-drag-hold-off") << "touchCleanDrag" << 2 << 100 << dragOffset << touchPaintingOff << holdShortcutsOff << cancelShortcutOff << QList<int>{16} << QList<int>{} << 98 << ignorePaintEventCount;
    QTest::addRow("normal_ending-clean-2p-tap-hold-off") << "touchCleanDrag" << 2 << 10 << tapOffset << touchPaintingOff << holdShortcutsOff << cancelShortcutOff << QList<int>{21} << QList<int>{} << 2 << ignorePaintEventCount;
    QTest::addRow("normal_ending-clean-3p-tap-hold-off") << "touchCleanDrag" << 3 << 10 << tapOffset << touchPaintingOff << holdShortcutsOff << cancelShortcutOff <<QList<int>{22} << QList<int>{} << 2 << ignorePaintEventCount;
    // hold timeout was not reached, so the touch-paiting-tap shortcut is triggered instead
    QTest::addRow("normal_ending-hold-drag-1p-before-delay") << "touchHoldFirstFourUpdateEventsFor50ms" << 1 << 3 << QPointF(1.0, 0) << touchPaintingOn << holdShortcutsOn << cancelShortcutOff << QList<int>{} << QList<int>{31} << ignoreActionEventCount << 2;
    QTest::addRow("normal_ending-hold-drag-1p-after-delay") << "touchHoldFirstFourUpdateEventsFor50ms" << 1 << 6 << QPointF(1.0, 0) << touchPaintingOn << holdShortcutsOn << cancelShortcutOff <<QList<int>{40} << QList<int>{} << 3 << ignorePaintEventCount;

    // cancelled shortcuts
    // the drag shortcut is ended normally
    QTest::addRow("cancelled-clean-2p-drag-hold-off") << "touchCleanDrag" << 2 << 100 << dragOffset << touchPaintingOff << holdShortcutsOff << cancelShortcutOn << QList<int>{16} << QList<int>{} << 98 << ignorePaintEventCount;
    // tap shortcut is cancelled
    QTest::addRow("cancelled-clean-2p-tap-hold-off") << "touchCleanDrag" << 2 << 10 << tapOffset << touchPaintingOff << holdShortcutsOff << cancelShortcutOn <<QList<int>{} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    // hold shortcut didn't have time to start, so it is completely cancelled, no touch-painting stroke is started
    QTest::addRow("cancelled-hold-drag-1p-before-delay") << "touchHoldFirstFourUpdateEventsFor50ms" << 1 << 3 << QPointF(1.0, 0) << touchPaintingOn << holdShortcutsOn << cancelShortcutOn << QList<int>{} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    // hold shortcut has already started, so just end it gracefully
    QTest::addRow("cancelled-hold-drag-1p-after-delay") << "touchHoldFirstFourUpdateEventsFor50ms" << 1 << 6 << QPointF(1.0, 0) << touchPaintingOn << holdShortcutsOn << cancelShortcutOn << QList<int>{40} << QList<int>{} << 3 << ignorePaintEventCount;

    // 3+ finger shortcuts have a special workaround on Android, they should never be cancelled,
    // because some Xiaomi tablets deliver Cancel events for them even when three-finger shortcuts
    // are disabled in the system.

    // three-finger-tap shortcut is still present, even though it is cancelled
    QTest::addRow("cancelled-clean-3p-tap-hold-off") << "touchCleanDrag" << 3 << 10 << tapOffset << touchPaintingOff << holdShortcutsOff << cancelShortcutOn <<QList<int>{22} << QList<int>{} << 2 << ignorePaintEventCount;

    // clang-format on
}

void KisInputManagerTest::testTouchMoves()
{
    std::unique_ptr<TestingAction> paintAction(new TestingAction("paint-action"));
    std::unique_ptr<TestingAction> a(new TestingAction("touch-action"));

    KisShortcutMatcher m;
    // the timeout should happen on the fourth event after the first (with a 30 ms margin)
    m.setTouchHoldDelay(170);
    m.setIgnoreMultiFingerCancelWorkaroundEnalbed(true);
    m.enterEvent();

    m.addShortcut(
        createKeyShortcut(a.get(), 10,
                          QSet<Qt::Key>() << Qt::Key_Shift,
                          Qt::Key_Enter));

    m.addShortcut(
        createStrokeShortcut(a.get(), 13,
                             QSet<Qt::Key>(),
                             Qt::LeftButton));

    m.addShortcut(
        createTouchShortcut(a.get(), 15, KisShortcutConfiguration::OneFingerDrag));
    m.addShortcut(
        createTouchShortcut(a.get(), 16, KisShortcutConfiguration::TwoFingerDrag));
    m.addShortcut(
        createTouchShortcut(a.get(), 17, KisShortcutConfiguration::ThreeFingerDrag));

    m.addShortcut(
        createTouchShortcut(a.get(), 20, KisShortcutConfiguration::OneFingerTap));
    m.addShortcut(
        createTouchShortcut(a.get(), 21, KisShortcutConfiguration::TwoFingerTap));
    m.addShortcut(
        createTouchShortcut(a.get(), 22, KisShortcutConfiguration::ThreeFingerTap));

    m.addShortcut(
        createTouchShortcut(a.get(), 22, KisShortcutConfiguration::ThreeFingerTap));

    {
        // Touch painting shortcuts
        auto *shortcutDrag = createTouchShortcut(paintAction.get(), 30, KisShortcutConfiguration::OneFingerDrag);
        shortcutDrag->setIsTouchPainting(true);
        shortcutDrag->setMinDragThreshold(1.5);
        m.addShortcut(shortcutDrag);

        auto *shortcutTap = createTouchShortcut(paintAction.get(), 31, KisShortcutConfiguration::OneFingerTap);
        shortcutTap->setIsTouchPainting(true);
        m.addShortcut(shortcutTap);
    }

    QFETCH(QString, sequenceName);
    QFETCH(int, fingerCount);
    QFETCH(int, eventsCount);
    QFETCH(QPointF, pointOffset);
    QFETCH(bool, enableTouchPainting);
    QFETCH(bool, enableHoldShortcuts);
    QFETCH(bool, cancelShortcut);
    QFETCH(QList<int>, triggeredTouchShortcuts);
    QFETCH(QList<int>, triggeredPaintShortcuts);
    QFETCH(int, actionEventsCount);
    QFETCH(int, paintEventsCount);

    if (enableHoldShortcuts) {
        m.addShortcut(
            createTouchShortcut(a.get(), 40, KisShortcutConfiguration::OneFingerHold));
    }
    QCOMPARE(m.hasTouchHoldShortcut(), enableHoldShortcuts);

    KisConfig(false).setTouchPainting(enableTouchPainting ? KisConfig::TOUCH_PAINTING_ENABLED
                                                          : KisConfig::TOUCH_PAINTING_DISABLED);

    std::unique_ptr<TouchSequenceGeneratorBase> strokeGenerator(
        createTouchSequenceGenerator(sequenceName, fingerCount, eventsCount - 1, pointOffset));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTouchDevice device;
    auto calcTouchPointStates = [] (const QList<TouchSequenceGeneratorBase::TouchPoint> &touchPoints) -> Qt::TouchPointStates {
        Qt::TouchPointStates result;

        for (auto it = touchPoints.begin(); it != touchPoints.end(); ++it) {
            result |= it->state();
        }

        return result;
    };
#else
    QPointingDevice device;
#endif

    QList<TouchSequenceGeneratorBase::TouchPoint> touchPoints;

    for (int eventIndex = 0; eventIndex < eventsCount; eventIndex++) {
        strokeGenerator->updatePoints(eventIndex, touchPoints);
        if (eventIndex == 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QTouchEvent e(QEvent::TouchBegin, &device, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints);
#else
            QTouchEvent e(QEvent::TouchBegin, &device, Qt::NoModifier, touchPoints);
#endif
            m.touchBeginEvent(&e);
        } else if (eventIndex > 0 && eventIndex < eventsCount - 1) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QTouchEvent e(QEvent::TouchUpdate, &device, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints);
#else
            QTouchEvent e(QEvent::TouchUpdate, &device, Qt::NoModifier, touchPoints);
#endif
            m.touchUpdateEvent(&e);
        } else if (eventIndex == eventsCount - 1) {
            QEvent::Type endingType = !cancelShortcut ? QEvent::TouchEnd : QEvent::TouchCancel;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QTouchEvent e(endingType, &device, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints);
#else
            QTouchEvent e(endingType, &device, Qt::NoModifier, touchPoints);
#endif
            if (e.type() == QEvent::TouchEnd) {
                m.touchEndEvent(&e);
            } else {
                m.touchCancelEvent(&e);
            }
        } else {
            qFatal("invalid step value");
        }
    }

    QCOMPARE(a->m_begunIndexes, triggeredTouchShortcuts);
    QCOMPARE(a->m_endedIndexes, triggeredTouchShortcuts);
    QCOMPARE(a->m_ended, !a->m_begunIndexes.isEmpty());

    QCOMPARE(paintAction->m_begunIndexes, triggeredPaintShortcuts);
    QCOMPARE(paintAction->m_endedIndexes, triggeredPaintShortcuts);
    QCOMPARE(paintAction->m_ended, !paintAction->m_begunIndexes.isEmpty());

    qDebug() << ppVar(a->m_inputEventCount) << ppVar(paintAction->m_inputEventCount);

    if (actionEventsCount >= 0) {
        QCOMPARE(a->m_inputEventCount, actionEventsCount);
    }

    if (paintEventsCount >= 0) {
        QCOMPARE(paintAction->m_inputEventCount, paintEventsCount);
    }
}

void KisInputManagerTest::testTouchHoldPostponer_data()
{
    QTest::addColumn<QString>("sequenceName");
    QTest::addColumn<int>("fingerCount");
    QTest::addColumn<int>("eventsCount");
    QTest::addColumn<QPointF>("pointOffset");
    QTest::addColumn<int>("holdTimeout");
    QTest::addColumn<int>("cancelOnIndex");
    QTest::addColumn<int>("completeOnIndex");

    const int noCancel = -1;
    const int noComplete = -1;

    const QPointF dragOffset(1,0);
    // the timeout should happen on the fourth event after the first (with a 30 ms margin)
    const int holdTimeout = 170 /* ms */;

    QTest::addRow("clean-1p-drag") << "touchCleanDrag" << 1 << 100 << dragOffset << holdTimeout << 21 << noComplete;
    QTest::addRow("clean-2p-drag") << "touchCleanDrag" << 2 << 100 << dragOffset << holdTimeout << 0 << noComplete;
    QTest::addRow("dirty-2p-drag") << "touchDragDirtyStart" << 2 << 100 << dragOffset << holdTimeout << 2 << noComplete;

    QTest::addRow("clean-1p-slow-drag") << "touchSlowDrag50ms" << 1 << 100 << dragOffset << holdTimeout << noCancel << 4;
}

void KisInputManagerTest::testTouchHoldPostponer()
{
    QFETCH(QString, sequenceName);
    QFETCH(int, fingerCount);
    QFETCH(int, eventsCount);
    QFETCH(QPointF, pointOffset);
    QFETCH(int, holdTimeout);
    QFETCH(int, cancelOnIndex);
    QFETCH(int, completeOnIndex);

    std::unique_ptr<TouchSequenceGeneratorBase> strokeGenerator(
        createTouchSequenceGenerator(sequenceName, fingerCount, eventsCount - 1, pointOffset));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTouchDevice device;
    auto calcTouchPointStates = [] (const QList<TouchSequenceGeneratorBase::TouchPoint> &touchPoints) -> Qt::TouchPointStates {
        Qt::TouchPointStates result;

        for (auto it = touchPoints.begin(); it != touchPoints.end(); ++it) {
            result |= it->state();
        }

        return result;
    };
#else
    QPointingDevice device;
#endif

    QList<TouchSequenceGeneratorBase::TouchPoint> touchPoints;

    KisTouchHoldEventsPostponer postponer(20.0, holdTimeout);

    bool holdHasBeenCompleted = false;
    connect(&postponer, &KisTouchHoldEventsPostponer::sigHoldCompleted, this, [&] () {
        holdHasBeenCompleted = true;
    });

    for (int eventIndex = 0; eventIndex < eventsCount; eventIndex++) {
        std::unique_ptr<QTouchEvent> event;

        // qWait() will happen here (if it does at all)
        strokeGenerator->updatePoints(eventIndex, touchPoints);

        // if the timer event has arrived, then verify consistency and exit
        if (postponer.state() == KisTouchHoldEventsPostponer::HoldCompleted || holdHasBeenCompleted) {
            QVERIFY(holdHasBeenCompleted);
            QCOMPARE(postponer.state(), KisTouchHoldEventsPostponer::HoldCompleted);
            QCOMPARE(eventIndex, completeOnIndex);

            // the last event has not yet been pushed, so no "+1"
            QCOMPARE(postponer.postponedEvents().size(), eventIndex);

            break;
        }

        if (eventIndex == 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            event.reset(new QTouchEvent(QEvent::TouchBegin, &device, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints));
#else
            event.reset(new QTouchEvent(QEvent::TouchBegin, &device, Qt::NoModifier, touchPoints));
#endif
        } else if (eventIndex > 0 && eventIndex < eventsCount - 1) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            event.reset(new QTouchEvent(QEvent::TouchUpdate, &device, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints));
#else
            event.reset(new QTouchEvent(QEvent::TouchUpdate, &device, Qt::NoModifier, touchPoints));
#endif
        } else if (eventIndex == eventsCount - 1) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            event.reset(new QTouchEvent(QEvent::TouchEnd, &device, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints));
#else
            event.reset(new QTouchEvent(QEvent::TouchEnd, &device, Qt::NoModifier, touchPoints));
#endif
        } else {
            qFatal("invalid step value");
        }

        postponer.pushThrough(event.get());

        if (cancelOnIndex >= 0 && cancelOnIndex == eventIndex) {
            QCOMPARE(postponer.state(), KisTouchHoldEventsPostponer::HoldCancelled);

            // the last event must also have been pushed into the postponed events queue
            // to be handled uniformly with the rest of the events
            QCOMPARE(postponer.postponedEvents().size(), eventIndex + 1);

            break;
        } else {
            QCOMPARE(postponer.state(), KisTouchHoldEventsPostponer::WaitingForHold);
        }
    }
}

void KisInputManagerTest::testTouchOverriddenByTablet_data()
{
    QTest::addColumn<QString>("sequenceName");
    QTest::addColumn<QPointF>("pointOffset");
    QTest::addColumn<int>("tabletStartEventIndex");
    QTest::addColumn<QList<int>>("triggeredShortcuts");

    const QPointF dragOffset(10,0);
    const QPointF holdOffset(1,0);
    const QPointF tapOffset(0,0);
    const int tabletStartsBeforeTouch = -1;

    QTest::addRow("clean-1p-drag-no-tablet") << "touchCleanDrag" << dragOffset << 200 << QList<int>{15};
    QTest::addRow("clean-1p-hold-no-tablet") << "touchHoldFirstFourUpdateEventsFor50ms" << holdOffset << 200 << QList<int>{40};
    QTest::addRow("clean-1p-tap-no-tablet") << "touchCleanDrag" << tapOffset << 200 << QList<int>{20};
    QTest::addRow("clean-1p-drag-slow-start-no-tablet") << "touchCleanDrag" << holdOffset << 200 << QList<int>{15};

    /// tablet action has been started while touch drag was running,
    /// the touch action is ended and a tablet action is started
    QTest::addRow("clean-1p-drag") << "touchCleanDrag" << dragOffset << 2 << QList<int>{15, 50};

    /// tablet action has been started while touch-hold was being waited for,
    /// the touch-hold action should never start until the next touch-begin
    QTest::addRow("clean-1p-hold") << "touchHoldFirstFourUpdateEventsFor50ms" << holdOffset << 2 << QList<int>{50};

    /// tablet action has been started before the touch-drag hasn't reached the
    /// drash threshold, the touch-drag action should never start until
    /// the next touch-begin
    QTest::addRow("clean-1p-drag-slow-start") << "touchCleanDrag" << holdOffset << 2 << QList<int>{50};

    /// touch-drag is requested during a tablet stroke, nothig should happen
    QTest::addRow("clean-1p-drag-over-tablet") << "touchCleanDrag" << dragOffset << tabletStartsBeforeTouch << QList<int>{50};

    /// touch-hold is requested during a tablet stroke, nothig should happen
    QTest::addRow("clean-1p-hold-over-tablet") << "touchHoldFirstFourUpdateEventsFor50ms" << holdOffset << tabletStartsBeforeTouch << QList<int>{50};

    /// touch-tap is requested during the tablet stroke, nothig should happen
    QTest::addRow("clean-1p-tap-over-tablet") << "touchCleanDrag" << tapOffset << tabletStartsBeforeTouch << QList<int>{50};
}

class TabletEventsGenerator
{
public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    using PointingDeviceHandle = int;
#else
    using PointingDeviceHandle = QPointingDevice*;
#endif

public:
    TabletEventsGenerator(int startEventIndex, int endEventIndex, KisShortcutMatcher &matcher, PointingDeviceHandle device)
        : m_matcher(matcher)
        , m_device(device)
        , m_startEventIndex(startEventIndex)
        , m_endEventIndex(endEventIndex)
    {
    }

    void sendTabletEvent(int eventIndex) {
        if (eventIndex == m_startEventIndex) {
            const QPointF pos(eventIndex, 0.0);
            std::unique_ptr<QTabletEvent> event =
                makeTabletEvent(QEvent::TabletPress, pos);
            m_matcher.buttonPressed(Qt::LeftButton, event.get());
        } else if (eventIndex > m_startEventIndex && eventIndex < m_endEventIndex) {
            const QPointF pos(eventIndex, 0.0);
            std::unique_ptr<QTabletEvent> event =
                makeTabletEvent(QEvent::TabletMove, pos);
            m_matcher.pointerMoved(event.get());
        } else if (eventIndex == m_endEventIndex) {
            const QPointF pos(eventIndex, 0.0);
            std::unique_ptr<QTabletEvent> event =
                makeTabletEvent(QEvent::TabletRelease, pos);
            m_matcher.buttonReleased(Qt::LeftButton, event.get());
        }
    }
private:
std::unique_ptr<QTabletEvent> makeTabletEvent(QEvent::Type type, const QPointF pos)
{
    std::unique_ptr<QTabletEvent> event(new QTabletEvent(type,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                                                         m_device,
#endif
                                                         pos,
                                                         pos,
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                                                         m_device,
                                                         QTabletEvent::Stylus,
#endif
                                                         1.0,
                                                         0.0,
                                                         0.0,
                                                         0.0,
                                                         0.0,
                                                         0.0,
                                                         Qt::NoModifier,
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                                                         4815162342,
#endif
                                                         type != QEvent::TabletMove ? Qt::LeftButton : Qt::NoButton,
                                                         type != QEvent::TabletRelease ? Qt::LeftButton : Qt::NoButton));
    return event;
}

private:
    KisShortcutMatcher &m_matcher;
    PointingDeviceHandle m_device;
    int m_startEventIndex;
    int m_endEventIndex;
};

void KisInputManagerTest::testTouchOverriddenByTablet()
{
    QFETCH(QString, sequenceName);
    QFETCH(QPointF, pointOffset);
    QFETCH(int, tabletStartEventIndex);
    QFETCH(QList<int>, triggeredShortcuts);

    const int eventsCount = 100;

    std::unique_ptr<TestingAction> paintAction(new TestingAction("paint-action"));
    std::unique_ptr<TestingAction> a(new TestingAction("touch-action"));

    KisShortcutMatcher m;
    // the timeout should happen on the fourth event after the first (with a 30 ms margin)
    m.setTouchHoldDelay(170);
    m.enterEvent();

    m.addShortcut(
        createTouchShortcut(a.get(), 15, KisShortcutConfiguration::OneFingerDrag));

    m.addShortcut(
            createTouchShortcut(a.get(), 20, KisShortcutConfiguration::OneFingerTap));

    m.addShortcut(
            createTouchShortcut(a.get(), 40, KisShortcutConfiguration::OneFingerHold));

    m.addShortcut(
            createStrokeShortcut(a.get(), 50, {}, Qt::LeftButton));

    KisConfig(false).setTouchPainting(KisConfig::TOUCH_PAINTING_DISABLED);

    std::unique_ptr<TouchSequenceGeneratorBase> touchStrokeEventsGenerator(
        createTouchSequenceGenerator(sequenceName, 1, eventsCount - 1, pointOffset));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTouchDevice touchDevice;
    auto calcTouchPointStates = [] (const QList<TouchSequenceGeneratorBase::TouchPoint> &touchPoints) -> Qt::TouchPointStates {
        Qt::TouchPointStates result;

        for (auto it = touchPoints.begin(); it != touchPoints.end(); ++it) {
            result |= it->state();
        }

        return result;
    };
    int tabletDeviceHandle = 1;
#else
    QPointingDevice touchDevice;
    QPointingDevice tabletDeviceObject;
    QPointingDevice *tabletDeviceHandle = &tabletDeviceObject;
#endif

    enum TabletOverrideMode {
        ModeTabletOverTouch,
        ModeTouchOverTablet
    };

    const TabletOverrideMode overrideMode = tabletStartEventIndex >= 0 ? ModeTabletOverTouch : ModeTouchOverTablet;

    std::optional<TabletEventsGenerator> tabletStrokeGenerator;

    if (overrideMode == ModeTabletOverTouch) {
        tabletStrokeGenerator.emplace(tabletStartEventIndex, tabletStartEventIndex + 10, m, tabletDeviceHandle);
    } else if (overrideMode == ModeTouchOverTablet) {
        tabletStrokeGenerator.emplace(0, 2, m, tabletDeviceHandle);
        tabletStrokeGenerator->sendTabletEvent(0);
        tabletStrokeGenerator->sendTabletEvent(1);
    }

    QList<TouchSequenceGeneratorBase::TouchPoint> touchPoints;

    for (int eventIndex = 0; eventIndex < eventsCount; eventIndex++) {
        touchStrokeEventsGenerator->updatePoints(eventIndex, touchPoints);
        if (eventIndex == 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QTouchEvent e(QEvent::TouchBegin, &touchDevice, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints);
#else
            QTouchEvent e(QEvent::TouchBegin, &touchDevice, Qt::NoModifier, touchPoints);
#endif
            m.touchBeginEvent(&e);
        } else if (eventIndex > 0 && eventIndex < eventsCount - 1) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QTouchEvent e(QEvent::TouchUpdate, &touchDevice, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints);
#else
            QTouchEvent e(QEvent::TouchUpdate, &touchDevice, Qt::NoModifier, touchPoints);
#endif
            m.touchUpdateEvent(&e);
        } else if (eventIndex == eventsCount - 1) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QTouchEvent e(QEvent::TouchEnd, &touchDevice, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints);
#else
            QTouchEvent e(QEvent::TouchEnd, &touchDevice, Qt::NoModifier, touchPoints);
#endif
            m.touchEndEvent(&e);
        } else {
            qFatal("invalid step value");
        }

        if (overrideMode == ModeTabletOverTouch) {
            tabletStrokeGenerator->sendTabletEvent(eventIndex);
        }
    }

    if (overrideMode == ModeTouchOverTablet) {
        tabletStrokeGenerator->sendTabletEvent(2);
    }

    QCOMPARE(a->m_begunIndexes, triggeredShortcuts);
    QCOMPARE(a->m_endedIndexes, triggeredShortcuts);
}

void KisInputManagerTest::testStrokeShortcutSwitchFallback()
{
    constexpr int normalShortcut = 0;
    constexpr int snappedShortcut = 1;

    KisShortcutMatcher m;
    m.enterEvent();

    TestingAction *normalAction = new TestingAction();
    TestingAction *snappedAction = new TestingAction();

    m.addShortcut(
        createStrokeShortcut(normalAction, normalShortcut,
                             QSet<Qt::Key>() << Qt::Key_V,
                             Qt::LeftButton));
    m.addShortcut(
        createStrokeShortcut(snappedAction, snappedShortcut,
                             QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_V,
                             Qt::LeftButton));

    QVERIFY(!normalAction->trySwitchShortcut(normalShortcut, snappedShortcut));

    QVERIFY(!m.keyPressed(Qt::Key_V));
    QCOMPARE(normalAction->m_activateCount, 1);
    QCOMPARE(normalAction->m_deactivateCount, 0);

    QVERIFY(!m.keyPressed(Qt::Key_Shift));
    QCOMPARE(normalAction->m_deactivateCount, 1);
    QCOMPARE(normalAction->m_deactivateIndex, normalShortcut);
    QCOMPARE(snappedAction->m_activateCount, 1);
    QCOMPARE(snappedAction->m_activateIndex, snappedShortcut);

    QVERIFY(!m.keyReleased(Qt::Key_Shift));
    QCOMPARE(snappedAction->m_deactivateCount, 1);
    QCOMPARE(snappedAction->m_deactivateIndex, snappedShortcut);
    QCOMPARE(normalAction->m_activateCount, 2);

    QVERIFY(!m.keyReleased(Qt::Key_V));
    QCOMPARE(normalAction->m_deactivateCount, 2);
}

void KisInputManagerTest::testStrokeShortcutSwitchModes()
{
    constexpr int normalShortcut = 0;
    constexpr int snappedShortcut = 1;

    KisShortcutMatcher m;
    m.enterEvent();

    SwitchableTestingAction *action = new SwitchableTestingAction();

    m.addShortcut(
        createStrokeShortcut(action, normalShortcut,
                             QSet<Qt::Key>() << Qt::Key_V,
                             Qt::LeftButton));
    m.addShortcut(
        createStrokeShortcut(action, snappedShortcut,
                             QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_V,
                             Qt::LeftButton));

    QVERIFY(!m.keyPressed(Qt::Key_V));
    QCOMPARE(action->m_activateCount, 1);
    QCOMPARE(action->m_mode, normalShortcut);
    QCOMPARE(action->m_interactionState, 1);

    QVERIFY(!m.keyPressed(Qt::Key_Shift));
    QCOMPARE(action->m_switchCount, 1);
    QCOMPARE(action->m_oldShortcut, normalShortcut);
    QCOMPARE(action->m_newShortcut, snappedShortcut);
    QCOMPARE(action->m_mode, snappedShortcut);
    QCOMPARE(action->m_activateCount, 1);
    QCOMPARE(action->m_deactivateCount, 0);
    QCOMPARE(action->m_interactionState, 1);

    QVERIFY(!m.keyReleased(Qt::Key_Shift));
    QCOMPARE(action->m_switchCount, 2);
    QCOMPARE(action->m_oldShortcut, snappedShortcut);
    QCOMPARE(action->m_newShortcut, normalShortcut);
    QCOMPARE(action->m_mode, normalShortcut);
    QCOMPARE(action->m_activateCount, 1);
    QCOMPARE(action->m_deactivateCount, 0);
    QCOMPARE(action->m_interactionState, 1);

    QVERIFY(!m.keyReleased(Qt::Key_V));
    QCOMPARE(action->m_deactivateCount, 1);
    QCOMPARE(action->m_deactivateIndex, normalShortcut);
    QCOMPARE(action->m_interactionState, 0);
}

void KisInputManagerTest::testStrokeShortcutModifierFirstMode()
{
    constexpr int normalShortcut = 0;
    constexpr int snappedShortcut = 1;

    KisShortcutMatcher m;
    m.enterEvent();

    SwitchableTestingAction *action = new SwitchableTestingAction();
    m.addShortcut(
        createStrokeShortcut(action, normalShortcut,
                             QSet<Qt::Key>() << Qt::Key_V,
                             Qt::LeftButton));
    m.addShortcut(
        createStrokeShortcut(action, snappedShortcut,
                             QSet<Qt::Key>() << Qt::Key_Shift << Qt::Key_V,
                             Qt::LeftButton));

    QVERIFY(!m.keyPressed(Qt::Key_Shift));
    QCOMPARE(action->m_activateCount, 0);

    QVERIFY(!m.keyPressed(Qt::Key_V));
    QCOMPARE(action->m_activateCount, 1);
    QCOMPARE(action->m_activateIndex, snappedShortcut);
    QCOMPARE(action->m_mode, snappedShortcut);

    QVERIFY(!m.keyReleased(Qt::Key_V));
    QCOMPARE(action->m_deactivateCount, 1);
    QCOMPARE(action->m_deactivateIndex, snappedShortcut);
    QVERIFY(!m.keyReleased(Qt::Key_Shift));
}

void KisInputManagerTest::testRunReadyShortcutSwitch()
{
    constexpr int normalShortcut = 0;
    constexpr int snappedShortcut = 1;

    KisShortcutMatcher m;
    m.enterEvent();

    SwitchableTestingAction *action = new SwitchableTestingAction();

    m.addShortcut(
        createStrokeShortcut(action, normalShortcut,
                             QSet<Qt::Key>() << Qt::Key_V,
                             Qt::LeftButton));
    m.addShortcut(
        createStrokeShortcut(action, snappedShortcut,
                             QSet<Qt::Key>() << Qt::Key_V,
                             Qt::RightButton));

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(),
                           Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(),
                             Qt::RightButton, Qt::NoButton, Qt::NoModifier);

    QVERIFY(!m.keyPressed(Qt::Key_V));
    QCOMPARE(action->m_activateCount, 1);
    QCOMPARE(action->m_activateIndex, normalShortcut);

    QVERIFY(m.buttonPressed(Qt::RightButton, &pressEvent));
    QCOMPARE(action->m_switchCount, 1);
    QCOMPARE(action->m_oldShortcut, normalShortcut);
    QCOMPARE(action->m_newShortcut, snappedShortcut);
    QCOMPARE(action->m_mode, snappedShortcut);
    QCOMPARE(action->m_activateCount, 1);
    QCOMPARE(action->m_deactivateCount, 0);
    QCOMPARE(action->m_beginCount, 1);
    QCOMPARE(action->m_beginIndex, snappedShortcut);
    QCOMPARE(action->m_interactionState, 1);

    QVERIFY(m.buttonReleased(Qt::RightButton, &releaseEvent));
    QCOMPARE(action->m_endCount, 1);
    QCOMPARE(action->m_switchCount, 2);
    QCOMPARE(action->m_oldShortcut, snappedShortcut);
    QCOMPARE(action->m_newShortcut, normalShortcut);
    QCOMPARE(action->m_mode, normalShortcut);
    QCOMPARE(action->m_activateCount, 1);
    QCOMPARE(action->m_deactivateCount, 0);

    QVERIFY(!m.keyReleased(Qt::Key_V));
    QCOMPARE(action->m_deactivateCount, 1);
}

void KisInputManagerTest::testKeyedStrokeLifecycle()
{
    constexpr int normalShortcut = 0;

    KisShortcutMatcher m;
    m.enterEvent();

    SwitchableTestingAction *action = new SwitchableTestingAction();
    m.addShortcut(
        createStrokeShortcut(action, normalShortcut,
                             QSet<Qt::Key>() << Qt::Key_V,
                             Qt::LeftButton));

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(10, 10),
                          Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(10, 10),
                             Qt::LeftButton, Qt::NoButton, Qt::NoModifier);

    QVERIFY(!m.keyPressed(Qt::Key_V));
    QCOMPARE(action->m_activateCount, 1);

    QVERIFY(m.buttonPressed(Qt::LeftButton, &pressEvent));
    QCOMPARE(action->m_beginCount, 1);
    QCOMPARE(action->m_beginIndex, normalShortcut);
    QCOMPARE(action->m_beginNonNull, true);

    QVERIFY(m.pointerMoved(&moveEvent));
    QCOMPARE(action->m_inputCount, 1);

    QVERIFY(m.buttonReleased(Qt::LeftButton, &releaseEvent));
    QCOMPARE(action->m_endCount, 1);
    QCOMPARE(action->m_endNonNull, true);
    QCOMPARE(action->m_activateCount, 1);
    QCOMPARE(action->m_deactivateCount, 0);
    QCOMPARE(action->m_interactionState, 1);

    QVERIFY(m.buttonPressed(Qt::LeftButton, &pressEvent));
    QCOMPARE(action->m_beginCount, 2);
    QCOMPARE(action->m_activateCount, 1);

    QVERIFY(!m.keyReleased(Qt::Key_V));
    QCOMPARE(action->m_endCount, 1);
    QCOMPARE(action->m_deactivateCount, 0);
    QCOMPARE(action->m_interactionState, 1);

    QVERIFY(m.buttonReleased(Qt::LeftButton, &releaseEvent));
    QCOMPARE(action->m_endCount, 2);
    QCOMPARE(action->m_deactivateCount, 1);
    QCOMPARE(action->m_deactivateIndex, normalShortcut);
    QCOMPARE(action->m_interactionState, 0);

    QVERIFY(!m.buttonPressed(Qt::LeftButton, &pressEvent));
    QCOMPARE(action->m_beginCount, 2);
}

void KisInputManagerTest::testStrokeShortcutCleanup()
{
    constexpr int normalShortcut = 0;

    KisShortcutMatcher m;
    m.enterEvent();

    SwitchableTestingAction *action = new SwitchableTestingAction();
    m.addShortcut(
        createStrokeShortcut(action, normalShortcut,
                             QSet<Qt::Key>() << Qt::Key_V,
                             Qt::LeftButton));

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    QVERIFY(!m.keyPressed(Qt::Key_V));
    QCOMPARE(action->m_activateCount, 1);
    m.leaveEvent();
    QCOMPARE(action->m_deactivateCount, 1);
    QCOMPARE(action->m_interactionState, 0);

    m.enterEvent();
    QCOMPARE(action->m_activateCount, 2);
    QCOMPARE(action->m_interactionState, 1);
    m.lostFocusEvent(QPointF());
    QCOMPARE(action->m_deactivateCount, 2);
    QCOMPARE(action->m_interactionState, 0);

    QVERIFY(!m.keyReleased(Qt::Key_V));
    QVERIFY(!m.keyPressed(Qt::Key_V));
    QCOMPARE(action->m_activateCount, 3);
    QVERIFY(m.buttonPressed(Qt::LeftButton, &pressEvent));
    QCOMPARE(action->m_beginCount, 1);

    m.lostFocusEvent(QPointF(5, 5));
    QCOMPARE(action->m_endCount, 1);
    QCOMPARE(action->m_deactivateCount, 3);
    QCOMPARE(action->m_interactionState, 0);

    QVERIFY(!m.keyReleased(Qt::Key_V));
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
