/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "KoServiceProvider.h"
#include <KoDocument.h>

#include <kservicetype.h>
#include <kdebug.h>

QByteArray KoServiceProvider::readNativeFormatMimeType(const KComponentData &componentData)   //static
{
    KService::Ptr service = readNativeService(componentData);
    if (!service)
        return QByteArray();

    if (service->property("X-KDE-NativeMimeType").toString().isEmpty()) {
        // It may be that the servicetype "CalligraPart" is missing, which leads to this property not being known
        KServiceType::Ptr ptr = KServiceType::serviceType("CalligraPart");
        if (!ptr)
            kError(30003) << "The serviceType CalligraPart is missing. Check that you have a calligrapart.desktop file in the share/servicetypes directory." << endl;
        else {
            kWarning(30003) << service->entryPath() << ": no X-KDE-NativeMimeType entry!";
        }
    }

    return service->property("X-KDE-NativeMimeType").toString().toLatin1();
}


QStringList KoServiceProvider::readExtraNativeMimeTypes(const KComponentData &componentData)   //static
{
    KService::Ptr service = readNativeService(componentData);
    if (!service)
        return QStringList();
    return service->property("X-KDE-ExtraNativeMimeTypes").toStringList();
}

KService::Ptr KoServiceProvider::readNativeService(const KComponentData &componentData)
{
    QString instname = componentData.isValid() ? componentData.componentName() : KGlobal::mainComponent().componentName();

    // The new way is: we look for a foopart.desktop in the kde_services dir.
    QString servicepartname = instname + "part.desktop";
    KService::Ptr service = KService::serviceByDesktopPath(servicepartname);
    if (service)
        kDebug(30003) << servicepartname << " found.";
    if (!service) {
        // The old way is kept as fallback for compatibility, but in theory this is really never used anymore.

        // Try by path first, so that we find the global one (which has the native mimetype)
        // even if the user created a words.desktop in ~/.kde/share/applnk or any subdir of it.
        // If he created it under ~/.kde/share/applnk/Office/ then no problem anyway.
        service = KService::serviceByDesktopPath(QString::fromLatin1("Office/%1.desktop").arg(instname));
    }
    if (!service)
        service = KService::serviceByDesktopName(instname);

    return service;
}
