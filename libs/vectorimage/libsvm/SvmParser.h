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

#ifndef SVMPARSER_H
#define SVMPARSER_H


#include "SvmGraphicsContext.h"
#include "SvmAbstractBackend.h"
#include "kovectorimage_export.h"

class QByteArray;
class QDataStream;


namespace Libsvm
{


class KOVECTORIMAGE_EXPORT SvmParser
{
 public:
    SvmParser();

    void setBackend(SvmAbstractBackend *backend);

    bool parse(const QByteArray &data);

 private:
    void parseRect(QDataStream &stream, QRect &rect);
    void parsePolygon(QDataStream &stream, QPolygon &polygon);
    void parseString(QDataStream &stream, QString &string);
    void parseFont(QDataStream &stream, QFont &font);

    void dumpAction(QDataStream &stream, quint16 version, quint32 totalSize);

 private:
    SvmGraphicsContext   mContext;
    SvmAbstractBackend  *mBackend;
};


}

#endif
