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
#include "Bitmap.h"

#include <KDebug>

namespace Libemf
{


/*****************************************************************************/


BitBltRecord::BitBltRecord( QDataStream &stream, quint32 recordSize )
    : m_bitmap(0)
{
    //kDebug(31000) << "stream position at the start: " << stream.device()->pos();
    //kDebug(31000) << "record size: " << recordSize;

    stream >> m_bounds;

    stream >> m_xDest;          // x, y of upper left corner of the destination.
    stream >> m_yDest;
    stream >> m_cxDest;         // width, height of the rectangle in logical coords.
    stream >> m_cyDest;
    //kDebug(31000) << "Destination" << m_xDest << m_yDest << m_cxDest << m_cyDest;

    stream >> m_BitBltRasterOperation;
    //kDebug(31000) << "bitblt raster operation:" << hex << m_BitBltRasterOperation << dec;

    stream >> m_xSrc;           // x, y of the source
    stream >> m_ySrc;
    //kDebug(31000) << "Source" << m_xSrc << m_ySrc;

    //kDebug(31000) << "position before the matrix: " << stream.device()->pos();
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    float M11, M12, M21, M22, Dx, Dy;
    stream >> M11;              // Transformation matrix
    stream >> M12;
    stream >> M21;
    stream >> M22;
    stream >> Dx;
    stream >> Dy;
    m_XFormSrc = QTransform( M11, M12, M21, M22, Dx, Dy );
    //kDebug(31000) << "Matrix" << m_XFormSrc;
    //kDebug(31000) << "position after the matrix: " << stream.device()->pos();

    stream >> m_red >> m_green >> m_blue >> m_reserved;
    //kDebug(31000) << "Background color" << m_red << m_green << m_blue << m_reserved;
    //kDebug(31000) << "position after background color: " << stream.device()->pos();

    stream >> m_UsageSrc;
    //kDebug(31000) << "Color table interpretation" << m_UsageSrc;

    stream >> m_offBmiSrc;      // Offset to start of bitmap header from start of record
    stream >> m_cbBmiSrc;       // Size of source bitmap header
    stream >> m_offBitsSrc;     // Offset to source bitmap from start of record
    stream >> m_cbBitsSrc;      // Size of source bitmap
#if 0
    kDebug(31000) << "header offset:" << m_offBmiSrc;
    kDebug(31000) << "header size:  " << m_cbBmiSrc;
    kDebug(31000) << "bitmap offset:" << m_offBitsSrc;
    kDebug(31000) << "bitmap size:  " << m_cbBitsSrc;
#endif

    //kDebug(31000) << "stream position before the image: " << stream.device()->pos();
    if (m_cbBmiSrc > 0) {
        m_bitmap = new Bitmap( stream, recordSize, 8 + 23 * 4, // header + 23 ints
                               m_offBmiSrc, m_cbBmiSrc,
                               m_offBitsSrc, m_cbBitsSrc );
    }

    //kDebug(31000) << "stream position at the end: " << stream.device()->pos();
}

BitBltRecord::~BitBltRecord()
{
}

bool BitBltRecord::hasImage() const
{
    return m_bitmap && m_bitmap->hasImage();
    //return ( ( m_cbBmiSrc != 0 ) && ( m_cbBitsSrc != 0 ) );
}

QImage BitBltRecord::image() 
{
    return m_bitmap->image();
#if 0
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
#endif
}

/*****************************************************************************/
StretchDiBitsRecord::StretchDiBitsRecord( QDataStream &stream, quint32 recordSize )
    : m_bitmap(0)
{
    kDebug(31000) << "stream position at the start: " << stream.device()->pos();
    kDebug(31000) << "recordSize =" << recordSize;

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

#if 1
    kDebug(31000) << "header offset:" << m_offBmiSrc;
    kDebug(31000) << "header size:  " << m_cbBmiSrc;
    kDebug(31000) << "bitmap offset:" << m_offBitsSrc;
    kDebug(31000) << "bitmap size:  " << m_cbBitsSrc;

    kDebug(31000) << "m_BitBltRasterOperation =" << hex << m_BitBltRasterOperation << dec;
#endif

    kDebug(31000) << "stream position before the image: " << stream.device()->pos();
    if (m_cbBmiSrc > 0) {
        m_bitmap = new Bitmap( stream, recordSize, 8 + 18 * 4, // header + 18 ints
                               m_offBmiSrc, m_cbBmiSrc,
                               m_offBitsSrc, m_cbBitsSrc );
    }

    kDebug(31000) << "stream position at the end: " << stream.device()->pos();
#if 0
    // Read away those bytes that preceed the header.  These are undefined
    // according to the spec.  80 is the size of the record above.
    qint32 dummy;
    int    padding = 0;
    while (m_offBmiSrc - padding > 80) {
        stream >> dummy;
        padding += 4;
    }
    m_BmiSrc = new BitmapHeader( stream, m_cbBmiSrc );

    // 40 is the size of the header record.
    while (m_offBitsSrc - padding > 80 + 40) {
        stream >> dummy;
        padding += 4;
    }
    m_imageData.resize( m_cbBitsSrc );
    stream.readRawData( m_imageData.data(), m_cbBitsSrc );
#endif
}

StretchDiBitsRecord::~StretchDiBitsRecord()
{
    delete m_bitmap;
    //delete m_image;
    // delete m_BmiSrc;
}

QRect StretchDiBitsRecord::bounds() const
{
    return m_Bounds;
}

QImage StretchDiBitsRecord::image() 
{
    return m_bitmap->image();
#if 0
    if ( m_image != 0 ) {
        return m_image;
    }

    QImage::Format format = QImage::Format_Invalid;

    // Start by determining which QImage format we are going to use.
    if (m_BmiSrc->bitCount() == BI_BITCOUNT_1) {
        format = QImage::Format_Mono;
    } else if ( m_BmiSrc->bitCount() == BI_BITCOUNT_4 ) {
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
        kDebug(31000) << "Unexpected format:" << m_BmiSrc->bitCount();
        //Q_ASSERT(0);
    }

    // According to MS-WMF 2.2.2.3, the sign of the height decides if
    // this is a compressed bitmap or not.
    if (m_BmiSrc->height() > 0) {
        // This bitmap is a top-down bitmap without compression.
        m_image = new QImage( (const uchar*)m_imageData.constData(),
                              m_BmiSrc->width(), m_BmiSrc->height(), format );

        // The WMF images are in the BGR color order.
        if (format == QImage::Format_RGB888)
            *m_image = m_image->rgbSwapped();

        // We have to mirror this bitmap in the X axis since WMF images are stored bottom-up.
        *m_image = m_image->mirrored(false, true);
    } else {
        // This bitmap is a bottom-up bitmap which uses compression.
        switch (m_BmiSrc->compression()) {
        case BI_RGB:
            m_image = new QImage( (const uchar*)m_imageData.constData(),
                                  m_BmiSrc->width(), -m_BmiSrc->height(), format );
            // The WMF images are in the BGR color order.
            *m_image = m_image->rgbSwapped();
            break;

        // These compressions are not yet supported, so return an empty image.
        case BI_RLE8:
        case BI_RLE4:
        case BI_BITFIELDS:
        case BI_JPEG:
        case BI_PNG:
        case BI_CMYK:
        case BI_CMYKRLE8:
        case BI_CMYKRLE4:
        default:
            m_image = new QImage(m_BmiSrc->width(), m_BmiSrc->height(), format);
            break;
        }
    }

    return m_image;
#endif
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
