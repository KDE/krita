/*  This file is part of the Calligra project, made within the KDE community.
 *
 * Copyright 2012  Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "KoPageNavigatorButton_p.h"

// KDE
#include <KIcon>
#include <KDebug>
// Qt
#include <QAction>


KoPageNavigatorButton::KoPageNavigatorButton(const char *iconName, QWidget *parent)
  : QToolButton(parent)
  , m_action(0)
{
    setIcon(KIcon(QLatin1String(iconName)));
    setFocusPolicy(Qt::NoFocus);
    setAutoRaise(true);
}

void KoPageNavigatorButton::setAction(QAction *action)
{
    if (! action) {
        kWarning()<<"Attempt to set a null action";
        return;
    }
    if (m_action) {
        kWarning()<<"Attempt to set a second action";
        return;
    }

    m_action = action;

    connect(this, SIGNAL(clicked(bool)), SLOT(onClicked()));
    connect(m_action, SIGNAL(changed()), SLOT(onActionChanged()));

    onActionChanged();
}


void KoPageNavigatorButton::onActionChanged()
{
    setEnabled(m_action->isEnabled());

    // always updating the tooltip is a workaround around that KPrView only updates
    // the action texts with the "slide" variants later, they are not already
    // set in KoPAView::initActions()
    setToolTip(m_action->toolTip());
}

void KoPageNavigatorButton::onClicked()
{
    m_action->activate(QAction::Trigger);
}
