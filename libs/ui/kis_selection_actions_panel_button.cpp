/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_selection_actions_panel_button.h"

#include "KisLongPressEventFilter.h"
#include "kis_icon_utils.h"
#include <qapplication.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <KoColorDisplayRendererInterface.h>

static constexpr int ICON_SIZE_OFFSET = 6;

KisSelectionActionsPanelButton::KisSelectionActionsPanelButton(const QString& iconName, const QString &tooltip, int size, QWidget *parent)
    : QAbstractButton(parent)
{
    setIcon(KisIconUtils::loadIcon(iconName));
    setFixedSize(size, size);
    setToolTip(tooltip);
    setCursor(Qt::PointingHandCursor);
    setIconSize(QSize(size - ICON_SIZE_OFFSET, size - ICON_SIZE_OFFSET));
    setAttribute(Qt::WA_AcceptTouchEvents);
    setProperty(KisLongPressEventFilter::ENABLED_PROPERTY, true);
}

KisSelectionActionsPanelButton::~KisSelectionActionsPanelButton()
{

}

void KisSelectionActionsPanelButton::draw(QPainter &painter, const KoColorDisplayRendererInterface *displayRendererInterface)
{
    QRect rect = geometry();
    //Draw an outline when the button is pressed
    if(this->isDown()) {
        QPainterPath path;
        path.addRoundedRect(rect, 3, 3);
        QPen pen = displayRendererInterface->systemPaletteForDisplayColorSpace().highlight().color();
        pen.setWidth(2);

        painter.setPen(pen);
        painter.drawPath(path)  ;
    }

    int padding = ICON_SIZE_OFFSET / 2;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    icon().paint(&painter, rect.marginsRemoved(QMargins(padding, padding, padding, padding)));
#else
    QImage ic = displayRendererInterface->convertImageToDisplayColorSpace(icon().pixmap(rect.width()-(padding*2), rect.height()-(padding*2)).toImage());
    QRect target = QRect(QPoint(0, 0), ic.deviceIndependentSize().toSize());
    target.moveCenter(rect.center());
    painter.drawImage(target.intersected(rect.marginsRemoved(QMargins(padding, padding, padding, padding))), ic);
#endif

}


//Dummy paint event, so that this class isn't treated like an abstract class
void KisSelectionActionsPanelButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
}

// For whatever reason the children of the canvas don't interpret tabletPress/Release events as mouse press/release events
// So we have to do this manually
void KisSelectionActionsPanelButton::tabletEvent(QTabletEvent *e)
{
    if (e->type() == QEvent::TabletPress) {
        QMouseEvent mouseEvent(QEvent::MouseButtonPress,
                               e->posF(),
                               e->globalPosF(),
                               e->button(),
                               e->buttons(),
                               e->modifiers());
        QAbstractButton::mousePressEvent(&mouseEvent);
        e->accept();
    } else if (e->type() == QEvent::TabletRelease) {
        QMouseEvent mouseEvent(QEvent::MouseButtonRelease,
                               e->posF(),
                               e->globalPosF(),
                               e->button(),
                               e->buttons(),
                               e->modifiers());
        QAbstractButton::mouseReleaseEvent(&mouseEvent);
        e->accept();
    }
}

bool KisSelectionActionsPanelButton::event(QEvent *e)
{
    //Manually handle the button being pressed on touch
    if (e->type() == QEvent::TouchBegin) {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(e);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        int pointsCount = touchEvent->points().count();
#else
        int pointsCount = touchEvent->touchPoints().count();
#endif
        if (pointsCount == 1) {
            this->animateClick();
            e->accept();
            return true;
        }
    }
    return QAbstractButton::event(e);
}

void KisSelectionActionsPanelButton::contextMenuEvent(QContextMenuEvent *event)
{
    Q_EMIT customContextMenuRequested(mapToGlobal(event->pos()));
    event->accept();
}

void KisSelectionActionsPanelButton::mousePressEvent(QMouseEvent *event)
{
    //Do not propagate the rmb event, to prevent other tool context menus from appearing
    if (event->button() == Qt::RightButton) {
        event->accept();
    } else {
        QAbstractButton::mousePressEvent(event);
    }
}
