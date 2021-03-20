/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_show_palette_action.h"

#include <QCursor>
#include <QMenu>

#include <klocalizedstring.h>

#include <kis_favorite_resource_manager.h>
#include <kis_canvas2.h>
#include "kis_tool_proxy.h"

#include "kis_input_manager.h"

KisShowPaletteAction::KisShowPaletteAction()
    : KisAbstractInputAction("Show Popup Palette"),
      m_requestedWithStylus(false)
{
    setName(i18n("Show Popup Palette"));
    setDescription(i18n("The <i>Show Popup Palette</i> displays the popup palette."));
}

KisShowPaletteAction::~KisShowPaletteAction()
{

}

int KisShowPaletteAction::priority() const
{
    return 1;
}

void KisShowPaletteAction::begin(int, QEvent *event)
{
    m_menu = inputManager()->toolProxy()->popupActionsMenu();

    if (m_menu) {
        m_requestedWithStylus = event && event->type() == QEvent::TabletPress;

        /**
         * Opening a menu changes the focus of the windows, so we should not open it
         * inside the filtering loop. Just raise it using the timer.
         */
        QTimer::singleShot(0, this, SLOT(slotShowMenu()));

    } else {
        QPoint pos = eventPos(event);
        if (pos.isNull()) {
            pos = inputManager()->canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());
        }

        inputManager()->canvas()->slotShowPopupPalette(pos);
    }
}

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

void KisShowPaletteAction::slotShowMenu()
{
    if (m_menu) {

        QPoint stylusOffset;
        QScopedPointer<SinglePressEventEater> eater;

        if (m_requestedWithStylus) {
            eater.reset(new SinglePressEventEater());
            m_menu->installEventFilter(eater.data());
            stylusOffset += QPoint(10,10);
        }

        m_menu->exec(QCursor::pos() + stylusOffset);
        m_menu.clear();
    }
}
