/* This file is part of the Calligra project

  Copyright 2011 Inge Wallin <inge@lysator.liu.se>

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

#include "SvmStructs.h"

#include <QDataStream>


static void soakBytes( QDataStream &stream, int numBytes )
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
    }
}


namespace Libsvm
{

VersionCompat::VersionCompat()
    : version(0)
    , length(0)
{
}

VersionCompat::VersionCompat(QDataStream &stream)
{
    stream >> version;
    stream >> length;
}

QDataStream &operator>>(QDataStream &stream, VersionCompat &compat)
{
    stream >> compat.version;
    stream >> compat.length;

    return stream;
}


Fraction::Fraction()
    : numerator(1)
    , denominator(1)
{
}

Fraction::Fraction(QDataStream &stream)
{
    stream >> numerator;
    stream >> denominator;
}

QDataStream &operator>>(QDataStream &stream, Fraction &fract)
{
    stream >> fract.numerator;
    stream >> fract.denominator;

    return stream;
}


MapMode::MapMode()
    : version()
    , unit(0)
    , origin(0, 0)
    , scaleX()
    , scaleY()
    , isSimple(true)
{
}

MapMode::MapMode(QDataStream &stream)
{
    stream >> *this;
}

QDataStream &operator>>(QDataStream &stream, MapMode &mm)
{
    stream >> mm.version;
    stream >> mm.unit;
    stream >> mm.origin;
    stream >> mm.scaleX;
    stream >> mm.scaleY;
    stream >> mm.isSimple;         // FIXME: how many bytes?

    return stream;
}

SvmHeader::SvmHeader()
    : versionCompat()
    , compressionMode()
    , mapMode()
    , width(0)
    , height(0)
    , actionCount(0)
{
}

SvmHeader::SvmHeader(QDataStream &stream)
{
    stream >> *this;
}

QDataStream &operator>>(QDataStream &stream, SvmHeader &header)
{
    stream >> header.versionCompat;
    stream >> header.compressionMode;
    stream >> header.mapMode;
    stream >> header.width;
    stream >> header.height;
    stream >> header.actionCount;

    if (header.versionCompat.version > 1)
        soakBytes(stream, 1);

    return stream;
}


}
