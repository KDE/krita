/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoMultipleColorConversionTransformation.h"

#include <QList>

#include <KoColorSpace.h>

struct Q_DECL_HIDDEN KoMultipleColorConversionTransformation::Private {
    QList<KoColorConversionTransformation*> transfos;
    quint32 maxPixelSize;
};


KoMultipleColorConversionTransformation::KoMultipleColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
    : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    , d(new Private)
{
    d->maxPixelSize = qMax(srcCs->pixelSize(), dstCs->pixelSize());
}
KoMultipleColorConversionTransformation::~KoMultipleColorConversionTransformation()
{
    Q_FOREACH (KoColorConversionTransformation* transfo, d->transfos) {
        delete transfo;
    }
    delete d;
}
void KoMultipleColorConversionTransformation::appendTransfo(KoColorConversionTransformation* transfo)
{
    d->transfos.append(transfo);
    d->maxPixelSize = qMax(d->maxPixelSize, transfo->srcColorSpace()->pixelSize());
    d->maxPixelSize = qMax(d->maxPixelSize, transfo->dstColorSpace()->pixelSize());
}
void KoMultipleColorConversionTransformation::transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    Q_ASSERT(d->transfos.size() > 1); // Be sure to have a more than one transformation
    quint8 *buff1 = new quint8[d->maxPixelSize*nPixels];
    quint8 *buff2 = 0;
    if (d->transfos.size() > 2) {
        buff2 = new quint8[d->maxPixelSize*nPixels]; // a second buffer is needed
    }
    d->transfos.first()->transform(src, buff1, nPixels);
    int lastIndex = d->transfos.size() - 2;
    for (int i = 1; i <= lastIndex; i++) {
        d->transfos[i]->transform(buff1, buff2, nPixels);
        quint8* tmp = buff1;
        buff1 = buff2;
        buff2 = tmp;
    }
    d->transfos.last()->transform(buff1, dst, nPixels);
    delete [] buff2;
    delete [] buff1;
}
