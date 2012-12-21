/* This file is part of the KDE libraries
   Copyright (C) 2007 Aurélien Gâteau <agateau@kde.org>
   Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoGroupButton.h"

// Qt
#include <QAction>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QToolButton>

// KDE
#include <KLocale>

class KoGroupButton::Private
{
public:
    Private(const GroupPosition position) : groupPosition(position)
    {}
    GroupPosition groupPosition;
    QWidget* dialogParent;
};

KoGroupButton::KoGroupButton(GroupPosition position, QWidget* parent)
: d(new Private(position))
{
    d->dialogParent = parent;

    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

KoGroupButton::~KoGroupButton()
{
    delete d;
}

void KoGroupButton::setGroupPosition(KoGroupButton::GroupPosition groupPosition)
{
    d->groupPosition = groupPosition;
}

KoGroupButton::GroupPosition KoGroupButton::groupPosition() const
{
    return d->groupPosition;
}

void KoGroupButton::paintEvent(QPaintEvent* event)
{
    if (groupPosition() == NoGroup) {
        QToolButton::paintEvent(event);
        return;
    }
    QStylePainter painter(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QStyleOptionToolButton panelOpt = opt;

    // Panel
    QRect& panelRect = panelOpt.rect;
    switch (groupPosition()) {
    case GroupLeft:
        panelRect.setWidth(panelRect.width() * 2);
        break;
    case GroupCenter:
        panelRect.setLeft(panelRect.left() - panelRect.width());
        panelRect.setWidth(panelRect.width() * 3);
        break;
    case GroupRight:
        panelRect.setLeft(panelRect.left() - panelRect.width());
        break;
    case NoGroup:
        Q_ASSERT(0);
    }
    painter.drawPrimitive(QStyle::PE_PanelButtonTool, panelOpt);

    // Separator
    const int y1 = opt.rect.top() + 6;
    const int y2 = opt.rect.bottom() - 6;
    if (d->groupPosition & GroupRight) {
        const int x = opt.rect.left();
        painter.setPen(opt.palette.color(QPalette::Light));
        painter.drawLine(x, y1, x, y2);
    }
    if (d->groupPosition & GroupLeft) {
        const int x = opt.rect.right();
        painter.setPen(opt.palette.color(QPalette::Mid));
        painter.drawLine(x, y1, x, y2);
    }

    // Text
    painter.drawControl(QStyle::CE_ToolButtonLabel, opt);

    // Filtering message on tooltip text for CJK to remove accelerators.
    // Quoting ktoolbar.cpp:
    // """
    // CJK languages use more verbose accelerator marker: they add a Latin
    // letter in parenthesis, and put accelerator on that. Hence, the default
    // removal of ampersand only may not be enough there, instead the whole
    // parenthesis construct should be removed. Provide these filtering i18n
    // messages so that translators can use Transcript for custom removal.
    // """
    if (!actions().isEmpty()) {
        QAction* action = actions().first();
        setToolTip(i18nc("@info:tooltip of custom triple button", "%1", action->toolTip()));
    }
}

#include <KoGroupButton.moc>
