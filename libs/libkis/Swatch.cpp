/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Swatch.h"

#include <KisSwatch.h>
#include <KoColor.h>

struct Swatch::Private {
    KisSwatch swatch;
};

Swatch::Swatch(const KisSwatch &kisSwatch)
    : d(new Private)
{
    d->swatch = kisSwatch;
}

Swatch::Swatch()
    : d(new Private)
{

}

Swatch::~Swatch()
{
    delete d;
}

Swatch::Swatch(const Swatch &rhs)
    : d(new Private)
{
    d->swatch = rhs.d->swatch;
}

Swatch &Swatch::operator=(const Swatch &rhs)
{
    if (&rhs == this) return *this;
    d->swatch = rhs.d->swatch;
    return *this;
}

QString Swatch::name() const
{
    return d->swatch.name();
}

void Swatch::setName(const QString &name)
{
    d->swatch.setName(name);
}

QString Swatch::id() const
{
    return d->swatch.id();
}
void Swatch::setId(const QString &id)
{
    d->swatch.setId(id);
}

ManagedColor *Swatch::color() const
{
    ManagedColor *c = new ManagedColor(d->swatch.color());
    return c;
}
void Swatch::setColor(ManagedColor *color)
{
    d->swatch.setColor(color->color());
}

bool Swatch::spotColor() const
{
    return d->swatch.spotColor();
}
void Swatch::setSpotColor(bool spotColor)
{
    d->swatch.setSpotColor(spotColor);
}

bool Swatch::isValid() const
{
    return d->swatch.isValid();
}

KisSwatch Swatch::kisSwatch() const
{
    return d->swatch;
}
