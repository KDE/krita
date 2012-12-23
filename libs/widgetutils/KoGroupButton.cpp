/* This file is part of the KDE libraries
   Copyright (C) 2007 Aurélien Gâteau <agateau@kde.org>
   Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
   Copyright (C) 2012 Jarosław Staniek <staniek@kde.org>

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
    Private(KoGroupButton *qq, const GroupPosition position) : groupPosition(position)
    {
        // Make the policy closer to QPushButton's default but horizontal shouldn't be Fixed,
        // otherwise spacing gets broken
        qq->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    GroupPosition groupPosition;
};

KoGroupButton::KoGroupButton(GroupPosition position, QWidget* parent)
 : QToolButton(parent), d(new Private(this, position))
{
}

KoGroupButton::KoGroupButton(QWidget* parent)
 : QToolButton(parent), d(new Private(this, NoGroup))
{
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
    if (!isChecked() && !isDown() && !(panelOpt.state & QStyle::State_MouseOver)) {
        // Use 'pushed' appearance for all buttons but button that are not really pushed
        // have less contrast and is toned down.
        panelOpt.state |= (QStyle::State_On | QStyle::State_Sunken);
        QPalette panelPal(panelOpt.palette);
        QColor c;
        c = panelPal.color(QPalette::Button);
        c.setAlpha(50);
        panelPal.setColor(QPalette::Button, c);
        c = panelPal.color(QPalette::Window);
        c.setAlpha(50);
        panelPal.setColor(QPalette::Window, c);
        panelOpt.palette = panelPal;
        painter.setOpacity(0.5);
    }
    painter.drawPrimitive(QStyle::PE_PanelButtonTool, panelOpt);
    painter.setOpacity(1.0);

    // Separator
    //! @todo make specific fixes for styles such as Plastique, Cleanlooks if there's practical no alernative
    const int y1 = opt.rect.top() + 1;
    const int y2 = opt.rect.bottom() - 1;
    painter.setOpacity(0.4);
    if (d->groupPosition != GroupRight) {
        const int x = opt.rect.right();
        painter.setPen(opt.palette.color(QPalette::Dark));
        painter.drawLine(x, y1, x, y2);
    }
    painter.setOpacity(1.0);

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
