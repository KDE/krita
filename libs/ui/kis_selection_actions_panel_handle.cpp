/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_selection_actions_panel_handle.h"
#include "kis_icon_utils.h"

struct KisSelectionActionsPanelHandle::Private
{
    Private(int size) { this->size = size;};
    QCursor default_cursor;
    QCursor held_cursor;
    QIcon handle_icon;
    int size;
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
    QPixmap map = d->handle_icon.pixmap(d->size);

    QRect rect = geometry();
    //Adjust the rct a bit to fill the right side of the bar properly
    rect.setY(rect.y()  - 4);
    rect.setHeight(rect.height() + 4);
    rect.setX(rect.x()  + 3);
    rect.setWidth(rect.width() + 1);

    QColor bgColor = Qt::darkGray;
    painter.fillRect(rect, bgColor);

    painter.fillRect(rect, bgColor);
    // Adjusting the icon location a bit to be properly centered
    painter.drawPixmap(geometry().x() + 3, geometry().y(), map);
}
