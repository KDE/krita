/*
    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

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

#include "KoAbstractGradient.h"
#include "KoColorSpaceRegistry.h"

struct KoAbstractGradient::Private {
    KoColorSpace* colorSpace;
    QGradient::Spread spread;
    QGradient::Type type;
};

KoAbstractGradient::KoAbstractGradient(const QString& filename)
    : KoResource(filename)
    , d(new Private)
{
    d->colorSpace = KoColorSpaceRegistry::instance()->rgb8();
}

KoAbstractGradient::~KoAbstractGradient()
{
   delete d;
}

void KoAbstractGradient::setColorSpace(KoColorSpace* colorSpace)
{
    d->colorSpace = colorSpace;
}

KoColorSpace* KoAbstractGradient::colorSpace() const
{
    return d->colorSpace;
}

void KoAbstractGradient::setSpread(QGradient::Spread spreadMethod)
{
    d->spread = spreadMethod;
}

QGradient::Spread KoAbstractGradient::spread () const
{
    return d->spread;
}

void KoAbstractGradient::setType(QGradient::Type repeatType)
{
    d->type = repeatType;
}

QGradient::Type KoAbstractGradient::type () const
{
    return d->type;
}

#include "KoAbstractGradient.moc"
