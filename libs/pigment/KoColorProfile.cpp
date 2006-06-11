/*
 *  kis_profile.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <config.h>
#include <lcms.h>
#include <limits.h>

#include <QImage>
#include <QTextStream>
#include <QFile>

#include <kdebug.h>

#include "KoColorProfile.h"


#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#include <QX11Info>
#endif


KoColorProfile::KoColorProfile(QByteArray rawData)
    : m_rawData(rawData),
      m_filename( QString() ),
      m_valid( false ),
      m_suitableForOutput(false)
{
    m_profile = cmsOpenProfileFromMem(rawData.data(), (DWORD)rawData.size());
    init();
}

KoColorProfile::KoColorProfile(const QString& file)
    : m_filename(file),
      m_valid( false ),
      m_suitableForOutput( false )
{
}

KoColorProfile::KoColorProfile(const cmsHPROFILE profile)
    : m_profile(profile),
      m_filename( QString() ),
      m_valid( true )
{
    size_t  bytesNeeded=0;

    // Make a raw data image ready for saving
    _cmsSaveProfileToMem(m_profile, 0, &bytesNeeded); // calc size
    m_rawData.resize(bytesNeeded);
    if(m_rawData.size() >= (int)bytesNeeded)
    {
        _cmsSaveProfileToMem(m_profile, m_rawData.data(), &bytesNeeded); // fill buffer
        cmsHPROFILE newprofile = cmsOpenProfileFromMem(m_rawData.data(), (DWORD) bytesNeeded);
        cmsCloseProfile(m_profile);
        m_profile = newprofile;
    }
    else
        m_rawData.resize(0);

    init();
}

KoColorProfile::~KoColorProfile()
{
    cmsCloseProfile(m_profile);
}


bool KoColorProfile::load()
{
    QFile file(m_filename);
    file.open(QIODevice::ReadOnly);
    m_rawData = file.readAll();
    m_profile = cmsOpenProfileFromMem(m_rawData.data(), (DWORD)m_rawData.size());
    file.close();

    if (m_profile == 0) {
        kWarning() << "Failed to load profile from " << m_filename << endl;
    }

    return init();

}

bool KoColorProfile::init()
{
    if (m_profile) {
        m_colorSpaceSignature = cmsGetColorSpace(m_profile);
        m_deviceClass = cmsGetDeviceClass(m_profile);
        m_productName = cmsTakeProductName(m_profile);
        m_productDescription = cmsTakeProductDesc(m_profile);
        m_productInfo = cmsTakeProductInfo(m_profile);
        m_valid = true;

        // Check if the profile can convert (something->this)
//         LPMATSHAPER OutMatShaper = cmsBuildOutputMatrixShaper(m_profile);
//         if( OutMatShaper )
//         {
//             m_suitableForOutput = true;
//         }
        cmsCIEXYZTRIPLE Primaries;

        if (cmsTakeColorants(&Primaries, m_profile))
        {
            m_suitableForOutput = true;
        }

#if 0
    // XXX: It wasn't that easy to save a little memory: thsi gives an lcms error
        // Okay, we know enough. Free the memory; we'll load it again if needed.

        cmsCloseProfile(m_profile);
        m_profile = 0;

#endif
        return true;
    }
    return false;
}

cmsHPROFILE KoColorProfile::profile()
{
#if 0
	if (m_profile = 0) {
	    QFile file(m_filename);
	    file.open(QIODevice::ReadOnly);
	    m_rawData = file.readAll();
	    m_profile = cmsOpenProfileFromMem(m_rawData.data(), (DWORD)m_rawData.size());
        file.close();
	}
#endif
	return m_profile;
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


