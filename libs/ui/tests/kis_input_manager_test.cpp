/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
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

        m_inputEventCount++;
    }
    void end(QEvent *event) override {
        KIS_ASSERT(m_state == Running);
        m_state = m_stateBeforeRunning;
        m_endedIndexes.append(m_beginIndex);

        m_ended = true; m_endNonNull = event;

        m_inputEventCount++;
    }
    void inputEvent(QEvent* event) override {
        KIS_ASSERT(m_state == Running);

        Q_UNUSED(event);
        m_gotInput = true;

        m_inputEventCount++;
    }

    void reset() {
        m_beginIndex = -1;
        m_begunIndexes.clear();
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
    int m_inputEventCount = 0;
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
        //s->setDisableOnTouchPainting(true);
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
    const int ignoreActionEventCount = -1;
    const int ignorePaintEventCount = -1;

    QTest::addRow("clean-1p-drag") << "touchCleanDrag" << 1 << 100 << dragOffset << touchPaintingOff << QList<int>{15} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-2p-drag") << "touchCleanDrag" << 2 << 100 << dragOffset << touchPaintingOff << QList<int>{16} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-3p-drag") << "touchCleanDrag" << 3 << 100 << dragOffset << touchPaintingOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    QTest::addRow("clean-1p-drag-paint") << "touchCleanDrag" << 1 << 100 << dragOffset << touchPaintingOn << QList<int>{} << QList<int>{30} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-2p-drag-paint") << "touchCleanDrag" << 2 << 100 << dragOffset << touchPaintingOn << QList<int>{16} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-3p-drag-paint") << "touchCleanDrag" << 3 << 100 << dragOffset << touchPaintingOn << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    // painting skips only the first two events
    QTest::addRow("clean-1p-drag-paint-delayed-start") << "touchCleanDrag" << 1 << 100 << QPointF(1.0, 0) << touchPaintingOn << QList<int>{} << QList<int>{30} << ignoreActionEventCount << 98;
    // action has higher threshold, so it skips the first 17 events
    QTest::addRow("clean-1p-drag-delayed-start") << "touchCleanDrag" << 1 << 100 << QPointF(1.0, 0) << touchPaintingOff << QList<int>{15} << QList<int>{} << 83 << ignorePaintEventCount;

    QTest::addRow("clean-1p-tap") << "touchCleanDrag" << 1 << 10 << tapOffset << touchPaintingOff << QList<int>{20} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-2p-tap") << "touchCleanDrag" << 2 << 10 << tapOffset << touchPaintingOff << QList<int>{21} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-3p-tap") << "touchCleanDrag" << 3 << 10 << tapOffset << touchPaintingOff << QList<int>{22} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    // touch painting stroke has a lower drag threshold, so we should use smaller offset value for it
    QTest::addRow("clean-1p-tap-paint") << "touchCleanDrag" << 1 << 10 << smallTapOffset << touchPaintingOn << QList<int>{} << QList<int>{31} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-2p-tap-paint") << "touchCleanDrag" << 2 << 10 << tapOffset << touchPaintingOn << QList<int>{21} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-3p-tap-paint") << "touchCleanDrag" << 3 << 10 << tapOffset << touchPaintingOn << QList<int>{22} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    // 16px is the threshold for distinguishing taps and drags (touch-begin + 16 steps + touch-end)
    QTest::addRow("clean-1p-tap-upper-bound") << "touchCleanDrag" << 1 << 18 << QPointF(1, 0) << touchPaintingOff << QList<int>{20} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("clean-1p-drag-lower-bound") << "touchCleanDrag" << 1 << 19 << QPointF(1, 0) << touchPaintingOff << QList<int>{15} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    QTest::addRow("dirty-start-2p-drag") << "touchDragDirtyStart" << 2 << 100 << dirtyDragOffset << touchPaintingOff << QList<int>{16} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("dirty-start-3p-drag") << "touchDragDirtyStart" << 3 << 100 << dirtyDragOffset << touchPaintingOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    QTest::addRow("dirty-end-2p-drag") << "touchDragDirtyEnd" << 2 << 100 << dirtyDragOffset << touchPaintingOff << QList<int>{16} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("dirty-end-3p-drag") << "touchDragDirtyEnd" << 3 << 100 << dirtyDragOffset << touchPaintingOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    QTest::addRow("dirty-end-2p-tap") << "touchDragDirtyEnd" << 2 << 10 << tapOffset << touchPaintingOff << QList<int>{21} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
    QTest::addRow("dirty-end-3p-tap") << "touchDragDirtyEnd" << 3 << 10 << tapOffset << touchPaintingOff << QList<int>{22} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    // the current implementation is expected to switch to a "more-fingers-rich" shortcut immetiately
    QTest::addRow("clean-2p-then-3p-drag") << "touchDragTwoThenThree" << 2 << 100 << dragOffset << touchPaintingOff << QList<int>{16, 17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    // the current implementation is expected to stay on the "finger-richest" shortcut and,
    // not to switch to a "finger-poorer" shortcut
    QTest::addRow("clean-3p-then-2p-drag") << "touchDragThreeThenTwo" << 3 << 100 << dragOffset << touchPaintingOff << QList<int>{17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;

    // the 3-point gesture should be run twice during the same run
    QTest::addRow("clean-3p-then-2p-then-3p-drag") << "touchDragThreeThenTwoThenThree" << 3 << 100 << dragOffset << touchPaintingOff << QList<int>{17, 17} << QList<int>{} << ignoreActionEventCount << ignorePaintEventCount;
}

void KisInputManagerTest::testTouchMoves()
{
    std::unique_ptr<TestingAction> paintAction(new TestingAction("paint-action"));
    std::unique_ptr<TestingAction> a(new TestingAction("touch-action"));

    KisShortcutMatcher m;
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
    QFETCH(QList<int>, triggeredTouchShortcuts);
    QFETCH(QList<int>, triggeredPaintShortcuts);
    QFETCH(int, actionEventsCount);
    QFETCH(int, paintEventsCount);

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QTouchEvent e(QEvent::TouchEnd, &device, Qt::NoModifier, calcTouchPointStates(touchPoints), touchPoints);
#else
            QTouchEvent e(QEvent::TouchEnd, &device, Qt::NoModifier, touchPoints);
#endif
            m.touchEndEvent(&e);
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

    if (actionEventsCount >= 0) {
        QCOMPARE(a->m_inputEventCount, actionEventsCount);
    }

    if (paintEventsCount >= 0) {
        QCOMPARE(paintAction->m_inputEventCount, paintEventsCount);
    }
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
