/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoFallBackColorTransformation.h"

#include "KoColorConversionTransformation.h"
#include "KoColorSpace.h"
#include "KoColorTransformation.h"
#include "KoColorConversionCache.h"
#include "KoColorSpaceRegistry.h"

#include "DebugPigment.h"

struct Q_DECL_HIDDEN KoFallBackColorTransformation::Private {
    const KoColorSpace* fallBackColorSpace;
    KoCachedColorConversionTransformation* csToFallBackCache;
    KoCachedColorConversionTransformation* fallBackToCsCache;
    const KoColorConversionTransformation* csToFallBack;
    const KoColorConversionTransformation* fallBackToCs;
    KoColorTransformation* colorTransformation;
    mutable quint8* buff;
    mutable qint32 buffSize;
};

KoFallBackColorTransformation::KoFallBackColorTransformation(const KoColorSpace* _cs, const KoColorSpace* _fallBackCS, KoColorTransformation* _transfo) : d(new Private)
{
    d->fallBackColorSpace = _fallBackCS;
    d->csToFallBackCache = new KoCachedColorConversionTransformation(KoColorSpaceRegistry::instance()->colorConversionCache()->cachedConverter(_cs, _fallBackCS, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags()));
    d->csToFallBack = d->csToFallBackCache->transformation();
    d->fallBackToCsCache = new KoCachedColorConversionTransformation(KoColorSpaceRegistry::instance()->colorConversionCache()->cachedConverter(_fallBackCS, _cs, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags()));
    d->fallBackToCs = d->fallBackToCsCache->transformation();
    d->colorTransformation = _transfo;
    d->buff = 0;
    d->buffSize = 0;
}

KoFallBackColorTransformation::KoFallBackColorTransformation(KoColorConversionTransformation* _csToFallBack, KoColorConversionTransformation* _fallBackToCs, KoColorTransformation* _transfo) : d(new Private)
{
    Q_ASSERT(*_csToFallBack->srcColorSpace() == *_fallBackToCs->dstColorSpace());
    Q_ASSERT(*_fallBackToCs->srcColorSpace() == *_csToFallBack->dstColorSpace());
    d->fallBackColorSpace = _fallBackToCs->srcColorSpace();
    d->csToFallBack = _csToFallBack;
    d->fallBackToCs = _fallBackToCs;
    d->csToFallBackCache = 0;
    d->fallBackToCsCache = 0;
    d->colorTransformation = _transfo;
    d->buff = 0;
    d->buffSize = 0;
}

KoFallBackColorTransformation::~KoFallBackColorTransformation()
{
    if (d->csToFallBackCache) {
        delete d->csToFallBackCache;
    } else {
        delete d->csToFallBack;
    }
    if (d->csToFallBackCache) {
        delete d->fallBackToCsCache;
    } else {
        delete d->fallBackToCs;
    }
    delete d->colorTransformation;
    delete[] d->buff;
    delete d;
}

void KoFallBackColorTransformation::transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    if (d->buffSize < nPixels) { // Expand the buffer if needed
        d->buffSize = nPixels;
        delete[] d->buff;
        d->buff = new quint8[ d->buffSize * d->fallBackColorSpace->pixelSize()];
    }
    d->csToFallBack->transform(src, d->buff, nPixels);
    d->colorTransformation->transform(d->buff, d->buff, nPixels);
    d->fallBackToCs->transform(d->buff, dst, nPixels);
}

QList<QString> KoFallBackColorTransformation::parameters() const
{
  return d->colorTransformation->parameters();
}

int KoFallBackColorTransformation::parameterId(const QString& name) const
{
  return d->colorTransformation->parameterId(name);
}

void KoFallBackColorTransformation::setParameter(int id, const QVariant& parameter)
{
  d->colorTransformation->setParameter(id, parameter);
}
