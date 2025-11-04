/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2021 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 * SPDX-FileCopyrightText: 2021 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPopupWidgetAction.h"

#include <QCursor>
#include <QMenu>
#include <QTouchEvent>

#include <klocalizedstring.h>

#include <kis_favorite_resource_manager.h>
#include <kis_canvas2.h>
#include "kis_tool_proxy.h"
#include "kis_popup_palette.h"
#include "kis_input_manager.h"

struct SinglePressEventEater : public QObject
{
    bool eventFilter(QObject *, QEvent *event) override {
        if (hungry && event->type() == QEvent::MouseButtonPress) {
            hungry = false;
            return true;
        }

        return false;
    }

private:
    bool hungry = true;
};


//=================================================================

KisPopupWidgetAction::KisPopupWidgetAction()
    : KisAbstractInputAction("Show Popup Widget")
{
    setName(i18n("Show Popup Widget"));
    setDescription(i18n("Show the current tool's popup widget."));
}

KisPopupWidgetAction::~KisPopupWidgetAction()
{
}

void KisPopupWidgetAction::end(QEvent *event)
{
    if (QMenu *popupMenu = inputManager()->toolProxy()->popupActionsMenu()) { // Handle popup menus...
        QEvent::Type requestingEventType = event ? event->type() : QEvent::None;

        // Touch events don't update the cursor position.
        QPointF touchPos;
        if (requestingEventType == QEvent::TouchBegin) {
            const QTouchEvent *touchEvent = static_cast<const QTouchEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const QList<QEventPoint> &touchPoints = event->points();
#else
            const QList<QTouchEvent::TouchPoint> &touchPoints = touchEvent->touchPoints();
#endif
            if (touchPoints.isEmpty()) {
                // Getting zero touch points can happen on Android when pressing
                // on the screen with an entire palm. Punt to using the cursor
                // position after all.
                requestingEventType = QEvent::None;
            } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                touchPos = touchPoints.constFirst().globalPosition();
#else
                touchPos = touchPoints.constFirst().screenPos();
#endif
            }
        }

        /**
         * Opening a menu changes the focus of the windows, so we should not open it
         * inside the filtering loop. Just raise it using the timer.
         */
        QTimer::singleShot(0, this, [popupMenu, requestingEventType, touchPos](){
            if (popupMenu) {
                QPoint popupPos;
                QScopedPointer<SinglePressEventEater> eventEater;

                if (requestingEventType == QEvent::TabletPress) {
                    eventEater.reset(new SinglePressEventEater());
                    popupMenu->installEventFilter(eventEater.data());
                    popupPos = QCursor::pos() + QPoint(10,10);
                } else if (requestingEventType == QEvent::TouchBegin) {
                    popupPos = touchPos.toPoint();
                } else {
                    popupPos = QCursor::pos();
                }

                popupMenu->exec(popupPos);
                popupMenu->clear();
            }
        });
    } else if (KisPopupWidgetInterface *popupWidget = inputManager()->toolProxy()->popupWidget()) { // Handle other popup widgets...
        if (!popupWidget->onScreen()) {
            QPoint pos = eventPos(event);
            if (pos.isNull()) {
                pos = inputManager()->canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());
            }
            inputManager()->registerPopupWidget(popupWidget);
            popupWidget->popup(pos);
        } else {
            popupWidget->dismiss();
        }
    }
}
