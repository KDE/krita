/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KoReportPage.h"
#include <QWidget>
#include <kdebug.h>
#include <QColor>
#include <QPixmap>
#include <KoPageFormat.h>
#include <KoUnit.h>
#include <KoGlobal.h>

#include <renderobjects.h>
#include <QPainter>
#include <QTimer>

KoReportPage::KoReportPage(QWidget *parent, ORODocument *document)
        : QObject(parent), QGraphicsRectItem()
{
    //TODO setAttribute(Qt::WA_NoBackground);
    kDebug() << "CREATED PAGE";
    m_reportDocument = document;
    m_page = 0;
    int pageWidth = 0;
    int pageHeight = 0;

    if (m_reportDocument) {
        QString pageSize = m_reportDocument->pageOptions().getPageSize();


        if (pageSize == "Custom") {
            // if this is custom sized sheet of paper we will just use those values
            pageWidth = (int)(m_reportDocument->pageOptions().getCustomWidth());
            pageHeight = (int)(m_reportDocument->pageOptions().getCustomHeight());
        } else {
            // lookup the correct size information for the specified size paper
            pageWidth = m_reportDocument->pageOptions().widthPx();
            pageHeight = m_reportDocument->pageOptions().heightPx();
        }
    }

    setRect(0,0,pageWidth, pageHeight);

    kDebug() << "PAGE IS " << pageWidth << "x" << pageHeight;

    m_pixmap = new QPixmap(pageWidth, pageHeight);

    m_renderer = m_factory.createInstance("screen");
    
    connect(m_reportDocument, SIGNAL(updated(int)), this, SLOT(pageUpdated(int)));

    m_renderTimer = new QTimer();
    m_renderTimer->setSingleShot(true);
    connect(m_renderTimer, SIGNAL(timeout()), this, SLOT(renderCurrentPage()));
    
    renderPage(1);
}

KoReportPage::~KoReportPage()
{
    delete m_renderer;
    delete m_pixmap;
    delete m_renderTimer;
}

void KoReportPage::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->drawPixmap(QPoint(0, 0), *m_pixmap);
}

void KoReportPage::renderPage(int page)
{
    m_page = page - 1;
    m_pixmap->fill();
    QPainter qp(m_pixmap);
    if (m_reportDocument) {
        KoReportRendererContext cxt;
        cxt.painter = &qp;
        m_renderer->render(cxt, m_reportDocument, m_page);
    }
    update();
}

void KoReportPage::pageUpdated(int pageNo)
{
    kDebug() << pageNo << m_page;
    //Refresh this page if it changes
    if (pageNo == m_page) {
        kDebug() << "Current page updated";
        m_renderTimer->start(100);
    }
}

void KoReportPage::renderCurrentPage()
{
    renderPage(m_page + 1);
}


#include "KoReportPage.moc"
