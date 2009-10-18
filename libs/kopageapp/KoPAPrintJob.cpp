/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPAPrintJob.h"

#include "KoPAView.h"
#include "KoPADocument.h"
#include "KoPAUtil.h"
#include "KoPAPageBase.h"
#include "KoPAPageProvider.h"

#include <QPainter>
#include <kdebug.h>

KoPAPrintJob::KoPAPrintJob(KoPAView * view)
: KoPrintJob(view)
, m_pages(view->kopaDocument()->pages())
, m_pageProvider(static_cast<KoPAPageProvider*>(view->kopaDocument()->dataCenterMap()[KoPAPageProvider::ID]))
{
    // TODO this feels wrong
    printer().setFromTo(1, m_pages.size());
}

KoPAPrintJob::~KoPAPrintJob()
{
}

QPrinter & KoPAPrintJob::printer()
{
    return m_printer;
}

QList<QWidget*> KoPAPrintJob::createOptionWidgets() const
{
    return QList<QWidget*>();
}

// TODO create a lot of fancy options for printing
// e.g. print also notes
// For now we print to the center of the page honoring the margins
// The page is zoomed to be as big as possible
void KoPAPrintJob::startPrinting(RemovePolicy removePolicy)
{
    int fromPage = m_printer.fromPage() ? m_printer.fromPage() - 1: 0;
    int toPage = m_printer.toPage() ? m_printer.toPage() - 1: m_pages.size() - 1;

    Q_ASSERT( fromPage >= 0 && fromPage < m_pages.size() );
    Q_ASSERT( toPage >= 0 && toPage < m_pages.size() );

    KoZoomHandler zoomHandler;
    zoomHandler.setResolution( m_printer.resolution(), m_printer.resolution() );

    QSize size = m_printer.pageRect().size();

    QPainter painter( &m_printer );
    for ( int i = fromPage; i <= toPage; ++i ) {
        painter.save();
        if (i != fromPage) {
            m_printer.newPage();
        }

        KoPAPageBase * page = m_pages.at(i);
        const KoPageLayout & layout = page->pageLayout();
        KoPAUtil::setZoom( layout, size, zoomHandler );
        QRect pageRect( KoPAUtil::pageRect( layout, size, zoomHandler ) );

        painter.setClipRect( pageRect );
        painter.setRenderHint( QPainter::Antialiasing );
        painter.translate( pageRect.topLeft() );
        m_pageProvider->setMasterPageNumber(i+1);
        page->paintPage( painter, zoomHandler );
        painter.restore();
    }

    if ( removePolicy == DeleteWhenDone ) {
        deleteLater();
    }
}
