/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMouseClickEater.h"

#include <QMouseEvent>
#include "kis_debug.h"

KisMouseClickEater::KisMouseClickEater(Qt::MouseButtons buttons,
                                       int clicksToEat,
                                       QObject *parent)
    : QObject(parent),
      m_buttons(buttons),
      m_clicksToEat(clicksToEat)
{
    reset();
}

KisMouseClickEater::~KisMouseClickEater()
{
}

void KisMouseClickEater::reset()
{
    m_clicksHappened = 0;
    m_timeSinceReset.start();
    m_blockTimedRelease = false;
}

bool KisMouseClickEater::eventFilter(QObject *watched, QEvent *event)
{
#ifdef Q_OS_WIN
    const int tabletMouseEventsFlowDelay = 500;
#else
    const int tabletMouseEventsFlowDelay = 100;
#endif

    if (event->type() == QEvent::TabletMove) {
        m_blockTimedRelease = true;
    }

    if (!m_blockTimedRelease &&
        m_timeSinceReset.elapsed() > tabletMouseEventsFlowDelay) {

        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease) {

        QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
        if (mevent->button() & m_buttons) {
            if (m_clicksHappened >= m_clicksToEat) {
                return false;
            }

            if (event->type() == QEvent::MouseButtonRelease) {
                m_clicksHappened++;
            }

            return true;
        }
    }

    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
        if (mevent->buttons() & m_buttons) {
            return m_clicksHappened < m_clicksToEat;
        }
    }

    return QObject::eventFilter(watched, event);
}
