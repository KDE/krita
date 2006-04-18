/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002, 2003 Nicolas GOUTTE <goutte@kde.org>

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

#include "KoPictureBase.h"

#include <KoXmlWriter.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>

#include <kcodecs.h>
#include <QPainter>
#include <q3picture.h>
#include <QPixmap>
#include <q3dragobject.h>
//Added by qt3to4:
#include <QBuffer>
static int s_useSlowResizeMode = -1; // unset

KoPictureBase::KoPictureBase(void)
{
    // Slow mode can be very slow, especially at high zoom levels -> configurable
    if ( s_useSlowResizeMode == -1 )
    {
        KConfigGroup group( KGlobal::config(), "KOfficeImage" );
        s_useSlowResizeMode = group.readEntry( "HighResolution", 1 );
        kDebug(30003) << "HighResolution = " << s_useSlowResizeMode << endl;
    }
}

KoPictureBase::~KoPictureBase(void)
{
}

KoPictureBase* KoPictureBase::newCopy(void) const
{
    return new KoPictureBase(*this);
}

KoPictureType::Type KoPictureBase::getType(void) const
{
    return KoPictureType::TypeUnknown;
}

bool KoPictureBase::isNull(void) const
{
    return true;    // A KoPictureBase is always null.
}

void KoPictureBase::draw(QPainter& painter, int x, int y, int width, int height, int, int, int, int, bool /*fastMode*/)
{
    // Draw a light red box (easier DEBUG)
    kWarning(30003) << "Drawing light red rectangle! (KoPictureBase::draw)" << endl;
    painter.save();
    painter.setBrush(QColor(128,0,0));
    painter.drawRect(x,y,width,height);
    painter.restore();
}

bool KoPictureBase::load(QIODevice* io, const QString& extension)
{
    return loadData(io->readAll(), extension);
}

bool KoPictureBase::loadData(const QByteArray&, const QString&)
{
    // Nothing to load!
    return false;
}

bool KoPictureBase::save(QIODevice*) const
{
    // Nothing to save!
    return false;
}

bool KoPictureBase::saveAsBase64( KoXmlWriter& writer ) const
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    if ( !save( &buffer ) )
        return false;
    QByteArray encoded = KCodecs::base64Encode( buffer.buffer() );
    writer.addTextNode( encoded );
    return true;
}

QSize KoPictureBase::getOriginalSize(void) const
{
    return QSize(0,0);
}

QPixmap KoPictureBase::generatePixmap(const QSize&, bool /*smoothScale*/)
{
    return QPixmap();
}

QString KoPictureBase::getMimeType(const QString&) const
{
    return QString(NULL_MIME_TYPE);
}

bool KoPictureBase::isSlowResizeModeAllowed(void) const
{
    return s_useSlowResizeMode != 0;
}

Q3DragObject* KoPictureBase::dragObject( QWidget * dragSource, const char * name )
{
    QImage image (generateImage(getOriginalSize()));
    if (image.isNull())
        return 0L;
    else
        return new Q3ImageDrag( image, dragSource, name );
}

QImage KoPictureBase::generateImage(const QSize& size)
{
    return generatePixmap(size,true).toImage();
}

void KoPictureBase::clearCache(void)
{
    // Nothign to do!
}
