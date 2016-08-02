/*
 * Copyright (c) 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "KoToolBoxButton_p.h"

#include <KoToolManager.h>
#include <QIcon>
#include <QPalette>
#include <QApplication>
#include <klocalizedstring.h>
#include <QKeySequence>
#include <KoIcon.h>

KoToolBoxButton::KoToolBoxButton(KoToolAction *toolAction, QWidget *parent)
    : QToolButton(parent)
    , m_toolAction(toolAction)
{
    setObjectName(m_toolAction->id());
    // ensure same L&F
    setCheckable(true);
    setAutoRaise(true);
    setIcon(kisIcon(m_toolAction->iconName()));

    setDataFromToolAction();

    connect(this, SIGNAL(clicked(bool)), m_toolAction, SLOT(trigger()));
    connect(m_toolAction, SIGNAL(changed()), SLOT(setDataFromToolAction()));
}

void KoToolBoxButton::setHighlightColor()
{
    QPalette p = qApp->palette();
    if (isChecked()) {
        QPalette palette_highlight(p);
        QColor c = p.color(QPalette::Highlight);
        palette_highlight.setColor(QPalette::Button, c);
        setPalette(palette_highlight);
    }
    else {
        setPalette(p);
    }
}

void KoToolBoxButton::setDataFromToolAction()
{
    const QString plainToolTip = m_toolAction->toolTip();
    const QKeySequence shortcut = m_toolAction->shortcut();
    const QString toolTip =
        shortcut.isEmpty() ?
            i18nc("@info:tooltip", "%1", plainToolTip) :
            i18nc("@info:tooltip %2 is shortcut", "%1 (%2)", plainToolTip,
                shortcut.toString());

    setToolTip(toolTip);
}
