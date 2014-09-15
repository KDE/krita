/* This file is part of the KDE project
 * Copyright (c) 2014 Boudewijn Rempt (boud@valdyas.org)
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
#include "KoServiceLocator.h"

#include <QDebug>
#include <QDir>
#include <QApplication>
#include <QSet>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmimetype.h>

namespace {

    QStringList mimeTypes(KService::Ptr service) {

        QStringList ret;

        QStringList serviceTypes = service->serviceTypes();
        foreach (const QString &sv, serviceTypes) {
            if (KMimeType::mimeType(sv)) {
                ret << sv;
            }
        }
        return ret;
    }
}

struct KoServiceLocator::Private {

    QMap<QString, KService::List> typeToService;

};

KoServiceLocator *KoServiceLocator::instance()
{
    K_GLOBAL_STATIC(KoServiceLocator, s_instance);
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

KoServiceLocator::~KoServiceLocator()
{
}

KService::List KoServiceLocator::entries(const QString &type) const
{
    KService::List list;
    if (d->typeToService.contains(type)) {
        list = d->typeToService[type];
    }
    return list;
}

KoServiceLocator::KoServiceLocator()
    : d(new Private)
{
}

void KoServiceLocator::init()
{
    // services
    QSet<QString> servicesDirs = KGlobal::dirs()->findDirs("services", "").toSet();
    QList<QString> services;

    QDir servicesDir(qApp->applicationDirPath() + "/../share/kde4/services");
    servicesDirs << servicesDir.absolutePath();

    foreach(const QString &dir, servicesDirs) {
        QDir servicesDir = QDir(dir + "/calligra");
        servicesDir.setNameFilters(QStringList() << "*.desktop");
        if (servicesDir.exists()) {
            foreach(const QString &entry, servicesDir.entryList(QDir::Files)) {
                KService::Ptr service = KService::Ptr(new KService(servicesDir.absoluteFilePath(entry)));
                const QString key(service->library() + '\n' + service->name());
                if (!services.contains(key)) {
                    services << key;
                    foreach(const QString &t, service->serviceTypes()) {
                        if (!mimeTypes(service).contains(t)) {
                            if (!d->typeToService.contains(t)) {
                                d->typeToService[t] = KService::List();
                            }
                            d->typeToService[t].append(service);
                        }
                    }
                }
            }
        }
    }

    // Our applications are also services, but the desktop files are not in the services directory, but the apps directory
    QSet<QString> possibleAppLocations = KGlobal::dirs()->findDirs("apps", "").toSet();
    possibleAppLocations << QDir(qApp->applicationDirPath() + "/../share/applications/kde4/calligra").absolutePath();

    foreach(const QString &dir, possibleAppLocations) {
        QDir applicationsDir(dir);
        applicationsDir.setNameFilters(QStringList() << "*.desktop");
        if (applicationsDir.exists()) {
            foreach(const QString &entry, applicationsDir.entryList(QDir::Files)) {
                KService::Ptr service = KService::Ptr(new KService(applicationsDir.absoluteFilePath(entry)));
                if (!services.contains(service->name())) {
                    services << service->name();

                    if (service->property("X-KDE-ServiceTypes", QVariant::StringList).toStringList().contains("Calligra/Application")) {
                        if (!d->typeToService.contains("Calligra/Application")) {
                            d->typeToService["Calligra/Application"] = KService::List();
                        }
                        if (!d->typeToService["Calligra/Application"].contains(service)) {
                            d->typeToService["Calligra/Application"].append(service);
                        }
                    }
                }
            }
        }
    }

}
