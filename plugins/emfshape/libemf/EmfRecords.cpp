/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "EmfEnums.h"
#include "EmfRecords.h"

#include <KDebug>

namespace Libemf
{

/*****************************************************************************/

DeviceInfoHeader::DeviceInfoHeader( QDataStream &stream )
{
    stream >> m_headerSize;
    stream >> m_width;
    // kDebug(33100) << "Width:" << m_width;
    stream >> m_height;
    // kDebug(33100) << "Height:" << m_height;
    stream >> m_planes;
    // kDebug(33100) << "planes:" << m_planes;
    stream >> m_bitCount;
    // kDebug(33100) << "BitCount:" << m_bitCount;
    stream >> m_compression;
    // kDebug(33100) << "Compression:" << m_compression;
    stream >> m_imageSize;
    // kDebug(33100) << "ImageSize:" << m_imageSize;
    stream >> m_xPelsPerMeter;
    stream >> m_yPelsPerMeter;
    stream >> m_colorUsed;
    stream >> m_colorImportant;
}

DeviceInfoHeader::~DeviceInfoHeader()
{
}


/*****************************************************************************/
BitBltRecord::BitBltRecord( QDataStream &stream )
{
    stream >> m_Bounds;
    stream >> m_xDest;
    stream >> m_yDest;
    stream >> m_cxDest;
    stream >> m_cyDest;
    stream >> m_BitBltRasterOperation;
    kDebug(33100) << "bitblt raster operation:" << m_BitBltRasterOperation;
    stream >> m_xSrc;
    stream >> m_ySrc;

    float M11, M12, M21, M22, Dx, Dy;
    stream >> M11;
    stream >> M12;
    stream >> M21;
    stream >> M22;
    stream >> Dx;
    stream >> Dy;
    m_XFormSrc = QMatrix( M11, M12, M21, M22, Dx, Dy );

    stream >> m_red >> m_green >> m_blue >> m_reserved;
    stream >> m_UsageSrc;
    stream >> m_offBmiSrc;
    stream >> m_cbBmiSrc;
    stream >> m_offBitsSrc;
    stream >> m_cbBitsSrc;
    if ( ( m_cbBmiSrc == 0 ) && ( m_cbBmiSrc == 0 ) ) {
	return;
    }
    if ( m_cbBmiSrc == 40 ) {
	m_BmiSrc = new DeviceInfoHeader( stream );
    } else {
	kDebug(33100) << "m_cbBmiSrc:" << m_cbBmiSrc;
	Q_ASSERT( 0 );
    }

    m_imageData.resize( m_cbBitsSrc );
    stream.readRawData( m_imageData.data(), m_cbBitsSrc );
}

BitBltRecord::~BitBltRecord()
{
}

bool BitBltRecord::hasImage() const
{
    return ( ( m_cbBmiSrc != 0 ) && ( m_cbBmiSrc != 0 ) );
}

QImage* BitBltRecord::image() 
{

    if ( ! hasImage() ) {
        return 0;
    }

    if ( m_image != 0 ) {
        return m_image;
    }

    QImage::Format format = QImage::Format_Invalid;
    if ( m_BmiSrc->bitCount() == BI_BITCOUNT_4 ) {
        if ( m_BmiSrc->compression() == 0x00 ) {
            format = QImage::Format_RGB555;
        } else {
            kDebug(33100) << "Unexpected compression format for BI_BITCOUNT_4:"
                     << m_BmiSrc->compression();
            Q_ASSERT( 0 );
        }
    } else if ( m_BmiSrc->bitCount() == BI_BITCOUNT_5 ) {
        format = QImage::Format_RGB888;
    } else {
        kDebug(33100) << "Unexpected format:" << m_BmiSrc->bitCount();
        Q_ASSERT( 0 );
    }
    m_image = new QImage( (const uchar*)m_imageData.constData(),
                          m_BmiSrc->width(), m_BmiSrc->height(), format );

    return m_image;
}

/*****************************************************************************/
StretchDiBitsRecord::StretchDiBitsRecord( QDataStream &stream ) :
    m_BmiSrc( 0 ), m_image( 0 )
{
    stream >> m_Bounds;
    stream >> m_xDest;
    stream >> m_yDest;
    stream >> m_xSrc;
    stream >> m_ySrc;
    stream >> m_cxSrc;
    stream >> m_cySrc;
    stream >> m_offBmiSrc;
    stream >> m_cbBmiSrc;
    stream >> m_offBitsSrc;
    stream >> m_cbBitsSrc;
    stream >> m_UsageSrc;
    stream >> m_BitBltRasterOperation;
    stream >> m_cxDest;
    stream >> m_cyDest;

    if ( m_cbBmiSrc == 40 ) {
        m_BmiSrc = new DeviceInfoHeader( stream );
    } else {
        Q_ASSERT( 0 );
    }
    m_imageData.resize( m_cbBitsSrc );
    stream.readRawData( m_imageData.data(), m_cbBitsSrc );
}

StretchDiBitsRecord::~StretchDiBitsRecord()
{
    delete m_image;
    // delete m_BmiSrc;
}

QRect StretchDiBitsRecord::bounds() const
{
    return m_Bounds;
}

QImage* StretchDiBitsRecord::image() 
{
    if ( m_image != 0 ) {
        kDebug(33100) << "null image";
        return m_image;
    }

    QImage::Format format = QImage::Format_Invalid;
    if ( m_BmiSrc->bitCount() == BI_BITCOUNT_4 ) {
        if ( m_BmiSrc->compression() == BI_RGB ) {
            format = QImage::Format_RGB555;
        } else {
            kDebug(33100) << "Unexpected compression format for BI_BITCOUNT_4:"
                     << m_BmiSrc->compression();
            Q_ASSERT( 0 );
        }
    } else if ( m_BmiSrc->bitCount() == BI_BITCOUNT_5 ) {
        format = QImage::Format_RGB888;
    } else {
        kDebug(33100) << "Unexpected format:" << m_BmiSrc->bitCount();
        Q_ASSERT( 0 );
    }

    m_image = new QImage( (const uchar*)m_imageData.constData(),
                          m_BmiSrc->width(), m_BmiSrc->height(), format );

    return m_image;
}

/*****************************************************************************/
ExtCreateFontIndirectWRecord::ExtCreateFontIndirectWRecord( QDataStream &stream, quint32 size )
{
    stream >> m_ihFonts;
    size -= 12;

    // TODO: Check size, we might need to do a LogFontExDv parse
    stream >> m_height;
    stream >> m_width;
    size -= 8;

    stream >> m_escapement;
    size -= 4;

    stream >> m_orientation;
    size -= 4;

    stream >> m_weight;
    size -= 4;

    stream >> m_italic;
    stream >> m_underline;
    stream >> m_strikeout;
    stream >> m_charSet;
    size -= 4;

    stream >> m_outPrecision;
    stream >> m_clipPrecision;
    stream >> m_quality;
    stream >> m_pitchAndFamily;
    size -= 4;

    QChar myChar[64];
    for ( int i = 0; i < 32; ++i ) {
	stream >> myChar[i];
    }
    size -= 64;

    for ( int i = 0; i < 32; ++i ) {
	if ( ! myChar[i].isNull() ) {
	    m_facename.append( myChar[i] );
	}
    }

#if 0
    for ( int i = 0; i < 64; ++i ) {
	stream >> myChar[i];
    }
    size -= 128;

    for ( int i = 0; i < 64; ++i ) {
	if ( ! myChar[i].isNull() ) {
	    m_fullName.append( myChar[i] );
	}
    }
    kDebug(33100) << "fullName:" << m_fullName;

    for ( int i = 0; i < 32; ++i ) {
	stream >> myChar[i];
    }
    size -= 64;
    for ( int i = 0; i < 32; ++i ) {
	if ( ! myChar[i].isNull() ) {
	    m_style.append( myChar[i] );
	}
    }
    kDebug(33100) << "style:" << m_style;

    for ( int i = 0; i < 32; ++i ) {
	stream >> myChar[i];
    }
    size -= 64;
    for ( int i = 0; i < 32; ++i ) {
	if ( ! myChar[i].isNull() ) {
	    m_script.append( myChar[i] );
	}
    }
    kDebug(33100) << "script:" << m_script;
#endif
    soakBytes( stream, size ); // rest of the record.
}

ExtCreateFontIndirectWRecord::~ExtCreateFontIndirectWRecord()
{
}

void ExtCreateFontIndirectWRecord::soakBytes( QDataStream &stream, int numBytes )
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
    }
}

