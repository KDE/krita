/*
 * This file is part of the KDE project
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoLcmsColorProfileContainer.h"

#include <cfloat>
#include <cmath>
#include <lcms.h>

#include <kdebug.h>

class KoLcmsColorProfileContainer::Private {
public:
    Private() : valid(false), suitableForOutput(false) { }

    cmsHPROFILE profile;
    icColorSpaceSignature colorSpaceSignature;
    icProfileClassSignature deviceClass;
    QString productDescription;
    QString productInfo;
    QString manufacturer;
    QString name;
    QString info;
    KoIccColorProfile::Data * data;
    bool valid;
    bool suitableForOutput;
};

KoLcmsColorProfileContainer::KoLcmsColorProfileContainer()
    : d(new Private())
{
    d->profile = 0;
}

KoLcmsColorProfileContainer::KoLcmsColorProfileContainer( KoIccColorProfile::Data * data)
    : d(new Private())
{
    d->data = data;
    d->profile = 0;
    init();
}

KoIccColorProfile* KoLcmsColorProfileContainer::createFromLcmsProfile(const cmsHPROFILE profile)
{
    size_t  bytesNeeded = 0;
    // Make a raw data image ready for saving
    _cmsSaveProfileToMem(profile, 0, &bytesNeeded); // calc size
    QByteArray rawData;
    rawData.resize(bytesNeeded);
    if(rawData.size() >= (int)bytesNeeded)
    {
        _cmsSaveProfileToMem(profile, rawData.data(), &bytesNeeded); // fill buffer
    }
    else
    {
        kError() << "Couldn't resize the profile buffer, system is probably running out of memory.";
        rawData.resize(0);
    }
    return new KoIccColorProfile(rawData);
}

KoLcmsColorProfileContainer::~KoLcmsColorProfileContainer()
{
    cmsCloseProfile(d->profile);
    delete d;
}

bool KoLcmsColorProfileContainer::init()
{
    if( d->profile ) cmsCloseProfile(d->profile);

    d->profile = cmsOpenProfileFromMem((void*)d->data->rawData().constData(), (DWORD)d->data->rawData().size());

    if (d->profile) {
        d->colorSpaceSignature = cmsGetColorSpace(d->profile);
        d->deviceClass = cmsGetDeviceClass(d->profile);
        d->productDescription = cmsTakeProductDesc(d->profile);
        d->productInfo = cmsTakeProductInfo(d->profile);
        d->valid = true;
        d->name = cmsTakeProductName(d->profile);
        d->info = d->productInfo;

        // Check if the profile can convert (something->this)
//         LPMATSHAPER OutMatShaper = cmsBuildOutputMatrixShaper(d->profile);
//         if( OutMatShaper )
//         {
//             d->suitableForOutput = true;
//         }
        cmsCIEXYZTRIPLE Primaries;

        if (cmsTakeColorants(&Primaries, d->profile))
        {
            d->suitableForOutput = true;
        }

        return true;
    }
    return false;
}

cmsHPROFILE KoLcmsColorProfileContainer::lcmsProfile()
{
#if 0
	if (d->profile = 0) {
	    QFile file(d->filename);
	    file.open(QIODevice::ReadOnly);
	    d->rawData = file.readAll();
	    d->profile = cmsOpenProfileFromMem((void*)d->rawData.constData(), (DWORD)d->rawData.size());
        file.close();
	}
#endif
	return d->profile;
}

icColorSpaceSignature KoLcmsColorProfileContainer::colorSpaceSignature() const {
    return d->colorSpaceSignature;
}

icProfileClassSignature KoLcmsColorProfileContainer::deviceClass() const {
    return d->deviceClass;
}

QString KoLcmsColorProfileContainer::productDescription() const {
    return d->productDescription;
}

QString KoLcmsColorProfileContainer::productInfo() const {
    return d->productInfo;
}

QString KoLcmsColorProfileContainer::manufacturer() const {
    return d->manufacturer;
}

bool KoLcmsColorProfileContainer::valid() const {
    return d->valid;
}

bool KoLcmsColorProfileContainer::isSuitableForOutput() const {
    return d->suitableForOutput;
}

bool KoLcmsColorProfileContainer::isSuitableForPrinting() const {
    return deviceClass() == icSigOutputClass;
}

bool KoLcmsColorProfileContainer::isSuitableForDisplay() const {
    return deviceClass() == icSigDisplayClass;
}

QString KoLcmsColorProfileContainer::name() const
{
    return d->name;
}
QString KoLcmsColorProfileContainer::info() const
{
    return d->info;
}
