/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

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
 * Boston, MA 02110-1301, USA.
*/

#include <QtGui>

#include <kdebug.h>

#include "kis_canvas2.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"

#include "kis_painterlymixer.h"

KisPainterlyMixer::KisPainterlyMixer(QWidget *parent, KisView2 *view)
    : QWidget(parent), m_view(view)
{
    setupUi(this);

    m_canvas->installEventFilter(this);
}

KisPainterlyMixer::~KisPainterlyMixer()
{
}

bool KisPainterlyMixer::eventFilter(QObject * /*obj*/, QEvent *event)
{
    if (event->type() == QEvent::TabletPress) {
        m_pressed = true;
        return true;
    }
    if (event->type() == QEvent::TabletRelease) {
        m_pressed = false;
        return true;
    }

    // The main loop!
    if (event->type() == QEvent::TabletMove && m_pressed) {
        QTabletEvent *e = static_cast<QTabletEvent*>(event);
        int x = e->x(), y = e->y();
        int xTilt = e->xTilt(), yTilt = e->yTilt();
        qreal pressure = e->pressure(), rotation = e->rotation();

        switch (e->device()) {
        case QTabletEvent::Stylus:
            kDebug() << "You're using a Stylus" << endl;
            break;
        default:
            kDebug() << "We handle only Stylus for now" << endl;
        }

        // TODO: the pen draws, the eraser erases...
        switch (e->pointerType()) {
        case QTabletEvent::Cursor:
            kDebug() << "QTabletEvent::Cursor" << endl;
            break;
        case QTabletEvent::Pen:
            kDebug() << "QTabletEvent::Pen" << endl;
            break;
        case QTabletEvent::Eraser:
            kDebug() << "QTabletEvent::Eraser" << endl;
            break;
        default:
            kDebug() << "Pointer type not handled" << endl;
        }

        kDebug() << "Information about the event:" << endl;
        kDebug() << "\tX: " << x << ", Y: " << y << endl;
        kDebug() << "\txTilt: " << xTilt << ", yTilt: " << yTilt << endl;
        kDebug() << "\tPressure: " << pressure << ", Rotation: " << rotation << endl;

        // TODO an extendible way to retrieve the wetness of the paintop.
        KoID opID = m_view->canvasBase()->resourceProvider()->resource(KisResourceProvider::CurrentPaintop).value<KoID>();
        kDebug() << opID.id() << endl;

        // TODO get the remaining paintop settings
        // TODO draw the footprint
        // TODO resize the footprint to the size set in the mixer
        // TODO mix the colors in the footprint with these in the mixer canvas
        // TODO draw the colors on the canvas (what to do with gamut, colorspaces etc?)
        // TODO set the color(s) of the paintop (what about the paintop redesign?)

        return true;
    }

    return false;
}


#include "kis_painterlymixer.moc"
