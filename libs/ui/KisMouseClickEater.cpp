/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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
