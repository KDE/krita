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
#include <QGridLayout>
#include <QLabel>
#include <QWidget>

#include <klocale.h>
#include <kdebug.h>

#include <KoToolManager.h>

KoToolDocker::KoToolDocker()
    : QDockWidget(i18n("Tool Options"))
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_stack = new QStackedWidget(this);
    setWidget(m_stack);

    m_label = new QWidget();
    QGridLayout *lay = new QGridLayout(m_label);
    QLabel *label = new QLabel(i18n("No options for current tool"), m_label);
    lay->addWidget(label, 0, 0, Qt::AlignTop | Qt::AlignHCenter);

    m_stack->addWidget( m_label );
    m_stack->setCurrentWidget( m_label );
}

KoToolDocker::~KoToolDocker()
{
    // Remove the tool option widgets from our layout: we don't own them,
    // we are not going to delete them. 
    // XXX: The Right (tm) solution here is to use QPointer (bsar, see http://doc.trolltech.com/qq/qq14-guardedpointers.html)
    while (m_stack->count() > 0) {
        QWidget * w = m_stack->widget(0);
        kDebug() << "Stack count: " << m_stack->count() << ", widget: " << w << endl;
        m_stack->removeWidget(w);
        w->setParent(0);
    }
    delete m_label;

    // Hack to avoid crashes due to dangling pointers
    KoToolManager::instance()->unsetToolOptionDocker(this);
}

void KoToolDocker::setOptionWidget(QWidget * widget)
{
    kDebug() << "Setting to " << widget << endl;
    if (widget ) {
        if ( m_stack->indexOf(widget) == -1) {
                m_stack->addWidget(widget);
        }
        m_stack->setCurrentWidget(widget);
    }
    else {
        kDebug() << "index of label " << m_stack->indexOf(m_label) << endl;
        m_stack->setCurrentWidget( m_label );
    }
}
