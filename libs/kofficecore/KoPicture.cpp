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

#include <QPainter>
#include <QFile>
//Added by qt3to4:
#include <QPixmap>

#include <kdebug.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include "KoPictureKey.h"
#include "KoPictureBase.h"
#include "KoPictureShared.h"
#include "KoPicture.h"

uint KoPicture::uniqueValue = 0;


KoPicture::KoPicture(void) : m_sharedData(NULL)
{
  m_uniqueName = "Pictures"+ QString::number(uniqueValue++);
}

KoPicture::~KoPicture(void)
{
    unlinkSharedData();
}

QString KoPicture::uniqueName() const
{
  return m_uniqueName;
}

KoPicture::KoPicture(const KoPicture &other)
{
    m_sharedData=NULL;
    (*this)=other;
}

void KoPicture::assignPictureId( uint _id)
{
    if ( m_sharedData )
        m_sharedData->assignPictureId(_id);
}

QString KoPicture::uniquePictureId() const
{
    if ( m_sharedData )
        return m_sharedData->uniquePictureId();
    else
        return QString();
}

KoPicture& KoPicture::operator=( const KoPicture &other )
{
    //kDebug(30003) << "KoPicture::= before" << endl;
    if (other.m_sharedData)
        other.linkSharedData();
    if (m_sharedData)
        unlinkSharedData();
    m_sharedData=other.m_sharedData;
    m_key=other.m_key;
    //kDebug(30003) << "KoPicture::= after" << endl;
    return *this;
}

void KoPicture::unlinkSharedData(void)
{
    if (m_sharedData && m_sharedData->deref())
        delete m_sharedData;

    m_sharedData=NULL;
}

void KoPicture::linkSharedData(void) const
{
    if (m_sharedData)
        m_sharedData->ref();
}

void KoPicture::createSharedData(void)
{
    if (!m_sharedData)
    {
        m_sharedData=new KoPictureShared();
        // Do not call m_sharedData->ref()
    }
}

KoPictureType::Type KoPicture::getType(void) const
{
    if (m_sharedData)
        return m_sharedData->getType();
    return KoPictureType::TypeUnknown;
}

KoPictureKey KoPicture::getKey(void) const
{
    return m_key;
}

void KoPicture::setKey(const KoPictureKey& key)
{
    m_key=key;
}


bool KoPicture::isNull(void) const
{
    if (m_sharedData)
        return m_sharedData->isNull();
    return true;
}

void KoPicture::draw(QPainter& painter, int x, int y, int width, int height, int sx, int sy, int sw, int sh, bool fastMode)
{
    if (m_sharedData)
        m_sharedData->draw(painter, x, y, width, height, sx, sy, sw, sh, fastMode);
    else
    {
        // Draw a white box
        kWarning(30003) << "Drawing white rectangle! (KoPicture::draw)" << endl;
        painter.save();
        painter.setBrush(QColor(255, 255, 255));
        painter.drawRect(x,y,width,height);
        painter.restore();
    }
}

bool KoPicture::loadXpm(QIODevice* io)
{
    kDebug(30003) << "KoPicture::loadXpm" << endl;
    if (!io)
    {
        kError(30003) << "No QIODevice!" << endl;
        return false;
    }
    createSharedData();
    return m_sharedData->loadXpm(io);
}

bool KoPicture::save(QIODevice* io) const
{
    if (!io)
        return false;
    if (m_sharedData)
        return m_sharedData->save(io);
    return false;
}

bool KoPicture::saveAsBase64( KoXmlWriter& writer ) const
{
    if ( m_sharedData )
        return m_sharedData->saveAsBase64( writer );
    return false;
}

void KoPicture::clear(void)
{
    unlinkSharedData();
}

void KoPicture::clearAndSetMode(const QString& newMode)
{
    createSharedData();
    m_sharedData->clearAndSetMode(newMode);
}

QString KoPicture::getExtension(void) const
{
    if (m_sharedData)
        return m_sharedData->getExtension();
    return "null"; // Just a dummy
}

QString KoPicture::getMimeType(void) const
{
    if (m_sharedData)
        return m_sharedData->getMimeType();
    return QString(NULL_MIME_TYPE);
}

bool KoPicture::load(QIODevice* io, const QString& extension)
{
    kDebug(30003) << "KoPicture::load(QIODevice*, const QString&) " << extension << endl;
    createSharedData();

    return m_sharedData->load(io,extension);
}

bool KoPicture::loadFromFile(const QString& fileName)
{
    kDebug(30003) << "KoPicture::loadFromFile " << fileName << endl;
    createSharedData();
    return m_sharedData->loadFromFile(fileName);
}

bool KoPicture::loadFromBase64( const QByteArray& str )
{
    createSharedData();
    return m_sharedData->loadFromBase64( str );
}

QSize KoPicture::getOriginalSize(void) const
{
    if (m_sharedData)
        return m_sharedData->getOriginalSize();
    return QSize(0,0);
}

QPixmap KoPicture::generatePixmap(const QSize& size, bool smoothScale)
{
    if (m_sharedData)
        return m_sharedData->generatePixmap(size, smoothScale);
    return QPixmap();
}

bool KoPicture::setKeyAndDownloadPicture(const KUrl& url, QWidget *window)
{
    bool result=false;

    QString tmpFileName;
    if ( KIO::NetAccess::download(url, tmpFileName, window) )
    {
        KoPictureKey key;
        key.setKeyFromFile( tmpFileName );
        setKey( key );
        result=loadFromFile( tmpFileName );
        KIO::NetAccess::removeTempFile( tmpFileName );
    }

    return result;
}

Q3DragObject* KoPicture::dragObject( QWidget *dragSource, const char *name )
{
    if (m_sharedData)
        return m_sharedData->dragObject( dragSource, name );
    return 0L;
}

QImage KoPicture::generateImage(const QSize& size)
{
    if (m_sharedData)
        return m_sharedData->generateImage( size );
    return QImage();
}

bool KoPicture::hasAlphaBuffer() const
{
    if (m_sharedData)
       return m_sharedData->hasAlphaBuffer();
    return false;
}

void KoPicture::setAlphaBuffer(bool enable)
{
    if (m_sharedData)
        m_sharedData->setAlphaBuffer(enable);
}

QImage KoPicture::createAlphaMask(Qt::ImageConversionFlags flags) const
{
    if (m_sharedData)
        return m_sharedData->createAlphaMask(flags);
    return QImage();
}

void KoPicture::clearCache(void)
{
    if (m_sharedData)
        m_sharedData->clearCache();
}
