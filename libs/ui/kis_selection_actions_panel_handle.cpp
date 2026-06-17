/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_selection_actions_panel_handle.h"
#include "kis_icon_utils.h"
#include <qapplication.h>
#include <qevent.h>

struct KisSelectionActionsPanelHandle::Private
{
    Private(int size) { this->size = size;};
    QCursor default_cursor;
    QCursor held_cursor;
    QIcon handle_icon;
    int size;
    Orientation orientation;
};

KisSelectionActionsPanelHandle::KisSelectionActionsPanelHandle(int size, QWidget * parent) :
    QWidget(parent),
    d(new Private(size))
{
    d->default_cursor = Qt::OpenHandCursor;
    d->held_cursor = Qt::ClosedHandCursor;
    d->handle_icon = KisIconUtils::loadIcon("drag-handle");
    this->setCursor(d->default_cursor);
    setFixedSize(size, size);
    setAttribute(Qt::WA_AcceptTouchEvents);
}

KisSelectionActionsPanelHandle::~KisSelectionActionsPanelHandle()
{
}

void KisSelectionActionsPanelHandle::set_held(bool held)
{
    if(held)
    {
        this->setCursor(d->held_cursor);
    }
    else
    {
        this->setCursor(d->default_cursor);
    }
}

void KisSelectionActionsPanelHandle::draw(QPainter& painter)
{
    QRect rect = geometry();
    QPoint offset;

    // Adjust the rect a bit to fill the right side of the bar properly
    if (d->orientation == Orientation::Horizontal) {
        painter.fillRect(rect.marginsAdded(QMargins(-3, 4, 1, 4)), qApp->palette().window().color());
        offset = QPoint(3, 0);
    } else {
        painter.fillRect(rect.marginsAdded(QMargins(4, -3, 4, 1)), qApp->palette().window().color());
        offset = QPoint(0, 3);
    }

    // Adjusting the icon location a bit to be properly centered
    d->handle_icon.paint(&painter, QRect(QPoint(rect.topLeft() + offset), QSize(d->size, d->size)));
}

void KisSelectionActionsPanelHandle::contextMenuEvent(QContextMenuEvent *event)
{
    Q_EMIT customContextMenuRequested(mapToGlobal(event->pos()));
    event->accept();
}

void KisSelectionActionsPanelHandle::mousePressEvent(QMouseEvent *event)
{
    //Do not propagate the rmb event, to prevent other tool context menus from appearing
    if (event->button() == Qt::RightButton) {
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void KisSelectionActionsPanelHandle::setOrientation(Orientation orientation)
{
    d->orientation = orientation;
}

void KisSelectionActionsPanelHandle::themeChanged()
{
    d->handle_icon = KisIconUtils::loadIcon(d->handle_icon.name());
}
