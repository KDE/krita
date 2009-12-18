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

#ifndef EMFRECORDS_H
#define EMFRECORDS_H

#include <QDataStream>
#include <QImage>
#include <QRect> // also provides QSize
#include <QString>

/**
   \file

   Primary definitions for EMF Records
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace Libemf
{

/*****************************************************************************/

/**
   Simple representation of a DeviceInfoHeader structure
   
   See MS-WMF 2.2.1.2 for details
*/
class DeviceInfoHeader 
{
public:
    /**
       Standard constructor

       The header structure is built from the specified stream.

       \param stream the data stream to read from.
    */
    DeviceInfoHeader( QDataStream &stream );
    ~DeviceInfoHeader();

    /**
       The width of the bitmap, in pixels
    */
    qint32 width() { return m_width; };

    /**
       The height of the bitmap, in pixels
    */
    qint32 height() { return m_height; };

    /**
       The number of bits that make up a pixel

       This is an enumerated type - see the BitCount enum
    */
    quint16 bitCount() { return m_bitCount; };

    /**
       The type of compression used in the image

       This is an enumerated type
    */
    quint32 compression() { return m_compression; };

private:
    quint32 m_headerSize;
    qint32 m_width;
    qint32 m_height;
    quint16 m_planes;
    quint16 m_bitCount;
    quint32 m_compression;
    quint32 m_imageSize;
    qint32 m_xPelsPerMeter;
    qint32 m_yPelsPerMeter;
    quint32 m_colorUsed;
    quint32 m_colorImportant;
};
 
/*****************************************************************************/

/** 
    Simple representation of an EMR_BITBLT record

    See MS-EMF Section 2.3.1.2 for details
*/
class BitBltRecord
{
public:
    /**
       Constructor for record type

       \param stream the stream to read the record structure from
    */
    BitBltRecord( QDataStream &stream );
    ~BitBltRecord();

   /**
       The X origin of the destination rectangle
    */
    qint32 xDest() const { return m_xDest; };

    /**
       The Y origin of the destination rectangle
    */
    qint32 yDest() const { return m_yDest; };

    /**
       The width of the destination rectangle
    */
    qint32 cxDest() const { return m_cxDest; };

    /**
       The height of the destination rectangle
    */
    qint32 cyDest() const { return m_cyDest; };

    /**
       The destination rectangle
    */
    QRect destinationRectangle() const { return QRect( xDest(), yDest(), cxDest(), cyDest() ); };

    /**
       The image to display
    */
    QImage* image();

    /**
       Whether there is a valid image in this BitBlt record
    */
    bool hasImage() const;

private:
    QRect m_Bounds;
    qint32 m_xDest;
    qint32 m_yDest;
    qint32 m_cxDest;
    qint32 m_cyDest;
    quint32 m_BitBltRasterOperation;
    qint32 m_xSrc;
    qint32 m_ySrc;
    QMatrix m_XFormSrc;
    // ColorRef - BkColorSrc - elements below
    quint8 m_red;
    quint8 m_green;
    quint8 m_blue;
    quint8 m_reserved;
    quint32 m_UsageSrc;
    quint32 m_offBmiSrc;
    quint32 m_cbBmiSrc;
    quint32 m_offBitsSrc;
    quint32 m_cbBitsSrc;
    DeviceInfoHeader *m_BmiSrc;
    QByteArray m_imageData;

    QImage *m_image;
};

/*****************************************************************************/

/** 
    Simple representation of an EMR_STRETCHDIBITS record

    See MS-EMF Section 2.3.1.7 for details
*/
class StretchDiBitsRecord
{
public:
    /**
       Constructor for record type

       \param stream the stream to read the record structure from
    */
    StretchDiBitsRecord( QDataStream &stream );
    ~StretchDiBitsRecord();

    /**
       The bounds of the affected area, in device units
    */
    QRect bounds() const;

    /**
       The X origin of the destination rectangle
    */
    qint32 xDest() const { return m_xDest; };

    /**
       The Y origin of the destination rectangle
    */
    qint32 yDest() const { return m_yDest; };

    /**
       The width of the destination rectangle
    */
    qint32 cxDest() const { return m_cxDest; };

    /**
       The height of the destination rectangle
    */
    qint32 cyDest() const { return m_cyDest; };

    /**
       The destination rectangle
    */
    QRect destinationRectangle() const { return QRect( xDest(), yDest(), cxDest(), cyDest() ); };

    /**
       The X origin of the source rectangle
    */
    qint32 xSrc() const { return m_xSrc; };

    /**
       The Y origin of the source rectangle
    */
    qint32 ySrc() const { return m_ySrc; };

    /**
       The width of the source rectangle
    */
    qint32 cxSrc() const { return m_cxSrc; };

