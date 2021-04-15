/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2021 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 * SPDX-FileCopyrightText: 2021 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPopupWidgetAction.h".h"

#include <QCursor>
#include <QMenu>

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
    : KisAbstractInputAction("Show Popup Widget"),
      m_requestedWithStylus(false)
{
    setName(i18n("Show Popup Widget"));
    setDescription(i18n("Show the current tool's popup widget."));
}

KisPopupWidgetAction::~KisPopupWidgetAction()
{
}

void KisPopupWidgetAction::begin(int, QEvent *event)
{
    if (QMenu *popupMenu = inputManager()->toolProxy()->popupActionsMenu()) { // Handle popup menus...
        m_requestedWithStylus = event && event->type() == QEvent::TabletPress;

        /**
         * Opening a menu changes the focus of the windows, so we should not open it
         * inside the filtering loop. Just raise it using the timer.
         */
        QTimer::singleShot(0, this, [this, popupMenu](){
            if (popupMenu) {
                QPoint stylusOffset;
                QScopedPointer<SinglePressEventEater> eventEater;

                if (m_requestedWithStylus) {
                    eventEater.reset(new SinglePressEventEater());
                    popupMenu->installEventFilter(eventEater.data());
                    stylusOffset += QPoint(10,10);
                }

                popupMenu->exec(QCursor::pos() + stylusOffset);
                popupMenu->clear();
            }
        });
    } else if (KisPopupWidgetInterface *popupWidget = inputManager()->toolProxy()->popupWidget()) { // Handle other popup widgets...
        QPoint pos = eventPos(event);

        if (pos.isNull()) {
            pos = inputManager()->canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());
        }

        inputManager()->registerPopupWidget(popupWidget);
        popupWidget->popup(pos);
    }
}
