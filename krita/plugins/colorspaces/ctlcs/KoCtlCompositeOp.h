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

#ifndef _KO_CTL_COMPOSITE_OP_H_
#define _KO_CTL_COMPOSITE_OP_H_

#include <KoCompositeOp.h>

#include <string>

class KoCtlColorSpace;

namespace GTLCore
{
class PixelDescription;
}

namespace OpenCTL
{
class Program;
class Template;
}

class KoCTLCompositeOp : public KoCompositeOp
{
public:
    KoCTLCompositeOp(OpenCTL::Template* _template, const KoCtlColorSpace* cs, const GTLCore::PixelDescription& _pd);
    virtual ~KoCTLCompositeOp();
public:
    using KoCompositeOp::composite;
    virtual void composite(quint8 *dstRowStart, qint32 dstRowStride,
                           const quint8 *srcRowStart, qint32 srcRowStride,
                           const quint8 *maskRowStart, qint32 maskRowStride,
                           qint32 rows, qint32 numColumns,
                           quint8 opacity,
                           const QBitArray & channelFlags) const;
    bool isValid() const;
private:
    // Those three functions are hack and aren't supposed to stay
    QString idForFile(const std::string& _file);
    QString descriptionForFile(const std::string& _file);
    QString categoryForFile(const std::string& _file);
private:
    OpenCTL::Program* m_withMaskProgram;
    OpenCTL::Program* m_withoutMaskProgram;
};

#endif
