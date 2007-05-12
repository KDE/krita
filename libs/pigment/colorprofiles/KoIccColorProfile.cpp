/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "KoIccColorProfile.h"

#include <limits.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#include <QX11Info>
#endif

struct KoIccColorProfile::Private
{
    QByteArray rawData;
};

KoIccColorProfile::KoIccColorProfile(QString fileName) : KoColorProfile(fileName), d(new Private)
{
    
}

KoIccColorProfile::KoIccColorProfile(const QByteArray& rawData) : KoColorProfile(""), d(new Private)
{
    setRawData(rawData);
}

KoIccColorProfile::~KoIccColorProfile()
{
    delete d;
}

QByteArray KoIccColorProfile::rawData() const {
    return d->rawData;
}

void KoIccColorProfile::setRawData(const QByteArray& rawData)
{
    d->rawData = rawData;
}

bool KoIccColorProfile::valid() const
{
    return true;
}

KoIccColorProfile *KoIccColorProfile::getScreenProfile(int screen )
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

        return new KoIccColorProfile(bytes);
    } else {
        return NULL;
    }
#else
    return NULL;

#endif
}

bool KoIccColorProfile::isSuitableForOutput() const
{
    return false;
}

bool KoIccColorProfile::isSuitableForPrinting() const
{
    return false;
}

bool KoIccColorProfile::isSuitableForDisplay() const
{
    return false;
}
