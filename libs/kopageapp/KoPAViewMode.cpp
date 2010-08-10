/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPAViewMode.h"

#include "KoPACanvas.h"
#include "KoPAPageBase.h"
#include "KoPAView.h"
#include <KoCanvasController.h>
#include <KoPageLayout.h>

#include <QCloseEvent>

KoPAViewMode::KoPAViewMode( KoPAViewBase * view, KoPACanvasBase * canvas )
: m_canvas( canvas )
, m_toolProxy( canvas->toolProxy() )
, m_view( view )
{
}

KoPAViewMode::~KoPAViewMode()
{
}

void KoPAViewMode::closeEvent( QCloseEvent * event )
{
    event->ignore();
}

void KoPAViewMode::setMasterMode( bool master )
{
    Q_UNUSED(master);
}

bool KoPAViewMode::masterMode()
{
    return false;
}

void KoPAViewMode::activate( KoPAViewMode * previousViewMode )
{
    Q_UNUSED( previousViewMode );
    m_canvas->updateSize();
    updateActivePage( m_view->activePage() );
    // this is done to set the preferred center
    m_canvas->canvasController()->setCanvasMode( KoCanvasController::Centered );
    m_canvas->canvasController()->recenterPreferred();
}

void KoPAViewMode::deactivate()
{
}

KoPACanvasBase * KoPAViewMode::canvas() const
{
    return m_canvas;
}

KoPAViewBase * KoPAViewMode::view() const
{
    return m_view;
}

KoViewConverter * KoPAViewMode::viewConverter( KoPACanvasBase * canvas )
{
    return m_view->viewConverter( canvas );
}

void KoPAViewMode::updateActivePage( KoPAPageBase *page )
{
    m_view->doUpdateActivePage( page );
}

void KoPAViewMode::addShape( KoShape *shape )
{
    Q_UNUSED( shape );
}

void KoPAViewMode::removeShape( KoShape *shape )
{
    Q_UNUSED( shape );
}

const KoPageLayout &KoPAViewMode::activePageLayout() const
{
    return m_view->activePage()->pageLayout();
}

void KoPAViewMode::changePageLayout( const KoPageLayout &pageLayout, bool applyToDocument, QUndoCommand *parent )
{
    Q_UNUSED( pageLayout );
    Q_UNUSED( applyToDocument );
    Q_UNUSED( parent );
}

QPointF KoPAViewMode::origin()
{
    return m_origin;
}

void KoPAViewMode::setOrigin(const QPointF &o)
{
    m_origin = o;
}

#include <KoPAViewMode.moc>

