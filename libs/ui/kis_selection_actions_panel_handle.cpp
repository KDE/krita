/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_selection_actions_panel_handle.h"
#include "kis_icon_utils.h"
#include <qapplication.h>
#include <KoColorDisplayRendererInterface.h>
#include <qevent.h>

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

void KisSelectionActionsPanelHandle::draw(QPainter& painter, const KoColorDisplayRendererInterface *renderInterface)
{
    QRect rect = geometry();

    // Adjust the rect a bit to fill the right side of the bar properly
    painter.fillRect(rect.marginsAdded(QMargins(-3, 4, 1, 4)), renderInterface->systemPaletteForDisplayColorSpace().window().color());

    // Adjusting the icon location a bit to be properly centered

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    d->handle_icon.paint(&painter, QRect(rect.x() + 3, rect.y(), d->size, d->size));
#else
    QImage ic = renderInterface->convertImageToDisplayColorSpace(d->handle_icon.pixmap(d->size, d->size).toImage());
    QRect target = QRect(QPoint(0, 0), ic.deviceIndependentSize().toSize());
    target.moveCenter(rect.center() + QPoint(3, 0));
    painter.drawImage(target, ic);
#endif
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
