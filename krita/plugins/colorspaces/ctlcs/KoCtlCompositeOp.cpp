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

#include "kis_debug.h"

#include "KoCtlColorSpace.h"
#include "KoCtlBuffer.h"
#include "KoCtlMutex.h"

#include <GTLCore/Value.h>
#include <OpenCTL/Program.h>
#include <OpenCTL/Module.h>

KoCTLCompositeOp::KoCTLCompositeOp(OpenCTL::Template* _template, const KoCtlColorSpace * cs, const GTLCore::PixelDescription& _pd) : KoCompositeOp(cs, idForFile(_template->fileName()), descriptionForFile(_template->fileName()), categoryForFile(_template->fileName())), m_withMaskProgram(0), m_withoutMaskProgram(0)
{
    QMutexLocker lock(ctlMutex);
    OpenCTL::Module* module = _template->generateModule(_pd);
    module->compile();
    if (module->isCompiled()) {
        std::list<GTLCore::PixelDescription> pdl;
        pdl.push_back(_pd);
        pdl.push_back(_pd);
        m_withoutMaskProgram = new OpenCTL::Program("compositeWithoutmask", module, pdl, _pd);
        if (!m_withoutMaskProgram->initialised()) {
            dbgPlugins << "Without mask failed";
            delete m_withoutMaskProgram;
            m_withoutMaskProgram = 0;
        }
        pdl.push_back(GTLCore::PixelDescription(GTLCore::Type::UnsignedInteger8, 1));
        m_withMaskProgram = new OpenCTL::Program("compositeWithmask", module, pdl, _pd);
        if (!m_withMaskProgram->initialised()) {
            dbgPlugins << "With mask failed";
            delete m_withMaskProgram;
            m_withMaskProgram = 0;
        }
        dbgPlugins << "m_withoutMaskProgram = " << m_withoutMaskProgram << " m_withMaskProgram = " << m_withMaskProgram;
    } else {
        dbgPlugins << "Composite op compilation failure";
    }
}


KoCTLCompositeOp::~KoCTLCompositeOp()
{
    delete m_withMaskProgram;
    delete m_withoutMaskProgram;
}

void KoCTLCompositeOp::composite(quint8 *dstRowStart, qint32 dstRowStride,
                                 const quint8 *srcRowStart, qint32 srcRowStride,
                                 const quint8 *maskRowStart, qint32 maskRowStride,
                                 qint32 rows, qint32 numColumns,
                                 quint8 opacity,
                                 const QBitArray & channelFlags) const
{
    Q_ASSERT(m_withMaskProgram);
    Q_ASSERT(m_withoutMaskProgram);
    while (rows > 0) {
        KoCtlBuffer src(reinterpret_cast<char*>(const_cast<quint8*>(srcRowStart)), numColumns * colorSpace()->pixelSize());
        KoCtlBuffer dst(reinterpret_cast<char*>(dstRowStart), numColumns * colorSpace()->pixelSize());
        std::list< GTLCore::Buffer* > ops;
        ops.push_back(&dst);
        ops.push_back(&src);
        if (maskRowStart) {
            KoCtlBuffer mask(reinterpret_cast<char*>(const_cast<quint8*>(maskRowStart)), numColumns * sizeof(quint8));
            ops.push_back(&mask);
            m_withMaskProgram->setVarying("opacity", GTLCore::Value(opacity));
            m_withMaskProgram->apply(ops, dst);
            maskRowStart += maskRowStride;
        } else {
            m_withoutMaskProgram->setVarying("opacity", GTLCore::Value(opacity));
            m_withoutMaskProgram->apply(ops, dst);
        }
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        --rows;
    }
}

bool KoCTLCompositeOp::isValid() const
{
    return m_withoutMaskProgram && m_withMaskProgram;
}

QString KoCTLCompositeOp::idForFile(const std::string& _file)
{
    QFileInfo fi(_file.c_str());
    QString basename = fi.baseName();
    if (basename == "over") {
        return COMPOSITE_OVER;
    }
    qFatal("No id for: %s", _file.c_str());
}

QString KoCTLCompositeOp::descriptionForFile(const std::string& _file)
{
    QFileInfo fi(_file.c_str());
    QString basename = fi.baseName();
    if (basename == "over") {
        return i18n("Normal");
    }
    qFatal("No description for: %s", _file.c_str());
}

QString KoCTLCompositeOp::categoryForFile(const std::string& _file)
{
    QFileInfo fi(_file.c_str());
    QString basename = fi.baseName();
    if (basename == "over") {
        return KoCompositeOp::categoryMix();
    }
    qFatal("No category for: %s", _file.c_str());
}
