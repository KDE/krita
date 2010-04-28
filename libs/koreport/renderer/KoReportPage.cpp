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
#include <qwidget.h>
#include <kdebug.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <KoPageFormat.h>
#include <KoUnit.h>
#include <KoGlobal.h>

#include <renderobjects.h>
#include <QPainter>

KoReportPage::KoReportPage(QWidget *parent, ORODocument *document)
        : QWidget(parent)
{
    setAttribute(Qt::WA_NoBackground);
    kDebug() << "CREATED PAGE";
    m_reportDocument = document;
    m_page = 1;
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

    setFixedSize(pageWidth, pageHeight);

    kDebug() << "PAGE IS " << pageWidth << "x" << pageHeight;
    m_repaint = true;
    m_pixmap = new QPixmap(pageWidth, pageHeight);
    setAutoFillBackground(true);

    m_renderer = m_factory.createInstance("screen");

    renderPage(1);
}

KoReportPage::~KoReportPage()
{
    delete m_renderer;
    m_renderer = 0;
}

void KoReportPage::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawPixmap(QPoint(0, 0), *m_pixmap);
}

void KoReportPage::renderPage(int page)
{
    kDebug() << page;
//js: is m_page needed?
    m_page = page;
    m_pixmap->fill();
    QPainter qp(m_pixmap);
    if (m_reportDocument) {
        KoReportRendererContext cxt;
        cxt.painter = &qp;
        m_renderer->render(cxt, m_reportDocument, m_page - 1);
    }
    m_repaint = true;
    repaint();
}

#include "KoReportPage.moc"
