/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002, 2003, 2004 Nicolas GOUTTE <goutte@kde.org>

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

#include <qfile.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3CString>

#include <kdebug.h>
#include <kurl.h>
#include <kfilterdev.h>
#include <kio/netaccess.h>
#include <QBuffer>
#include "KoPictureKey.h"
#include "KoPictureBase.h"
#include "KoPictureImage.h"
#include "KoPictureEps.h"
#include "KoPictureClipart.h"
#include "KoPictureWmf.h"
#include "KoPictureShared.h"
#include <kcodecs.h>


KoPictureShared::KoPictureShared(void) : m_base(NULL)
{
}

void KoPictureShared::assignPictureId( uint _id)
{
    m_pictureId = _id;
}

QString KoPictureShared::uniquePictureId() const
{
    return "Pictures"+ QString::number(m_pictureId);
}

KoPictureShared::~KoPictureShared(void)
{
    delete m_base;
}

KoPictureShared::KoPictureShared(const KoPictureShared &other)
    : Q3Shared() // Some compilers want it explicitly!
{
    // We need to use newCopy, because we want a real copy, not just a copy of the part of KoPictureBase
    if (other.m_base)
        m_base=other.m_base->newCopy();
    else
        m_base=NULL;
}

KoPictureShared& KoPictureShared::operator=( const KoPictureShared &other )
{
    clear();
    kDebug(30003) << "KoPictureShared::= before" << endl;
    if (other.m_base)
        m_base=other.m_base->newCopy();
    kDebug(30003) << "KoPictureShared::= after" << endl;
    return *this;
}

KoPictureType::Type KoPictureShared::getType(void) const
{
    if (m_base)
        return m_base->getType();
    return KoPictureType::TypeUnknown;
}

bool KoPictureShared::isNull(void) const
{
    if (m_base)
        return m_base->isNull();
    return true;
}

void KoPictureShared::draw(QPainter& painter, int x, int y, int width, int height, int sx, int sy, int sw, int sh, bool fastMode)
{
    if (m_base)
        m_base->draw(painter, x, y, width, height, sx, sy, sw, sh, fastMode);
    else
    {
        // Draw a red box (easier DEBUG)
        kWarning(30003) << "Drawing red rectangle! (KoPictureShared::draw)" << endl;
        painter.save();
        painter.setBrush(QColor(255,0,0));
        painter.drawRect(x,y,width,height);
        painter.restore();
    }
}

bool KoPictureShared::loadWmf(QIODevice* io)
{
    kDebug(30003) << "KoPictureShared::loadWmf" << endl;
    if (!io)
    {
        kError(30003) << "No QIODevice!" << endl;
        return false;
    }

    clear();

    // The extension .wmf was used (KOffice 1.1.x) for QPicture files
    // For an extern file or in the storage, .wmf can mean a real Windows Meta File.

    QByteArray array ( io->readAll() );

    if ((array[0]=='Q') && (array[1]=='P') &&(array[2]=='I') && (array[3]=='C'))
    {
        m_base=new KoPictureClipart();
        setExtension("qpic");
    }
    else
    {
        m_base=new KoPictureWmf();
        setExtension("wmf");
    }
    return m_base->loadData(array, m_extension);
}

bool KoPictureShared::loadTmp(QIODevice* io)
// We have a temp file, probably from a downloaded file
//   We must check the file type
{
    kDebug(30003) << "KoPictureShared::loadTmp" << endl;
    if (!io)
    {
        kError(30003) << "No QIODevice!" << endl;
        return false;
    }

    // The extension .wmf was used (KOffice 1.1.x) for QPicture files
    // For an extern file or in the storage, .wmf can mean a real Windows Meta File.

    QByteArray array ( io->readAll() );
    return identifyAndLoad( array );
}

