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

#ifndef SVMSTRUCT_H
#define SVMSTRUCT_H


#include <QtGlobal>
#include <QPoint>

class QDataStream;

/**
 * @file
 *
 * Structs used in various parts of SVM files.
 */

/**
   Namespace for StarView Metafile (SVM) classes
*/
namespace Libsvm
{

/**
 * Contains version and length of an action.
 */
struct VersionCompat {
    VersionCompat();
    VersionCompat(QDataStream &stream);

    quint16  version;
    quint32  length;
};

QDataStream &operator>>(QDataStream &stream, VersionCompat &compat);



struct Fraction {
    Fraction();
    Fraction(QDataStream &stream);

    quint32  numerator;
    quint32  denominator;
};

QDataStream &operator>>(QDataStream &stream, Fraction &fract);


struct MapMode {
    MapMode();
    MapMode(QDataStream &stream);

    VersionCompat  version;
    quint16        unit;
    QPoint         origin;
    Fraction       scaleX;
    Fraction       scaleY;
    bool           isSimple;
};

QDataStream &operator>>(QDataStream &stream, MapMode &mm);


/**
 * The header of an SVM file.
 */
struct SvmHeader {
    SvmHeader();
    SvmHeader(QDataStream &stream);

    VersionCompat  versionCompat;
    quint32        compressionMode;
    MapMode        mapMode;
    quint32        width;
    quint32        height;
    quint32        actionCount;
};

QDataStream &operator>>(QDataStream &stream, SvmHeader &header);


}

#endif
