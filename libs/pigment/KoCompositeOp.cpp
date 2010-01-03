/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include <klocale.h>

#include "KoCompositeOp.h"
#include "KoColorSpace.h"

QString KoCompositeOp::categoryMix()
{
    return i18n("Mix");
}
QString KoCompositeOp::categoryLight()
{
    return i18n("Light");
}
QString KoCompositeOp::categoryArithmetic()
{
    return i18n("Arithmetic");
}
QString KoCompositeOp::categoryColor()
{
    return i18n("Color");
}
QString KoCompositeOp::categoryMisc()
{
    return i18n("Misc");
}

struct KoCompositeOp::Private {
    const KoColorSpace * colorSpace;
    QString id;
    QString description;
    QString category;
    bool userVisible;
    QBitArray defaultChannelFlags;
};

KoCompositeOp::KoCompositeOp() : d(new Private)
{

}

KoCompositeOp::~KoCompositeOp()
{
    delete d;
}

KoCompositeOp::KoCompositeOp(const KoColorSpace * cs, const QString& id,  const QString& description, const QString & category, const bool userVisible)
        : d(new Private)
{
    d->colorSpace = cs;
    d->id = id;
    d->description = description;
    d->userVisible = userVisible;
    d->category = category;
    if (d->category.isEmpty()) {
        d->category = categoryMisc();
    }
}

void KoCompositeOp::composite(quint8 *dstRowStart, qint32 dstRowStride,
                              const quint8 *srcRowStart, qint32 srcRowStride,
                              const quint8 *maskRowStart, qint32 maskRowStride,
                              qint32 rows, qint32 numColumns,
                              quint8 opacity) const
{
    composite(dstRowStart, dstRowStride,
              srcRowStart, srcRowStride,
              maskRowStart, maskRowStride,
              rows, numColumns,
              opacity, d->defaultChannelFlags);
}

QString KoCompositeOp::category() const
{
    return d->category;
}

QString KoCompositeOp::id() const
{
    return d->id;
}

QString KoCompositeOp::description() const
{
    return d->description;
}

const KoColorSpace * KoCompositeOp::colorSpace() const
{
    return d->colorSpace;
}

bool KoCompositeOp::userVisible() const
{
    return d->userVisible;
}
