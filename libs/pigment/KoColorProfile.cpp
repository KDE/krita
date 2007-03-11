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
#include <cfloat>
#include <cmath>
#include <lcms.h>
#include <limits.h>

#include <QImage>
#include <QFile>

#include <kdebug.h>

#include "KoColorProfile.h"


#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#include <QX11Info>
#endif


class KoColorProfile::Private {
public:
    Private() : valid(false), suitableForOutput(false) { }

    cmsHPROFILE profile;
    icColorSpaceSignature colorSpaceSignature;
    icProfileClassSignature deviceClass;
    QString productName;
    QString productDescription;
    QString productInfo;
    QString manufacturer;

    QByteArray rawData;

    QString filename;
    bool valid;
    bool suitableForOutput;
};

KoColorProfile::KoColorProfile(const QByteArray& rawData)
    : d(new Private())
{
    d->rawData = rawData;
    d->profile = cmsOpenProfileFromMem((void*)rawData.constData(), (DWORD)rawData.size());
    init();
}

KoColorProfile::KoColorProfile(const QString& file)
    : d(new Private())
{
    d->filename = file;
}

KoColorProfile::KoColorProfile(const cmsHPROFILE profile)
    : d(new Private())
{
    d->profile = profile;
    d->valid = true;

    size_t  bytesNeeded=0;

    // Make a raw data image ready for saving
    _cmsSaveProfileToMem(d->profile, 0, &bytesNeeded); // calc size
    d->rawData.resize(bytesNeeded);
    if(d->rawData.size() >= (int)bytesNeeded)
    {
        _cmsSaveProfileToMem(d->profile, d->rawData.data(), &bytesNeeded); // fill buffer
        cmsHPROFILE newprofile = cmsOpenProfileFromMem((void*)d->rawData.constData(), (DWORD) bytesNeeded);
        cmsCloseProfile(d->profile);
        d->profile = newprofile;
    }
    else
        d->rawData.resize(0);

    init();
}

KoColorProfile::~KoColorProfile()
{
    cmsCloseProfile(d->profile);
    delete d;
}


bool KoColorProfile::load()
{
    QFile file(d->filename);
    file.open(QIODevice::ReadOnly);
    d->rawData = file.readAll();
    d->profile = cmsOpenProfileFromMem((void*)d->rawData.constData(), (DWORD)d->rawData.size());
    file.close();

    if (d->profile == 0) {
        kWarning() << "Failed to load profile from " << d->filename << endl;
    }

    return init();

}

bool KoColorProfile::init()
{
    if (d->profile) {
        d->colorSpaceSignature = cmsGetColorSpace(d->profile);
        d->deviceClass = cmsGetDeviceClass(d->profile);
        d->productName = cmsTakeProductName(d->profile);
        d->productDescription = cmsTakeProductDesc(d->profile);
        d->productInfo = cmsTakeProductInfo(d->profile);
        d->valid = true;

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

#if 0
    // XXX: It wasn't that easy to save a little memory: thsi gives an lcms error
        // Okay, we know enough. Free the memory; we'll load it again if needed.

        cmsCloseProfile(d->profile);
        d->profile = 0;

#endif
        return true;
    }
    return false;
}

cmsHPROFILE KoColorProfile::profile()
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

bool KoColorProfile::save()
{
    return false;
}


KoColorProfile *  KoColorProfile::getScreenProfile (int screen)
{

#ifdef Q_WS_X11

    Atom type;
    int format;
    unsigned long nitems;
    unsigned long bytes_after;
    quint8 * str;

    static Atom icc_atom = XInternAtom( QX11Info::display(), "_ICC_PROFILE", False );

    if  ( XGetWindowProperty ( QX11Info::display(),
                    QX11Info::appRootWindow( screen ),
                    icc_atom,
                    0,
                    INT_MAX,
                    False,
                    XA_CARDINAL,
                    &type,
                    &format,
                    &nitems,
                    &bytes_after,
                    (unsigned char **) &str)
                ) {

        QByteArray bytes (nitems, '\0');
        bytes = QByteArray::fromRawData((char*)str, (quint32)nitems);

        return new KoColorProfile(bytes);
    } else {
        return NULL;
    }
#else
    return NULL;

#endif
}


icColorSpaceSignature KoColorProfile::colorSpaceSignature() const {
    return d->colorSpaceSignature;
}

icProfileClassSignature KoColorProfile::deviceClass() const {
    return d->deviceClass;
}

QString KoColorProfile::productName() const {
    return d->productName;
}

QString KoColorProfile::productDescription() const {
    return d->productDescription;
}

QString KoColorProfile::productInfo() const {
    return d->productInfo;
}

QString KoColorProfile::manufacturer() const {
    return d->manufacturer;
}

bool KoColorProfile::valid() const {
    return d->valid;
}

bool KoColorProfile::isSuitableForOutput() {
    return d->suitableForOutput;
}

QString KoColorProfile::filename() const {
    return d->filename;
}

QByteArray KoColorProfile::rawData() const {
    return d->rawData;
}

bool operator==( const KoColorProfile & p1,  const KoColorProfile & p2 )
{
    return p1.d->profile == p2.d->profile;
}

