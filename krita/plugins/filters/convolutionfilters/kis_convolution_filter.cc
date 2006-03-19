/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "qdom.h"
#include "klocale.h"
#include "kdebug.h"

#include "kis_painter.h"
#include "kis_convolution_filter.h"
#include "kis_convolution_painter.h"
#include "kis_progress_display_interface.h"
#include "kis_progress_subject.h"

void KisConvolutionConfiguration::fromXML(const QString & s)
{
    m_matrix = new KisKernel();

    QDomDocument doc;
    doc.setContent( s );
    QDomElement e = doc.documentElement();
    QDomNode n = e.firstChild();

    m_name = e.attribute("name");
    m_version = e.attribute("version").toInt();

    QDomElement matrix = n.toElement();
    m_matrix->width = QString( matrix.attribute( "width" ) ).toInt();
    m_matrix->height = QString( matrix.attribute( "height" ) ).toInt();
    m_matrix->offset = QString( matrix.attribute( "offset" ) ).toInt();
    m_matrix->factor = QString( matrix.attribute( "factor" ) ).toInt();

    m_matrix->data = new Q_INT32[m_matrix->width * m_matrix->height];

    QStringList data = QStringList::split( ",", e.text() );
    QStringList::Iterator start = data.begin();
    QStringList::Iterator end = data.end();
    int i = 0;
    for ( QStringList::Iterator it = start; it != end; ++it ) {
        QString s = *it;
        m_matrix->data[i] = s.toInt();
        i++;
    }
}

QString KisConvolutionConfiguration::toString()
{
    QDomDocument doc = QDomDocument("filterconfig");
    QDomElement root = doc.createElement( "filterconfig" );
    root.setAttribute( "name", name() );
    root.setAttribute( "version", version() );

    doc.appendChild( root );

    QDomElement e = doc.createElement( "kernel" );
    e.setAttribute( "width", m_matrix->width );
    e.setAttribute( "height", m_matrix->height );
    e.setAttribute( "offset", m_matrix->offset );
    e.setAttribute( "factor", m_matrix->factor );

    QString data;

    for ( uint i = 0; i < m_matrix->width * m_matrix->height; ++i ) {
        data += QString::number( m_matrix->data[i] );
        data += ",";
    }

    QDomText text = doc.createCDATASection(data);
    e.appendChild(text);
    root.appendChild(e);

    return doc.toString();

}

void KisConvolutionFilter::process(KisPaintDeviceSP src,
                                   KisPaintDeviceSP dst,
                                   KisFilterConfiguration* configuration,
                                   const QRect& rect)
{

    if (dst != src) {
        kdDebug() << "src != dst\n";
        KisPainter gc(dst);
        gc.bitBlt(rect.x(), rect.y(), COMPOSITE_COPY, src, rect.x(), rect.y(), rect.width(), rect.height());
        gc.end();
    }

    KisConvolutionPainter painter( dst );
    if (m_progressDisplay)
        m_progressDisplay->setSubject( &painter, true, true );

    KisKernelSP kernel = ((KisConvolutionConfiguration*)configuration)->matrix();
    painter.applyMatrix(kernel, rect.x(), rect.y(), rect.width(), rect.height(), BORDER_REPEAT);

    if (painter.cancelRequested()) {
        cancel();
    }

    setProgressDone();
}

KisFilterConfiguration* KisConvolutionConstFilter::configuration(QWidget*)
{
    return new KisConvolutionConfiguration( id().id(), m_matrix );
}

#include "kis_convolution_filter.moc"