    /**
       The height of the source rectangle
    */
    qint32 cySrc() const { return m_cySrc; };

    /**
       The source rectangle
    */
    QRect sourceRectangle() const { return QRect( xSrc(), ySrc(), cxSrc(), cySrc() ); };

    /**
       The image to display
    */
    QImage* image();

private:
    QRect m_Bounds;
    qint32 m_xDest;
    qint32 m_yDest;
    qint32 m_xSrc;
    qint32 m_ySrc;
    qint32 m_cxSrc;
    qint32 m_cySrc;
    quint32 m_offBmiSrc;
    quint32 m_cbBmiSrc;
    quint32 m_offBitsSrc;
    quint32 m_cbBitsSrc;
    quint32 m_UsageSrc;
    quint32 m_BitBltRasterOperation;
    qint32 m_cxDest;
    qint32 m_cyDest;
    DeviceInfoHeader *m_BmiSrc;
    QByteArray m_imageData;

    QImage *m_image;
};

/*****************************************************************************/

/** 
    Simple representation of an EMR_EXTCREATEFONTINDIRECTW record

    See MS-EMF Section 2.3.7.8 for details
*/
class ExtCreateFontIndirectWRecord
{
public:
    /**
       Constructor for record type

       \param stream the stream to read the record structure from
       \param size the number of bytes in this record
    */
    ExtCreateFontIndirectWRecord( QDataStream &stream, quint32 size );
    ~ExtCreateFontIndirectWRecord();


    /**
       The font handle index
    */
    quint32 ihFonts() const { return m_ihFonts; };

    /**
       The height of the font
    */
    qint32 height() const { return m_height; };

    /**
       Whether this is a italic font
    */
    quint8 italic() const { return m_italic; };

    /**
       Whether this is a underlined font
    */
    quint8 underline() const { return m_underline; };

    /**
       The weight of this font
    */
    quint32 weight() const { return m_weight; };

    /**
       The name of the font face
    */
    QString fontFace() const { return m_facename; };

private:
    quint32 m_ihFonts;

    qint32 m_height;
    qint32 m_width;
    qint32 m_escapement;
    qint32 m_orientation;
    qint32 m_weight;
    quint8 m_italic;
    quint8 m_underline;
    quint8 m_strikeout;
    quint8 m_charSet;
    quint8 m_outPrecision;
    quint8 m_clipPrecision;
    quint8 m_quality;
    quint8 m_pitchAndFamily;
    QString m_facename;
    QString m_fullName;
    QString m_style;
    QString m_script;

    // Routine to throw away a specific number of bytes
    void soakBytes( QDataStream &stream, int numBytes );
};

/*****************************************************************************/

/** 
    Simple representation of an EmrText object

    See MS-EMF Section 2.2.4 for details
*/
class EmrTextObject
{
public:
    /**
      The type of text to read
      */
    enum TextType { EightBitChars, SixteenBitChars };
    
    /**
       Constructor for EmrText object

       \param stream the stream to read the record structure from
       \param size the number of bytes in this record
       \param textType whether the text is normal (EightBitChars) or wide
       characters (SixteenBitChars)
    */
    EmrTextObject( QDataStream &stream, quint32 size, TextType textType );
    ~EmrTextObject();
    
    /**
        The reference point for the text output
     */
    QPoint referencePoint() const;
    
    /**
        The text to be output
     */
    QString textString() const;

private:

    QPoint m_referencePoint;
    quint32 m_charCount;
    quint32 m_offString;
    quint32 m_options;
    QRect m_rectangle;
    quint32 m_offDx;
    QString m_textString;

    // Convenience function to handle a 2-byte wide char stream
    QString recordWChars( QDataStream &stream, int numChars );

    // Convenience function to handle a 1-byte wide char stream
    QString recordChars( QDataStream &stream, int numChars );

    // Routine to throw away a specific number of bytes
    void soakBytes( QDataStream &stream, int numBytes );
};

/*****************************************************************************/

/** 
    Simple representation of an EMR_EXTTEXTOUTA Record

    See MS-EMF Section 2.2.4 for details
*/
class ExtTextOutARecord
{
public:
    /**
       Constructor for EMR_EXTTEXTOUTA Record

       \param stream the stream to read the record structure from
       \param size the number of bytes in this record
    */
    ExtTextOutARecord( QDataStream &stream, quint32 size );
    ~ExtTextOutARecord();

    /**
        The reference point for the text output
     */
    QPoint referencePoint() const;
    
    /**
        The text to be output
     */
    QString textString() const;

private:
    QRect m_bounds;
    quint32 m_iGraphicsMode;
    float m_exScale;
    float m_eyScale;
    EmrTextObject *m_emrText;
};

}

#endif
