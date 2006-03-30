/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (c) 2001 David Faure <faure@kde.org>
   Copyright (C) 2002 Nicolas GOUTTE <goutte@kde.org>

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

#include <qbuffer.h>
#include <qpainter.h>
#include <q3picture.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kdeversion.h>

#include <kowmfpaint.h>
#include "KoPictureKey.h"
#include "KoPictureBase.h"
#include "KoPictureWmf.h"

KoPictureWmf::KoPictureWmf(void) : m_clipart(KoPictureType::formatVersionQPicture)
{
}

KoPictureWmf::~KoPictureWmf(void)
{
}

KoPictureBase* KoPictureWmf::newCopy(void) const
{
    return new KoPictureWmf(*this);
}

KoPictureType::Type KoPictureWmf::getType(void) const
{
    return KoPictureType::TypeWmf;
}

bool KoPictureWmf::isNull(void) const
{
    return m_clipart.isNull();
}

void KoPictureWmf::drawQPicture(QPicture& clipart, QPainter& painter,
    int x, int y, int width, int height, int sx, int sy, int sw, int sh)
{
    kDebug(30003) << "Drawing KoPictureWmf " << this << endl;
    kDebug(30003) << "  x=" << x << " y=" << y << " width=" << width << " height=" << height << endl;
    kDebug(30003) << "  sx=" << sx << " sy=" << sy << " sw=" << sw << " sh=" << sh << endl;
    painter.save();
    // Thanks to Harri, Qt3 makes it much easier than Qt2 ;)
    QRect br = clipart.boundingRect();
    kDebug(30003) << "  Bounding rect. " << br << endl;

    painter.translate(x,y); // Translating must be done before scaling!
    if ( br.width() && br.height() )
        painter.scale(double(width)/double(br.width()),double(height)/double(br.height()));
    else
        kWarning(30003) << "Null bounding rectangle: " << br.width() << " x " << br.height() << endl;
    painter.drawPicture(0,0,clipart);
    painter.restore();
}

void KoPictureWmf::draw(QPainter& painter, int x, int y, int width, int height, int sx, int sy, int sw, int sh, bool /*fastMode*/)
{
    drawQPicture(m_clipart, painter, x, y, width, height, sx, sy, sw, sh);
}

bool KoPictureWmf::loadData(const QByteArray& array, const QString& /* extension */)
{
    // Second, create the original clipart
    kDebug(30003) << "Trying to load clipart... (Size:" << array.size() << ")" << endl;
    m_rawData=array;

    KoWmfPaint wmf;
    if (!wmf.load(array))
    {
        kWarning(30003) << "Loading WMF has failed! (KoPictureWmf::load)" << endl;
        return false;
    }
    m_originalSize = wmf.boundingRect().size();
    // draw wmf file with relative coordinate
    wmf.play(m_clipart, true);

    return true;
}

bool KoPictureWmf::save(QIODevice* io) const
{
    // We save the raw data, as the SVG supposrt in QPicture is poor
    Q_ULONG size=io->write(m_rawData); // WARNING: writeBlock returns Q_LONG but size() Q_ULONG!
    return (size==m_rawData.size());
}

QSize KoPictureWmf::getOriginalSize(void) const
{
    return m_originalSize;
}

QPixmap KoPictureWmf::generatePixmap(const QSize& size, bool /*smoothScale*/)
{
    // Not sure if it works, but it worked for KoPictureFilePreviewWidget::setClipart
    QPixmap pixmap(size);
    QPainter p;

    p.begin( &pixmap );
    p.setBackgroundColor( Qt::white );
    pixmap.fill( Qt::white );

    if ( m_originalSize.width() && m_originalSize.height() )
        p.scale( (double)pixmap.width() / (double)m_originalSize.width(), (double)pixmap.height() / (double)m_originalSize.height() );
    p.drawPicture( 0, 0, m_clipart );
    p.end();
    return pixmap;
}

QString KoPictureWmf::getMimeType(const QString& /* extension */) const
{
    return "image/x-wmf";
}
