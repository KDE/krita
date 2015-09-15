/*
 * Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "KoResourcePaths.h"

#include <QGlobalStatic>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QStandardPaths>

// XXX: temporary!
#include <kstandarddirs.h>
#include <kglobal.h>

Q_GLOBAL_STATIC(KoResourcePaths, s_instance);


#ifdef Q_OS_WIN
static const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
static const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

class Q_DECL_HIDDEN KoResourcePaths::Private {
public:
    QMap<QString, QStringList> absolutes; // For each resource type, the list of absolute paths, from most local (most priority) to most global
    QMap<QString, QStringList> relatives; // Same with relative paths

    QStandardPaths::StandardLocation mapTypeToQStandardPaths(const QString &type) {
        if (type == "tmp") {
            return QStandardPaths::TempLocation;
        }
        else if (type == "appdata") {
            return QStandardPaths::DataLocation;
        }
        else if (type == "data") {
            return QStandardPaths::GenericDataLocation;
        }
        else if (type == "config") {
            return QStandardPaths::GenericConfigLocation;
        }
        else if (type == "cache") {
            return QStandardPaths::CacheLocation;
        }
        else if (type == "locale") {
            return QStandardPaths::GenericDataLocation;
        }
    }
};

KoResourcePaths::KoResourcePaths()
    : d(new Private)
{
}

KoResourcePaths::~KoResourcePaths()
{
}

void KoResourcePaths::addResourceType(const char *type, const char *basetype,
                                      const QString &relativeName, bool priority)
{
    s_instance->addResourceTypeInternal(QString::fromLatin1(type), QString::fromLatin1(basetype), relativeName, priority);
}

void KoResourcePaths::addResourceDir(const char *type, const QString &dir, bool priority)
{
    s_instance->addResourceDirInternal(QString::fromLatin1(type), dir, priority);
}

QString KoResourcePaths::findResource(const char *type, const QString &fileName)
{
    return s_instance->findResourceInternal(QString::fromLatin1(type), fileName);
}

QStringList KoResourcePaths::findDirs(const char *type, const QString &reldir)
{
    return s_instance->findDirsInternal(QString::fromLatin1(type), reldir);
}

QStringList KoResourcePaths::findAllResources(const char *type,
                                              const QString &filter,
                                              SearchOptions options)
{
    return s_instance->findAllResourcesInternal(QString::fromLatin1(type), filter, options);
}

QStringList KoResourcePaths::resourceDirs(const char *type)
{
    return s_instance->resourceDirsInternal(QString::fromLatin1(type));
}

QString KoResourcePaths::saveLocation(const char *type, const QString &suffix, bool create)
{
    return s_instance->saveLocationInternal(QString::fromLatin1(type), suffix, create);
}

QString KoResourcePaths::locate(const char *type, const QString &filename)
{
    return s_instance->locateInternal(QString::fromLatin1(type), filename);
}

QString KoResourcePaths::locateLocal(const char *type, const QString &filename, bool createDir)
{
    return s_instance->locateLocalInternal(QString::fromLatin1(type), filename, createDir);
}

void KoResourcePaths::addResourceTypeInternal(const QString &type, const QString &basetype,
                                              const QString &relativename,
                                              bool priority)
{
    if (relativename.isEmpty()) return;

    QString copy = relativename;
    if (!basetype.isNull()) {
        copy = QLatin1Char('%') + basetype + QLatin1Char('/');
        if (relativename != QLatin1String("/")) {
            copy += relativename;
        }
    }

    if (!copy.endsWith(QLatin1Char('/'))) {
        copy += QLatin1Char('/');
    }

    QStringList &rels = d->relatives[type]; // find or insert

    if (!rels.contains(copy, cs)) {
        if (priority) {
            rels.prepend(copy);
        } else {
            rels.append(copy);
        }
    }

    KGlobal::dirs()->addResourceType(type.toLatin1(), basetype.toLatin1(), relativename, priority);
}

void KoResourcePaths::addResourceDirInternal(const QString &type, const QString &absdir, bool priority)
{
    if (absdir.isEmpty() || type.isEmpty()) return;

    // find or insert entry in the map
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != QLatin1Char('/')) {
        copy += QLatin1Char('/');
    }

    QStringList &paths = d->absolutes[type];
    if (!paths.contains(copy, cs)) {
        if (priority) {
            paths.prepend(copy);
        } else {
            paths.append(copy);
        }
    }
    KGlobal::dirs()->addResourceDir(type.toLatin1(), absdir, priority);
}

QString KoResourcePaths::findResourceInternal(const QString &type, const QString &fileName)
{
    return KGlobal::dirs()->findResource(type.toLatin1(), fileName);
}

QStringList KoResourcePaths::findDirsInternal(const QString &type, const QString &relDir)
{
    return KGlobal::dirs()->findDirs(type.toLatin1(), relDir);
}

QStringList KoResourcePaths::findAllResourcesInternal(const QString &type,
                                                      const QString &filter,
                                                      SearchOptions options) const
{
    KStandardDirs::SearchOptions o;
    if (options == KoResourcePaths::NoSearchOptions) {
        o = KStandardDirs::NoSearchOptions;
    }
    else {
        if (options & KoResourcePaths::Recursive) {
            o |= KStandardDirs::Recursive;
        }
        if (options & KoResourcePaths::NoDuplicates) {
            o |= KStandardDirs::NoDuplicates;
        }
        if (options & KoResourcePaths::IgnoreExecBit) {
            o |= KStandardDirs::IgnoreExecBit;
        }
    }
    return KGlobal::dirs()->findAllResources(type.toLatin1(), filter, o);
}

QStringList KoResourcePaths::resourceDirsInternal(const QString &type)
{
    return KGlobal::dirs()->resourceDirs(type.toLatin1());
}

QString KoResourcePaths::saveLocationInternal(const QString &type, const QString &suffix, bool create)
{
    return KGlobal::dirs()->saveLocation(type.toLatin1(), suffix, create);
}

QString KoResourcePaths::locateInternal(const QString &type, const QString &filename)
{
    return KGlobal::dirs()->locate(type.toLatin1(), filename);
}

QString KoResourcePaths::locateLocalInternal(const QString &type, const QString &filename, bool createDir)
{
    return KGlobal::dirs()->locateLocal(type.toLatin1(), filename, createDir);
}
