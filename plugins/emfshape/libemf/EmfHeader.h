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

#ifndef ENHHEADER_H
#define ENHHEADER_H

#include <QDataStream>
#include <QRect> // also provides QSize
#include <QString>

/**
   \file

   Primary definitions for EMF Header record
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace Libemf
{

/** 
    Simple representation of an EMF File header

    See MS-EMF Section 2.3.4.2 for details
*/
class Header
{
public:
    /**
       Constructor for header

       \param stream the stream to read the header structure from
    */
    Header( QDataStream &stream );
    ~Header();

    /**
       Check whether this is a valid EMF Header
     */
    bool isValid() const;

    /**
       The number of records in the metafile
     */
    quint32 recordCount() const;

    /**
       The bounds of the file content, in device units
    */
    QRect bounds() const;

private:
    // Temporary hack to read some bytes.
    void soakBytes( QDataStream &stream, int numBytes );

    quint32 mType;
    quint32 mSize;
    QRect mBounds;
    QRect mFrame;
    quint32 mSignature;
    quint32 mVersion;
    quint32 mBytes;
    quint32 mRecords;
    quint16 mHandles;
    quint16 mReserved;
    quint32 m_nDescription;
    quint32 m_offDescription;
    quint32 m_nPalEntries;
    QSize mDevice;      // this might need to be converted to something better
    QSize mMillimeters; // this might need to be converted to something better
};

}

#endif
