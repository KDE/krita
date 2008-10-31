/* This file is part of the KDE project
 * Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPABackgroundTool.h"

//Qt includes
#include <QMap>
#include <QLabel>

//KDE includes
#include <klocale.h>
#include <kdebug.h>

//KOffice includes
#include <KoPACanvas.h>
#include <KoCanvasResourceProvider.h>
#include <KoPAView.h>
#include <KoPointerEvent.h>

#include "KoPAMasterPageDocker.h"
#include "KoPABackgroundToolWidget.h"

KoPABackgroundTool::KoPABackgroundTool( KoCanvasBase *canvas )
: KoTool( canvas )
{
}

KoPABackgroundTool::~KoPABackgroundTool()
{
}


void KoPABackgroundTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED( painter );
    Q_UNUSED( converter );
}


void KoPABackgroundTool::activate( bool temporary )
{
    Q_UNUSED( temporary );

    KoPAView *view = static_cast<KoPACanvas *>(m_canvas)->koPAView();
    m_canvas->resourceProvider()->setResource( KoPageApp::CurrentPage, view->activePage() );
}

void KoPABackgroundTool::deactivate()
{
    m_canvas->resourceProvider()->clearResource( KoPageApp::CurrentPage );
}

void KoPABackgroundTool::mousePressEvent( KoPointerEvent *event )
{
    event->ignore();
}

void KoPABackgroundTool::mouseMoveEvent( KoPointerEvent *event )
{
    event->ignore();
}

void KoPABackgroundTool::mouseReleaseEvent( KoPointerEvent *event )
{
    event->ignore();
}


QMap<QString, QWidget *> KoPABackgroundTool::createOptionWidgets()
{
    KoPABackgroundToolWidget * widget = new KoPABackgroundToolWidget();
    QMap<QString, QWidget *> widgets;
    widgets.insert( i18n("Background Tool"), widget );
    QLabel dummy4( i18n("Use the styles docker to manipulate the backgound.") );
#if 0
    KoPAMasterPageDocker *masterPageDocker = new KoPAMasterPageDocker();
    masterPageDocker->setView( static_cast<KoPACanvas *>(m_canvas)->koPAView() );
    widgets.insert( i18n("Master Page"), masterPageDocker );
#endif
    return widgets;
}

#include "KoPABackgroundTool.moc"
