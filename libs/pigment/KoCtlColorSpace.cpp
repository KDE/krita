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

#include "KoCtlColorSpace.h"

#include "colorprofiles/KoCtlColorProfile.h"

struct KoCtlColorSpace::Private
{
    KoCtlColorProfile* profile;
};

KoCtlColorSpace::KoCtlColorSpace(const QString &id, const QString &name, const KoColorSpace* fallBack, const KoCtlColorProfile* profile) : d(new Private)
{
    d->profile = 0;//static_cast<KoCTLColorProfile*>(profile->clone());
}

KoCtlColorSpace::~KoCtlColorSpace()
{
    delete d;
}

bool KoCtlColorSpace::profileIsCompatible(const KoColorProfile* profile) const
{
    return ( dynamic_cast<const KoCtlColorProfile*>(profile) );
}

bool KoCtlColorSpace::hasHighDynamicRange() const
{
    return false;
}

const KoColorProfile * KoCtlColorSpace::profile() const
{
    return d->profile;
}

KoColorProfile * KoCtlColorSpace::profile()
{
    return d->profile;
}

KoColorTransformation *KoCtlColorSpace::createBrightnessContrastAdjustment(const quint16 *transferValues) const
{
    return 0;
}

KoColorTransformation *KoCtlColorSpace::createDesaturateAdjustment() const
{
    return 0;
}

KoColorTransformation *KoCtlColorSpace::createPerChannelAdjustment(const quint16 * const* transferValues) const
{
    return 0;
}

KoColorTransformation *KoCtlColorSpace::createDarkenAdjustment(qint32 shade, bool compensate, double compensation) const
{
    return 0;
}

KoColorTransformation *KoCtlColorSpace::createInvertTransformation() const
{
    return 0;
}

quint8 KoCtlColorSpace::difference(const quint8* src1, const quint8* src2) const
{
    return 0;
}

void KoCtlColorSpace::fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile ) const
{
    
}

void KoCtlColorSpace::toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile ) const
{
    
}

QImage KoCtlColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   const KoColorProfile *  dstProfile, KoColorConversionTransformation::Intent renderingIntent ) const
{
    return QImage();
}

quint8 KoCtlColorSpace::intensity8(const quint8 * src) const
{
    return 0;
}

KoID KoCtlColorSpace::mathToolboxId() const
{
}

void KoCtlColorSpace::colorToXML( const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    
}
void KoCtlColorSpace::colorFromXML( quint8* pixel, const QDomElement& elt) const
{
    
}
