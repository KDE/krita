/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoCtlCompositeOp.h"

#include <OpenCTL/Template.h>

#include <QFileInfo>

#include <klocale.h>

#include "KoCtlColorSpace.h"

KoCTLCompositeOp::KoCTLCompositeOp(OpenCTL::Template* _template, const KoCtlColorSpace * cs, const GTLCore::PixelDescription& _pd) : KoCompositeOp(cs, idForFile(_template->fileName()), descriptionForFile(_template->fileName()), categoryForFile(_template->fileName())), m_withMaskProgram(0), m_withoutMaskProgram(0)
{
  
}

void KoCTLCompositeOp::composite(quint8 *dstRowStart, qint32 dstRowStride,
            const quint8 *srcRowStart, qint32 srcRowStride,
            const quint8 *maskRowStart, qint32 maskRowStride,
            qint32 rows, qint32 numColumns,
            quint8 opacity,
            const QBitArray & channelFlags) const
{
}

bool KoCTLCompositeOp::isValid() const
{
  return m_withoutMaskProgram && m_withMaskProgram;
}

QString KoCTLCompositeOp::idForFile( const std::string& _file )
{
  QFileInfo fi(_file.c_str());
  QString basename = fi.baseName();
  if(basename == "over")
  {
    return COMPOSITE_OVER;
  }
  qFatal( "No id for: %s", _file.c_str());
}

QString KoCTLCompositeOp::descriptionForFile( const std::string& _file )
{
  QFileInfo fi(_file.c_str());
  QString basename = fi.baseName();
  if(basename == "over")
  {
    return i18n("Normal" );
  }
  qFatal( "No description for: %s", _file.c_str());
}

QString KoCTLCompositeOp::categoryForFile( const std::string& _file )
{
  QFileInfo fi(_file.c_str());
  QString basename = fi.baseName();
  if(basename == "over")
  {
    return KoCompositeOp::categoryMix();
  }
  qFatal( "No category for: %s", _file.c_str());
}
