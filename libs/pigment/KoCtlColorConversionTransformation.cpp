/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "KoCtlColorConversionTransformation.h"

#include <kdebug.h>

#include <GTLCore/Buffer.h>
#include <OpenCTL/Program.h>

#include "KoColorSpace.h"
#include "colorprofiles/KoCtlColorProfile.h"

#include <QString>

struct KoCtlColorConversionTransformation::Private {
    OpenCTL::Program* program;
};

KoCtlColorConversionTransformation::KoCtlColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs), d(new Private)
{
    kDebug() << "init KoCtlColorConversionTransformation " << srcCs->id() << " and " << dstCs->id();
    d->program = 0;
    const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>( srcCs->profile() );
    if(ctlp)
    {
        d->program = ctlp->createColorConversionProgram(srcCs, dstCs);
    }
    if( not d->program and (ctlp = dynamic_cast<const KoCtlColorProfile*>( dstCs->profile() )))
    {
        d->program = ctlp->createColorConversionProgram(srcCs, dstCs);
    }
    Q_ASSERT(d->program);
}

KoCtlColorConversionTransformation::~KoCtlColorConversionTransformation()
{
    delete d;
}

class KoCtlBuffer : public GTLCore::Buffer {
    public:
        KoCtlBuffer(char* _buffer, int _size) : m_buffer(_buffer), m_size(_size)
        {
        }
        virtual ~KoCtlBuffer() {}
        virtual char * rawData() { return m_buffer; }
        virtual const char * rawData() const { return m_buffer; }
        virtual int size() const  { return m_size; }
    private:
        char * m_buffer;
        int m_size;
};

void KoCtlColorConversionTransformation::transform(const quint8 *src8, quint8 *dst8, qint32 nPixels) const
{
    KoCtlBuffer src( reinterpret_cast<char*>(const_cast<quint8*>(src8) ), nPixels * srcColorSpace()->pixelSize());
    KoCtlBuffer dst( reinterpret_cast<char*>(dst8), nPixels * dstColorSpace()->pixelSize());
    d->program->apply(src, dst);
}

struct KoCtlColorConversionTransformationFactory::Private {
    
};

KoCtlColorConversionTransformationFactory::KoCtlColorConversionTransformationFactory(QString _srcModelId, QString _srcDepthId, QString _srcProfile, QString _dstModelId, QString _dstDepthId, QString _dstProfile) :
        KoColorConversionTransformationFactory(_srcModelId, _srcDepthId, _srcProfile, _dstModelId, _dstDepthId, _dstProfile), d(new Private)
{
}

KoColorConversionTransformation* KoCtlColorConversionTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent ) const
{
    Q_UNUSED(renderingIntent);
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));
    kDebug(/*DBG_PIGMENT*/) << "Creating transformation from " << srcColorSpace->id() << " to " << dstColorSpace->id();
    return new KoCtlColorConversionTransformation(srcColorSpace, dstColorSpace);
}

KoCtlColorConversionTransformationFactory::~KoCtlColorConversionTransformationFactory()
{
    delete d;
}

bool KoCtlColorConversionTransformationFactory::conserveColorInformation() const
{
    return true; // Assume colors are keeped, which is not necesseraly true
}
bool KoCtlColorConversionTransformationFactory::conserveDynamicRange() const
{
    return true; // Assume hdrness is kept, but it's wrong, it should be fixed Be conservative here for now, but a true value should be computed
}


