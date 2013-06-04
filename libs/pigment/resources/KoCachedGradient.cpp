/*
    Copyright (C) 2005 Tim Beaulen <tbscope@gmail.org>
    Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (c) 2013 C. Boemann <cbo@boemann.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoCachedGradient.h"

#include <cfloat>

#include <QColor>

#include <klocale.h>
#include <kdebug.h>

#include "KoColorSpaceRegistry.h"

#include <KoColorModelStandardIds.h>

struct KoCachedGradient::Private {
    const KoAbstractGradient *subject;
    const KoColorSpace *colorSpace;
    qint32 max;
    KoColor *colors;
};

KoCachedGradient::KoCachedGradient(const KoAbstractGradient *subject, qint32 steps, const KoColorSpace *cs)
        : KoAbstractGradient(subject->filename())
        , d(new Private)
{
    d->subject = subject;
    d->max = steps-1;
    d->colors = new KoColor[steps];
    d->colorSpace = cs;

    KoColor tmpColor(d->colorSpace);
    for(qint32 i = 0; i < steps; i++) {
        d->subject->colorAt(tmpColor, qreal(i) / d->max);
        d->colors[i] = tmpColor;
    }
}

KoCachedGradient::~KoCachedGradient()
{
    delete[] d->colors;
}

QGradient* KoCachedGradient::toQGradient() const
{
    return d->subject->toQGradient();
}

quint8 *KoCachedGradient::cachedAt(qreal t) const
{
    qint32 tInt = t * d->max + 0.5;
    return d->colors[tInt].data();
}