/*****************************************************************************/
EmrTextObject::EmrTextObject( QDataStream &stream, quint32 size, TextType textType )
{
    stream >> m_referencePoint;
    size -= 8;

    stream >> m_charCount;
    size -= 4;

    stream >> m_offString;
    size -= 4;
    // 36 bytes for the body of the parent structure (EMR_EXTTEXTOUTA or EMR_EXTTEXTOUTW)
    // then parts of the EmrText structure
    quint32 offString = m_offString - 36 - 8 - 4 - 4; 

    stream >> m_options;
    size -= 4;
    offString -= 4;

    stream >> m_rectangle;
    size -= 16;
    offString -= 16;

    stream >> m_offDx;
    size -= 4;
    offString -= 4;
    // as for offString. 36 bytes for parent, then the earlier parts of EmrText
    quint32 offDx = m_offDx - 36 - 8 - 4 - 4 - 4 - 16 - 4;

    soakBytes( stream, offString ); // skips over UndefinedSpace1.
    size -= offString;
    offDx -= offString;

    if ( textType ==  SixteenBitChars ) {
        m_textString = recordWChars( stream, m_charCount );
        size -= 2 * m_charCount;
        offDx -= 2 * m_charCount;
    } else {
        m_textString = recordChars( stream, m_charCount );
        size -= m_charCount;
        offDx -= m_charCount;
    }
    // TODO: parse the spacing array
    soakBytes( stream, size );
}

