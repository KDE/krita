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

#include <GTLCore/Buffer.h>
#include <GTLCore/Value.h>
#include <OpenCTL/Program.h>

#include "KoColorSpace.h"
#include "KoCtlColorProfile.h"
#include "DebugPigment.h"
#include "KoCtlBuffer.h"

#include <QString>

struct KoCtlColorConversionTransformation::Private {
    OpenCTL::Program* program;
    bool srcIsCTL;
};

KoCtlColorConversionTransformation::KoCtlColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs), d(new Private)
{
    dbgPigment << "init KoCtlColorConversionTransformation " << srcCs->id() << " and " << dstCs->id();
    d->program = 0;
    const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>(srcCs->profile());
    if (ctlp) {
        d->program = ctlp->createColorConversionProgram(srcCs, dstCs);
        d->srcIsCTL = true;
    }
    if (!d->program && (ctlp = dynamic_cast<const KoCtlColorProfile*>(dstCs->profile()))) {
        d->program = ctlp->createColorConversionProgram(srcCs, dstCs);
        d->srcIsCTL = false;
    }
    Q_ASSERT(d->program);
}

KoCtlColorConversionTransformation::~KoCtlColorConversionTransformation()
{
    delete d;
}

void KoCtlColorConversionTransformation::transform(const quint8 *src8, quint8 *dst8, qint32 nPixels) const
{
    dbgPigment << "Transformation from " << srcColorSpace()->id() << " " << srcColorSpace()->profile()->name() << " to " << dstColorSpace()->id() << " " << dstColorSpace()->profile()->name();
    KoCtlBuffer src(reinterpret_cast<char*>(const_cast<quint8*>(src8)), nPixels * srcColorSpace()->pixelSize());
    KoCtlBuffer dst(reinterpret_cast<char*>(dst8), nPixels * dstColorSpace()->pixelSize());
    const KoColorProfile* ctlp = d->srcIsCTL ? srcColorSpace()->profile() : dstColorSpace()->profile();
    for (std::list<GTLCore::String>::const_iterator cit = d->program->varyings().begin();
            cit != d->program->varyings().end(); ++cit) {
        QVariant v = ctlp->property(cit->c_str());
        dbgPigment << "Setting " << cit->c_str() << " to " << v;
        if (v.type() == QVariant::Double) {
            d->program->setVarying(*cit, (float)v.toDouble());
        } else if (v.type() == QVariant::Int) {
            d->program->setVarying(*cit, v.toInt());
        } else if (v.type() == QVariant::Bool) {
            d->program->setVarying(*cit, v.toBool());
        } else {
            dbgPigment << "Unsuitable type";
        }
    }
    d->program->apply(src, dst);
}

struct KoCtlColorConversionTransformationFactory::Private {

};

KoCtlColorConversionTransformationFactory::KoCtlColorConversionTransformationFactory(QString _srcModelId, QString _srcDepthId, QString _srcProfile, QString _dstModelId, QString _dstDepthId, QString _dstProfile) :
        KoColorConversionTransformationFactory(_srcModelId, _srcDepthId, _srcProfile, _dstModelId, _dstDepthId, _dstProfile), d(new Private)
{
}

KoColorConversionTransformation* KoCtlColorConversionTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    Q_UNUSED(renderingIntent);
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));
    dbgPigment << "Creating transformation from " << srcColorSpace->id() << " to " << dstColorSpace->id();
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