bool KoPictureShared::identifyAndLoad( QByteArray array )
{
    if ( array.size() < 5 )
    {
        kError(30003) << "Picture is less than 5 bytes long!" << endl;
        return false;
    }

    QString strExtension;
    bool flag=false;

    // Try to find the file type by comparing magic on the first few bytes!
    // ### TODO: could not QImageIO::imageFormat do it too? (At least most of them?)
    if ((array[0]==char(0x89)) && (array[1]=='P') &&(array[2]=='N') && (array[3]=='G'))
    {
        strExtension="png";
    }
    else if ((array[0]==char(0xff)) && (array[1]==char(0xd8)) &&(array[2]==char(0xff)) && (array[3]==char(0xe0)))
    {
        strExtension="jpeg";
    }
    else if ((array[0]=='B') && (array[1]=='M'))
    {
        strExtension="bmp";
    }
    else if ((array[0]==char(0xd7)) && (array[1]==char(0xcd)) &&(array[2]==char(0xc6)) && (array[3]==char(0x9a)))
    {
        strExtension="wmf";
    }
    else if ((array[0]=='<') && (array[1]=='?') && ( array[2]=='x' ) && (array[3]=='m') && ( array[4]=='l' ) )
    {
        strExtension="svg";
    }
    else if ((array[0]=='Q') && (array[1]=='P') &&(array[2]=='I') && (array[3]=='C'))
    {
        strExtension="qpic";
    }
    else if ((array[0]=='%') && (array[1]=='!') &&(array[2]=='P') && (array[3]=='S'))
    {
        strExtension="eps";
    }
    else if ((array[0]==char(0xc5)) && (array[1]==char(0xd0)) && (array[2]==char(0xd3)) && (array[3]==char(0xc6)))
    {
        // So called "MS-DOS EPS file"
        strExtension="eps";
    }
    else if ((array[0]=='G') && (array[1]=='I') && (array[2]=='F') && (array[3]=='8'))
    {
        // GIF (87a or 89a)
        strExtension="gif";
    }
    else if ( ( array[0] == char( 0037 ) ) && ( array[1] == char( 0213 ) ) )
    {
        // Gzip
        QBuffer buffer(&array);
        buffer.open(QIODevice::ReadOnly);

        const bool flag = loadCompressed( &buffer, "application/x-gzip", "tmp" );
        buffer.close();
        return flag;
    }
    else if ( ( array[0] == 'B' ) && ( array[1] == 'Z' ) && ( array[2] == 'h') )
    {
        // BZip2
        QBuffer buffer(&array);
        buffer.open(QIODevice::ReadOnly);
        const bool flag = loadCompressed( &buffer, "application/x-bzip2", "tmp" );
        buffer.close();
        return flag;
    }
    else
    {
        kDebug(30003) << "Cannot identify the type of temp file!"
            << " Trying to convert to PNG! (in KoPictureShared::loadTmp" << endl;

        // Do not trust QBuffer and do not work directly on the QByteArray array
        // DF: It would be faster to work on array here, and to create a completely
        // different QBuffer for the writing code!
        QBuffer buf( &array );
        if (!buf.open(QIODevice::ReadOnly))
        {
            kError(30003) << "Could not open read buffer!" << endl;
            return false;
        }

        QImageReader imageReader( &buf );
        QImage image = imageReader.read();
        if ( image.isNull() )
        {
            kError(30003) << "Could not read image!" << endl;
            return false;
        }
        buf.close();

        if ( !buf.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        {
            kError(30003) << "Could not open write buffer!" << endl;
            return false;
        }

        QImageWriter imageWriter( &buf, "PNG" );

        if ( !imageWriter.write( image ) )
        {
            kError(30003) << "Could not write converted image!" << endl;
            return false;
        }
        buf.close();

        array = buf.buffer();

        strExtension="png";
    }

    kDebug(30003) << "Temp file considered to be " << strExtension << endl;

    clearAndSetMode(strExtension);
    if (m_base)
        flag = m_base->loadData(array,strExtension);
    setExtension(strExtension);

    return flag;
}



bool KoPictureShared::loadXpm(QIODevice* io)
{
    kDebug(30003) << "KoPictureShared::loadXpm" << endl;
    if (!io)
    {
        kError(30003) << "No QIODevice!" << endl;
        return false;
    }

    clear();

    // Old KPresenter XPM files have char(1) instead of some "
    // Therefore we need to treat XPM separately

    QByteArray array=io->readAll();

    // As XPM files are normally only ASCII files, we can replace it without problems

    int pos=0;

    while ((pos=array.find(char(1),pos))!=-1)
    {
        array[pos]='"';
    }

    // Now that the XPM file is corrected, we need to load it.

    m_base=new KoPictureImage();

    QBuffer buffer(&array);
    bool check = m_base->load(&buffer,"xpm");
    setExtension("xpm");
    return check;
}

bool KoPictureShared::save(QIODevice* io) const
{
    if (!io)
        return false;
    if (m_base)
        return m_base->save(io);
    return false;
}

bool KoPictureShared::saveAsBase64( KoXmlWriter& writer ) const
{
    if ( m_base )
        m_base->saveAsBase64( writer );
    return false;
}

void KoPictureShared::clear(void)
{
    // Clear does not reset the key m_key!
    delete m_base;
    m_base=NULL;
}

void KoPictureShared::clearAndSetMode(const QString& newMode)
{
    delete m_base;
    m_base=NULL;

    const QString mode=newMode.lower();

    if ((mode=="svg") || (mode=="qpic"))
    {
        m_base=new KoPictureClipart();
    }
    else if (mode=="wmf")
    {
        m_base=new KoPictureWmf();
    }
    else if ( (mode=="eps") || (mode=="epsi") || (mode=="epsf") )
    {
        m_base=new KoPictureEps();
    }
    else
    {   // TODO: test if QImageIO really knows the file format
        m_base=new KoPictureImage();
    }
}

QString KoPictureShared::getExtension(void) const
{
    return m_extension;
}

void KoPictureShared::setExtension(const QString& extension)
{
    m_extension = extension;
}

QString KoPictureShared::getMimeType(void) const
{
   if (m_base)
        return m_base->getMimeType(m_extension);
    return QString(NULL_MIME_TYPE);
}


bool KoPictureShared::loadFromBase64( const Q3CString& str )
{
    clear();
    QByteArray data;
    KCodecs::base64Decode( str, data );
    return identifyAndLoad( data );
}

bool KoPictureShared::load(QIODevice* io, const QString& extension)
{
    kDebug(30003) << "KoPictureShared::load(QIODevice*, const QString&) " << extension << endl;
    bool flag=false;
    QString ext(extension.lower());
    if (ext=="wmf")
        flag=loadWmf(io);
    else if (ext=="tmp") // ### TODO: also remote scripts need this, don't they?
        flag=loadTmp(io);
    else if ( ext == "bz2" )
    {
        flag = loadCompressed( io, "application/x-bzip2", "tmp" );
    }
    else if ( ext == "gz" )
    {
        flag = loadCompressed( io, "application/x-gzip", "tmp" );
    }
    else if ( ext == "svgz" )
    {
        flag = loadCompressed( io, "application/x-gzip", "svg" );
    }
    else
    {
        clearAndSetMode(ext);
        if (m_base)
            flag = m_base->load(io, ext);
        setExtension(ext);
    }
    if (!flag)
    {
        kError(30003) << "File was not loaded! (KoPictureShared::load)" << endl;
    }
    return flag;
}

bool KoPictureShared::loadFromFile(const QString& fileName)
{
    kDebug(30003) << "KoPictureShared::loadFromFile " << fileName << endl;
    if ( fileName.isEmpty() )
    {
        kError(30003) << "Cannot load file with empty name!" << endl;
        return false;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    bool flag = false;
    const int pos=fileName.findRev('.');
    if (pos==-1)
    {
        kDebug(30003) << "File with no extension!" << endl;
        // As we have no extension, consider it like a temporary file
        flag = loadTmp( &file );
    }
    else
    {
        const QString extension( fileName.mid( pos+1 ) );
        // ### TODO: check if the extension if gz or bz2 and find the previous extension
        flag = load( &file, extension );
    }
    file.close();
    return flag;
}

QSize KoPictureShared::getOriginalSize(void) const
{
    if (m_base)
        return m_base->getOriginalSize();
    return QSize(0,0);
}

QPixmap KoPictureShared::generatePixmap(const QSize& size, bool smoothScale)
{
    if (m_base)
        return m_base->generatePixmap(size, smoothScale);
    return QPixmap();
}

Q3DragObject* KoPictureShared::dragObject( QWidget *dragSource, const char *name )
{
    if (m_base)
        return m_base->dragObject( dragSource, name );
    return 0L;
}

QImage KoPictureShared::generateImage(const QSize& size)
{
    if (m_base)
        return m_base->generateImage( size );
    return QImage();
}

bool KoPictureShared::hasAlphaBuffer() const
{
   if (m_base)
       return m_base->hasAlphaBuffer();
   return false;
}

void KoPictureShared::setAlphaBuffer(bool enable)
{
    if (m_base)
        m_base->setAlphaBuffer(enable);
}

QImage KoPictureShared::createAlphaMask(Qt::ImageConversionFlags flags) const
{
    if (m_base)
        return m_base->createAlphaMask(flags);
    return QImage();
}

void KoPictureShared::clearCache(void)
{
    if (m_base)
        m_base->clearCache();
}

bool KoPictureShared::loadCompressed( QIODevice* io, const QString& mimeType, const QString& extension )
{
    // ### TODO: check that we do not have an endless recursion
    QIODevice* in = KFilterDev::device( io, mimeType, false);

    if ( !in )
    {
        kError(30003) << "Cannot create device for uncompressing! Aborting!" << endl;
        return false;
    }


    if ( !in->open( QIODevice::ReadOnly ) )
    {
        kError(30003) << "Cannot open file for uncompressing! Aborting!" << endl;
        delete in;
        return false;
    }

    const bool flag = load( in, extension );

    in->close();
    delete in;

    return flag;
}
