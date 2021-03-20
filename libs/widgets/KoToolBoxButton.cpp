/*
 * SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
            plainToolTip :
            i18nc("@info:tooltip %2 is shortcut", "%1 (%2)", plainToolTip,
                shortcut.toString(QKeySequence::NativeText));

    setToolTip(toolTip);
}
