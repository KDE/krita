/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoToolDocker.h"
#include <QStackedWidget>

#include <klocale.h>

KoToolDocker::KoToolDocker()
    : QDockWidget(i18n("Tool Options"))
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_stack = new QStackedWidget(this);
    setWidget(m_stack);
}

void KoToolDocker::setOptionWidget(QWidget * widget)
{
    if (widget && m_stack->indexOf(widget) == -1) {
        m_stack->addWidget(widget);
    }
    m_stack->setCurrentWidget(widget);
}
