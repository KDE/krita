/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include <QWidget>

#include "KoTool.h"
#include "KoCanvasBase.h"
#include "KoViewConverter.h"
#include "KoGfxEvent.h"
#include "KoTool.moc"

KoTool::KoTool(const QString & name, const QString & id, const QString & type, KoCanvasBase *canvas )
: m_optionWidget( 0 )
, m_action( 0 )
, m_canvas(canvas)
, m_name( name )
, m_id( id )
, m_type( type )
{
}

quint32 KoTool::priority() {
    return 0;
}

QString KoTool::quickHelp() {
    return  QString( "" );
}

void KoTool::setup(KActionCollection *ac) {
    Q_UNUSED(ac);
}
QWidget * KoTool::optionWidget(QWidget * parent) {
    if ( !m_optionWidget )
        m_optionWidget = new QWidget( parent );

    return m_optionWidget;
}

void KoTool::activate(bool temporary) {
    Q_UNUSED(temporary);
}

void KoTool::deactivate() {
}


bool KoTool::wantsAutoScroll() {
    return true;
}
QCursor KoTool::cursor( const QPointF &position ) {
    Q_UNUSED(position);
    return Qt::ArrowCursor;
}

void KoTool::mouseDoubleClickEvent( KoGfxEvent *event ) {
    event->ignore();
}

void KoTool::keyPressEvent(QKeyEvent *e) {
    e->ignore();
}

void KoTool::keyReleaseEvent(QKeyEvent *e) {
    e->ignore();
}
