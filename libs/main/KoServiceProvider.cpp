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

QByteArray KoServiceProvider::readNativeFormatMimeType()   //static
{
    KService::Ptr service = readNativeService();
    if (!service)
        return QByteArray();

    if (service->property("X-KDE-NativeMimeType").toString().isEmpty()) {
        // It may be that the servicetype "Calligra/Part" is missing, which leads to this property not being known
        KServiceType::Ptr ptr = KServiceType::serviceType("Calligra/Part");
        if (!ptr)
            kError(30003) << "The serviceType Calligra/Part is missing. Check that you have a calligra_part.desktop file in the share/servicetypes directory." << endl;
        else {
            kWarning(30003) << service->entryPath() << ": no X-KDE-NativeMimeType entry!";
        }
    }

    return service->property("X-KDE-NativeMimeType").toString().toLatin1();
}


QStringList KoServiceProvider::readExtraNativeMimeTypes()   //static
{
    QStringList mimetypes;
    KService::Ptr service = readNativeService();
    if (service)
        mimetypes = service->property("X-KDE-ExtraNativeMimeTypes").toStringList();
    return mimetypes;
}

KService::Ptr KoServiceProvider::readNativeService()
{
    QString instname = QCoreApplication::applicationName();

    if (instname.isEmpty() && QCoreApplication::instance()) {
        instname = qAppName();
    }
    if (instname.isEmpty()) {
        instname = QString::fromLatin1("kde");
    }

    // The new way is: we look for a foopart.desktop in the kde_services dir.
    QString servicepartname = instname + "part.desktop";
    KService::Ptr service = KService::serviceByDesktopPath(servicepartname);

    Q_ASSERT(service);

    return service;
}
