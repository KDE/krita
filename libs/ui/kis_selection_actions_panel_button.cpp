/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_selection_actions_panel_button.h"

#include "kis_icon_utils.h"
#include <qapplication.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpainterpath.h>

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
}

KisSelectionActionsPanelButton::~KisSelectionActionsPanelButton()
{

}

void KisSelectionActionsPanelButton::draw(QPainter &painter)
{
    QRect rect = geometry();
    //Draw an outline when the button is pressed
    if(this->isDown()) {
        QColor bgColor = qApp->palette().window().color();
        //Invert the background color
        bgColor.setRed(255 - bgColor.red());
        bgColor.setGreen(255 - bgColor.green());
        bgColor.setBlue(255 - bgColor.blue());

        QPainterPath path;
        path.addRoundedRect(rect, 3, 3);
        QPen pen = bgColor;
        pen.setWidth(2);

        painter.setPen(pen);
        painter.drawPath(path)  ;
    }

    QPixmap map = this->icon().pixmap(iconSize());
    painter.drawPixmap(rect.x() + ICON_SIZE_OFFSET / 2.0 , rect.y() + ICON_SIZE_OFFSET / 2.0, map);
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (((QTouchEvent *)e)->points().count() == 1) {
            this->animateClick();
            e->accept();
            return true;
        }
#else
        if (((QTouchEvent *)e)->touchPoints().count() == 1) {
            this->animateClick();
            e->accept();
            return true;
        }
#endif
    }
    return QAbstractButton::event(e);
}
