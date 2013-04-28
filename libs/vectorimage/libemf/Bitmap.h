/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2010 Inge Wallin <inge@lysator.liu.se>

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

#ifndef EMFBITMAP_H
#define EMFBITMAP_H


#include <Qt>                   // For qint, etc.
#include <QByteArray>
#include <QImage>

#include "BitmapHeader.h"


class QDataStream;

/**
 * @file
 *
 * definitions for Device Independent Bitmaps, as used in both WMF and EMF files.
*/

/// Namespace for Enhanced Metafile (EMF) classes

namespace Libemf
{

/**
 * @class Bitmap
 *
 * Representation of a bitmap from an EMF file
 */
class Bitmap
{
public:
    /**
     * Constructor
     *
     * The Bitmap structure is built from the specified stream.
     *
     * \param stream the data stream to read from.
     * \param recordSize the size of the EMF record that this bitmap is part of.
     * \param usedBytes  number of already used bytes of the EMF record before the bitmap part
     * \param offBmiSrc  offset to start of bitmapheader
     * \param cbBmiSrc   size of bitmap header
     * \param offBitsSrc offset to source bitmap
     * \param cbBitsSrc  size of source bitmap
     */
    Bitmap( QDataStream &stream, 
            quint32 recordSize,  // total size of the EMF record
            quint32 usedBytes,   // used bytes of the EMF record before the bitmap part
            quint32 offBmiSrc,   // offset to start of bitmapheader
            quint32 cbBmiSrc,    // size of bitmap header
            quint32 offBitsSrc,  // offset to source bitmap
            quint32 cbBitsSrc);  // size of source bitmap
    ~Bitmap();

    /**
       The bitmap header
    */
    BitmapHeader *header() const { return m_header; };

    /**
       Return true if there is an image in this record.
     */
    bool hasImage() const { return m_hasImage; };

    /**
       The image.

       QImage shares its memory already.
    */
    QImage image();

private:
    // No copying for now, because we will get into trouble with the pointers.
    // The remedy is to write a real operator=() and Bitmap(Bitmap&).
    explicit Bitmap(Bitmap&);
    Bitmap &operator=(Bitmap&);

private:
    bool          m_hasImage;
    BitmapHeader *m_header;

    QByteArray    m_imageData;
    QImage        m_image;
    bool          m_imageIsValid;
};
 


} // namespace Libemf

#endif
