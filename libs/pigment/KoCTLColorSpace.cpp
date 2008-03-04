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

#include "KoCTLColorSpace.h"

#include "colorprofiles/KoCTLColorProfile.h"

struct KoCTLColorSpace::Private
{
    KoCTLColorProfile* profile;
};

KoCTLColorSpace::KoCTLColorSpace(const QString &id, const QString &name, const KoColorSpace* fallBack, const KoCTLColorProfile* profile) : d(new Private)
{
    d->profile = 0;//static_cast<KoCTLColorProfile*>(profile->clone());
}

KoCTLColorSpace::~KoCTLColorSpace()
{
    delete d;
}

bool KoCTLColorSpace::profileIsCompatible(const KoColorProfile* profile) const
{
    return ( dynamic_cast<const KoCTLColorProfile*>(profile) );
}

bool KoCTLColorSpace::hasHighDynamicRange() const
{
    return false;
}

const KoColorProfile * KoCTLColorSpace::profile() const
{
    return d->profile;
}

KoColorProfile * KoCTLColorSpace::profile()
{
    return d->profile;
}

KoColorTransformation *KoCTLColorSpace::createBrightnessContrastAdjustment(const quint16 *transferValues) const
{
    return 0;
}

KoColorTransformation *KoCTLColorSpace::createDesaturateAdjustment() const
{
    return 0;
}

KoColorTransformation *KoCTLColorSpace::createPerChannelAdjustment(const quint16 * const* transferValues) const
{
    return 0;
}

KoColorTransformation *KoCTLColorSpace::createDarkenAdjustment(qint32 shade, bool compensate, double compensation) const
{
    return 0;
}

KoColorTransformation *KoCTLColorSpace::createInvertTransformation() const
{
    return 0;
}

quint8 KoCTLColorSpace::difference(const quint8* src1, const quint8* src2) const
{
    return 0;
}

void KoCTLColorSpace::fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile ) const
{
    
}

void KoCTLColorSpace::toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile ) const
{
    
}

QImage KoCTLColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   const KoColorProfile *  dstProfile, KoColorConversionTransformation::Intent renderingIntent ) const
{
    return QImage();
}

quint8 KoCTLColorSpace::intensity8(const quint8 * src) const
{
    return 0;
}

KoID KoCTLColorSpace::mathToolboxId() const
{
}

void KoCTLColorSpace::colorToXML( const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    
}
void KoCTLColorSpace::colorFromXML( quint8* pixel, const QDomElement& elt) const
{
    
}