EmrTextObject::~EmrTextObject()
{
}

QPoint EmrTextObject::referencePoint() const
{
    return m_referencePoint;
}

QString EmrTextObject::textString() const
{
    return m_textString;
}

QString EmrTextObject::recordWChars( QDataStream &stream, int numChars )
{
    QString text;
    QChar myChar;
    for ( int i = 0; i < numChars; ++i ) {
        stream >> myChar;
        text.append( myChar );
    }
    return text;
}

QString EmrTextObject::recordChars( QDataStream &stream, int numChars )
{
    QString text;
    quint8 myChar;
    for ( int i = 0; i < numChars; ++i ) {
        stream >> myChar;
        text.append( QChar( myChar ) );
    }
    return text;
}

void EmrTextObject::soakBytes( QDataStream &stream, int numBytes )
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
    }
}

/*****************************************************************************/

ExtTextOutARecord::ExtTextOutARecord( QDataStream &stream, quint32 size )
{
    size -= 8;
    stream >> m_bounds;
    size -= 16;
    stream >> m_iGraphicsMode;
    size -= 4;
    stream >> m_exScale;
    size -= 4;
    stream >> m_eyScale;
    size -= 4;
    m_emrText = new EmrTextObject( stream, size, EmrTextObject::EightBitChars );
}

ExtTextOutARecord::~ExtTextOutARecord()
{
   delete m_emrText;
}

QPoint ExtTextOutARecord::referencePoint() const
{
    return m_emrText->referencePoint();
}

QString ExtTextOutARecord::textString() const
{
    return m_emrText->textString();
}

}
