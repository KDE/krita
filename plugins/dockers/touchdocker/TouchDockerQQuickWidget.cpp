/*
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TouchDockerQQuickWidget.h"

#include <QInputEvent>

#include "kis_debug.h"

// Block mouse events that happens within x milliseconds from the last touch
// event. 100 ms seems to be a reasonable time?
constexpr ulong MOUSE_EVENT_BLOCK_TIME = 100;

TouchDockerQQuickWidget::~TouchDockerQQuickWidget()
{
}

bool TouchDockerQQuickWidget::event(QEvent *event)
{
    bool blocked = false;

    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        // Record the timestamp of the last touch event received so we can
        // block mouse events initiated within a certain time.
        m_lastTouchEventTimeMills = dynamic_cast<QInputEvent *>(event)->timestamp();
        break;
    case QEvent::MouseButtonPress:
        if (dynamic_cast<QInputEvent *>(event)->timestamp() - m_lastTouchEventTimeMills
                < MOUSE_EVENT_BLOCK_TIME)
        {
            dbgInput << "TouchDockerQQuickWidget blocking mouse press";
            blocked = true;
            // Start blocking mouse events until released.
            m_shouldBlockUntilMouseUp = true;
            // Also block the next double-click, if there is one.
            m_shouldBlockNextDblClick = true;
        } else {
            dbgInput << "TouchDockerQQuickWidget NOT blocking mouse press";
            // We don't need to block mouse events.
            m_shouldBlockUntilMouseUp = false;
            // We shouldn't block the next double-click either.
            m_shouldBlockNextDblClick = false;
        }
        break;
    case QEvent::MouseButtonDblClick:
        if (m_shouldBlockNextDblClick) {
            dbgInput << "TouchDockerQQuickWidget blocking mouse dblclick";
            blocked = true;
            // Start blocking mouse events until released.
            m_shouldBlockUntilMouseUp = true;
            // Clear this flag as we already blocked the double-click that we
            // expected.
            m_shouldBlockNextDblClick = false;
        } else {
            dbgInput << "TouchDockerQQuickWidget NOT blocking mouse dblclick";
            // We don't need to block mouse events.
            m_shouldBlockUntilMouseUp = false;
        }
        break;
    case QEvent::MouseMove:
        if (m_shouldBlockUntilMouseUp) {
            dbgInput << "TouchDockerQQuickWidget blocking mouse move";
            blocked = true;
        } else {
            dbgInput << "TouchDockerQQuickWidget NOT blocking mouse move";
        }
        break;
    case QEvent::MouseButtonRelease:
        if (m_shouldBlockUntilMouseUp) {
            dbgInput << "TouchDockerQQuickWidget blocking mouse release";
            blocked = true;
        } else {
            dbgInput << "TouchDockerQQuickWidget NOT blocking mouse release";
        }
        m_shouldBlockUntilMouseUp = false;
        break;
    default:
        break;
    }

    if (blocked) {
        return true;
    }
    return QQuickWidget::event(event);
}

