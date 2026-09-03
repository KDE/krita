/*
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kritaui_export.h>

#include <kis_debug.h>
#include "input/kis_tablet_debugger.h"

class KisToolInvocationAction;


class KRITAUI_EXPORT KisInputEventsEater
{
public:
    enum EventBlockingReason {
        NotBlocked = 0x0,
        BlockedByTabletProximity = 0x01,
        BlockedByTabletHover = 0x02,
        BlockedByTabletPress = 0x04,
        BlockedByTouchPress = 0x08,
        BlockedByNextPressSuppression = 0x10,
        BlockedByButtonsWorkaround = 0x20,
        BlockedBySynthetic = 0x40
    };
    Q_DECLARE_FLAGS(EventBlockingReasons, EventBlockingReason)
public:
    KisInputEventsEater();

    bool eventFilter(QObject *target, QEvent *event);

    void notifyTabletEnterProximity();
    void notifyTabletLeaveProximity();

    // On Windows, we sometimes receive mouse events very late, so watch & wait.
    void eatOneMousePress();

    template<class Event>
    static void debugEvent(QEvent *event, EventBlockingReasons reasons = NotBlocked)
    {
        if (!KisTabletDebugger::instance()->debugEnabled())
            return;

        QString reasonsString;

        if (reasons.testFlag(BlockedByTabletProximity)) {
            reasonsString += "Prx";
        }

        if (reasons.testFlag(BlockedByTabletHover)) {
            reasonsString += "Hov";
        }

        if (reasons.testFlag(BlockedByTabletPress)) {
            reasonsString += "Prs";
        }

        if (reasons.testFlag(BlockedByNextPressSuppression)) {
            reasonsString += "Nxt";
        }

        if (reasons.testFlag(BlockedByTouchPress)) {
            reasonsString += "Tch";
        }

        if (reasons.testFlag(BlockedBySynthetic)) {
            reasonsString += "Syn";
        }

        QString msg1 = QString("[%1] ").arg(reasonsString, 15);
        Event *specificEvent = static_cast<Event *>(event);
        dbgTablet << KisTabletDebugger::instance()->eventToString(*specificEvent, msg1);
    }

private:

    bool eatOneMousePressEvent{false}; // Eat a single mouse press event
    bool activateSecondaryButtonsWorkaround{false}; // Use mouse events for right- and middle-clicks

    bool tabletIsInProximity{false};
    bool tabletIsHovering{false};
    bool tabletIsPressed{false};
    bool touchIsActive{false};

private:
    void debugEaterStateTransition(const QLatin1String &stateName,
                                   bool newValue,
                                   QEvent::Type eventType,
                                   const QLatin1String &comment = QLatin1String());
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisInputEventsEater::EventBlockingReasons)