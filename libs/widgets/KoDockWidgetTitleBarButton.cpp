/* This file is part of the KDE project
   Copyright (c) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoDockWidgetTitleBarButton.h"

#include <WidgetsDebug.h>

#include <QAbstractButton>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>

class Q_DECL_HIDDEN KoDockWidgetTitleBarButton::Private
{
public:
    Private() : styleSize(0, 0), iconSize(0) {}
    QSize styleSize;
    int iconSize;
};

KoDockWidgetTitleBarButton::KoDockWidgetTitleBarButton(QWidget *parent)
        : QAbstractButton(parent), d(new Private())
{
    setFocusPolicy(Qt::NoFocus);
}

KoDockWidgetTitleBarButton::~KoDockWidgetTitleBarButton()
{
    delete d;
}

QSize KoDockWidgetTitleBarButton::sizeHint() const
{
    ensurePolished();

    const int margin = style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, 0, this);
    if (icon().isNull()) {
        return QSize(18, 18);
    }

    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
    if (iconSize != d->iconSize) {
        const_cast<KoDockWidgetTitleBarButton*>(this)->d->iconSize = iconSize;
        const QPixmap pm = icon().pixmap(iconSize);
        const_cast<KoDockWidgetTitleBarButton*>(this)->d->styleSize = QSize(pm.width() + margin, pm.height() + margin);
    }
    return d->styleSize;
}

QSize KoDockWidgetTitleBarButton::minimumSizeHint() const
{
    return sizeHint();
}

// redraw the button when the mouse enters or leaves it
void KoDockWidgetTitleBarButton::enterEvent(QEvent *event)
{
    if (isEnabled())
        update();
    QAbstractButton::enterEvent(event);
}

void KoDockWidgetTitleBarButton::leaveEvent(QEvent *event)
{
    if (isEnabled())
        update();
    QAbstractButton::leaveEvent(event);
}

void KoDockWidgetTitleBarButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QStyleOptionToolButton opt;
    opt.initFrom(this);
    opt.state |= QStyle::State_AutoRaise;

    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

    opt.icon = icon();
    opt.subControls = 0;
    opt.activeSubControls = 0;
    opt.features = QStyleOptionToolButton::None;
    opt.arrowType = Qt::NoArrow;
    int size = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
    opt.iconSize = QSize(size, size);
    style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
}
