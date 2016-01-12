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
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QSet>
#include <QApplication>
#include <QMutex>

Q_GLOBAL_STATIC(KoResourcePaths, s_instance);

static QString cleanup(const QString &path)
{
    return QDir::cleanPath(path);
}


static QStringList cleanup(const QStringList &pathList)
{
    QStringList cleanedPathList;
    Q_FOREACH(const QString &path, pathList) {
      cleanedPathList << cleanup(path);
    }
    return cleanedPathList;
}


static QString cleanupDirs(const QString &path)
{
  return QDir::cleanPath(path) + QDir::separator();
}

static QStringList cleanupDirs(const QStringList &pathList)
{
  QStringList cleanedPathList;
  Q_FOREACH(const QString &path, pathList) {
    cleanedPathList << cleanupDirs(path);
  }
  return cleanedPathList;
}


#ifdef Q_OS_WIN
static const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
static const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

#ifdef Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif

QString getInstallationPrefix() {
#ifdef Q_OS_MAC
     CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
     CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef,
                                            kCFURLPOSIXPathStyle);
     const char *pathPtr = CFStringGetCStringPtr(macPath,
                                            CFStringGetSystemEncoding());
     QString bundlePath = QString::fromLatin1(pathPtr);

//     qDebug() << "1" << bundlePath << (bundlePath + QString::fromLatin1("/Contents/MacOS/share"));
     if (QFile(bundlePath + QString::fromLatin1("/Contents/MacOS/share")).exists()) {
//         qDebug() << "running from a deployed bundle";
         bundlePath += QString::fromLatin1("/Contents/MacOS/");
     }
     else {
//         qDebug() << "running from make install";
         bundlePath += "/../";
     }

     CFRelease(appUrlRef);
     CFRelease(macPath);

     return bundlePath;
 #else
     return qApp->applicationDirPath() + QDir::separator();
 #endif
}

class Q_DECL_HIDDEN KoResourcePaths::Private {
public:
    QMap<QString, QStringList> absolutes; // For each resource type, the list of absolute paths, from most local (most priority) to most global
    QMap<QString, QStringList> relatives; // Same with relative paths

    QMutex relativesMutex;
    QMutex absolutesMutex;

    QStringList aliases(const QString &type)
    {
        QStringList r;
        QStringList a;
        relativesMutex.lock();
        if (relatives.contains(type)) {
            r += relatives[type];
        }
        relativesMutex.unlock();
        //qDebug() << "\trelatives" << r;
        absolutesMutex.lock();
        if (absolutes.contains(type)) {
            a += absolutes[type];
        }
        //qDebug() << "\tabsolutes" << a;
        absolutesMutex.unlock();

        return r + a;
    }

    QStandardPaths::StandardLocation mapTypeToQStandardPaths(const QString &type)
    {
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
        else {
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

QString KoResourcePaths::getApplicationRoot()
{
    return getInstallationPrefix();
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
    return cleanup(s_instance->findResourceInternal(QString::fromLatin1(type), fileName));
}

QStringList KoResourcePaths::findDirs(const char *type, const QString &reldir)
{
    return cleanupDirs(s_instance->findDirsInternal(QString::fromLatin1(type), reldir));
}

QStringList KoResourcePaths::findAllResources(const char *type,
                                              const QString &filter,
                                              SearchOptions options)
{
    return cleanup(s_instance->findAllResourcesInternal(QString::fromLatin1(type), filter, options));
}

QStringList KoResourcePaths::resourceDirs(const char *type)
{
    return cleanupDirs(s_instance->resourceDirsInternal(QString::fromLatin1(type)));
}

QString KoResourcePaths::saveLocation(const char *type, const QString &suffix, bool create)
{
    return cleanupDirs(s_instance->saveLocationInternal(QString::fromLatin1(type), suffix, create));
}

QString KoResourcePaths::locate(const char *type, const QString &filename)
{
    return cleanup(s_instance->locateInternal(QString::fromLatin1(type), filename));
}

QString KoResourcePaths::locateLocal(const char *type, const QString &filename, bool createDir)
{
    return cleanup(s_instance->locateLocalInternal(QString::fromLatin1(type), filename, createDir));
}

void KoResourcePaths::addResourceTypeInternal(const QString &type, const QString &basetype,
                                              const QString &relativename,
                                              bool priority)
{
    Q_UNUSED(basetype);
    if (relativename.isEmpty()) return;

    QString copy = relativename;

    Q_ASSERT(basetype == "data");

    if (!copy.endsWith(QLatin1Char('/'))) {
        copy += QLatin1Char('/');
    }

    d->relativesMutex.lock();
    QStringList &rels = d->relatives[type]; // find or insert

    if (!rels.contains(copy, cs)) {
        if (priority) {
            rels.prepend(copy);
        } else {
            rels.append(copy);
        }
    }
    d->relativesMutex.unlock();

    //qDebug() << "addResourceType: type" << type << "basetype" << basetype << "relativename" << relativename << "priority" << priority << d->relatives[type];
}

void KoResourcePaths::addResourceDirInternal(const QString &type, const QString &absdir, bool priority)
{
    if (absdir.isEmpty() || type.isEmpty()) return;

    // find or insert entry in the map
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != QLatin1Char('/')) {
        copy += QLatin1Char('/');
    }

    d->absolutesMutex.lock();
    QStringList &paths = d->absolutes[type];
    if (!paths.contains(copy, cs)) {
        if (priority) {
            paths.prepend(copy);
        } else {
            paths.append(copy);
        }
    }
    d->absolutesMutex.unlock();

    //qDebug() << "addResourceDir: type" << type << "absdir" << absdir << "priority" << priority << d->absolutes[type];
}

QString KoResourcePaths::findResourceInternal(const QString &type, const QString &fileName)
{
    QStringList aliases = d->aliases(type);

    QString resource = QStandardPaths::locate(d->mapTypeToQStandardPaths(type), fileName, QStandardPaths::LocateFile);
    if (resource.isEmpty()) {
        Q_FOREACH (const QString &alias, aliases) {
            resource = QStandardPaths::locate(d->mapTypeToQStandardPaths(type), alias + '/' + fileName, QStandardPaths::LocateFile);
            if (!resource.isEmpty()) {
                continue;
            }
        }
    }
    //Q_ASSERT(!resource.isEmpty());
    //qDebug() << "findResource: type" << type << "filename" << fileName << "resource" << resource;
    return resource;
}

QStringList KoResourcePaths::findDirsInternal(const QString &type, const QString &relDir)
{
    QStringList aliases = d->aliases(type);
    //qDebug() << type << aliases << d->mapTypeToQStandardPaths(type);

    QStringList dirs;

#ifdef Q_OS_MAC
    QString bundlePath = getApplicationRoot() + "/share/" + relDir;
    dirs << bundlePath;
    bundlePath = getApplicationRoot() + "/../share/" + relDir;
    dirs << bundlePath;
#endif

    dirs << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), relDir, QStandardPaths::LocateDirectory);

    Q_FOREACH (const QString &alias, aliases) {
        dirs << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias + '/' + relDir, QStandardPaths::LocateDirectory);
    }

    //Q_ASSERT(!dirs.isEmpty());
    //qDebug() << "findDirs: type" << type << "relDir" << relDir<< "resource" << dirs;
    return dirs;
}


QStringList filesInDir(const QString &startdir, const QString & filter, bool noduplicates, bool recursive)
{
    //qDebug() << "filesInDir: startdir" << startdir << "filter" << filter << "noduplicates" << noduplicates << "recursive" << recursive;
    QStringList result;

    // First the entries in this path
    QStringList nameFilters;
    nameFilters << filter;
    const QStringList fileNames = QDir(startdir).entryList(nameFilters, QDir::Files | QDir::CaseSensitive, QDir::Name);
    //qDebug() << "\tFound:" << fileNames.size() << ":" << fileNames;
    Q_FOREACH (const QString &fileName, fileNames) {
        QString file = startdir + '/' + fileName;
        result << file;
    }

    // And then everything underneath, if recursive is specified
    if (recursive) {
        const QStringList entries = QDir(startdir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString &subdir, entries) {
            //qDebug() << "\tGoing to look in subdir" << subdir << "of" << startdir;
            result << filesInDir(startdir + '/' + subdir, filter, noduplicates, recursive);
        }
    }
    return result;
}

QStringList KoResourcePaths::findAllResourcesInternal(const QString &type,
                                                      const QString &_filter,
                                                      SearchOptions options) const
{
//    qDebug() << "=====================================================";
//    qDebug() << type << _filter << QStandardPaths::standardLocations(d->mapTypeToQStandardPaths(type));
    bool noDuplicates = options & KoResourcePaths::NoDuplicates;
    bool recursive = options & KoResourcePaths::Recursive;

//    qDebug() << "findAllResources: type" << type << "filter" << _filter << "no dups" << noDuplicates << "recursive" << recursive;

    QStringList aliases = d->aliases(type);
    QString filter = _filter;

    // In cases where the filter  is like "color-schemes/*.colors" instead of "*.kpp", used with unregistered resource types
    if (filter.indexOf('*') > 0) {
        aliases << filter.split('*').first();
        filter = '*' + filter.split('*')[1];
        //qDebug() << "Split up alias" << aliases << "filter" << filter;
    }

    QStringList resources;
    if (aliases.isEmpty()) {
        resources << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), filter, QStandardPaths::LocateFile);
    }

//    qDebug() << "\tresources from qstandardpaths:" << resources.size();


    Q_FOREACH (const QString &alias, aliases) {
//        qDebug() << "\t\talias:" << alias;

        const QStringList dirs = QStringList() << getInstallationPrefix() + "../share/" + alias + "/"
                                               << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias, QStandardPaths::LocateDirectory);
        QSet<QString> s = QSet<QString>::fromList(dirs);

//        qDebug() << "\t\tdirs:" << dirs;
        Q_FOREACH (const QString &dir, s) {
            resources << filesInDir(dir, filter, noDuplicates, recursive);
        }
    }

//    qDebug() << "\tresources also from aliases:" << resources.size();

    if (resources.isEmpty()) {
        QFileInfo fi(filter);
        resources << filesInDir(getInstallationPrefix() + "../share/" + fi.path(), fi.fileName(), noDuplicates, false);
    }

//    qDebug() << "\tresources from installation:" << resources.size();

    if (noDuplicates) {
        QSet<QString> s = QSet<QString>::fromList(resources);
        resources = s.toList();
    }

//    qDebug() << "=====================================================";


    return resources;
}

QStringList KoResourcePaths::resourceDirsInternal(const QString &type)
{
    QStringList resourceDirs;
    QStringList aliases = d->aliases(type);

    Q_FOREACH (const QString &alias, aliases) {
        resourceDirs << getInstallationPrefix() + "../share/" + alias + "/"
                                               << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias, QStandardPaths::LocateDirectory);

        resourceDirs << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias, QStandardPaths::LocateDirectory);
    }

    //qDebug() << "resourceDirs: type" << type << resourceDirs;

    return resourceDirs;
}

QString KoResourcePaths::saveLocationInternal(const QString &type, const QString &suffix, bool create)
{
    QStringList aliases = d->aliases(type);
    QString path;
    if (aliases.size() > 0) {
        path = QStandardPaths::writableLocation(d->mapTypeToQStandardPaths(type)) + '/' + aliases.first();
    }
    else {
        path = QStandardPaths::writableLocation(d->mapTypeToQStandardPaths(type)) + '/' + (suffix.isEmpty() ? "krita" : suffix);
    }
    QDir d(path);

    if (!d.exists() && create) {
        d.mkpath(path);
    }
//    qDebug() << "saveLocation: type" << type << "suffix" << suffix << "create" << create << "path" << path;

    return path;
}

QString KoResourcePaths::locateInternal(const QString &type, const QString &filename)
{
    QStringList aliases = d->aliases(type);

    QStringList locations;
    if (aliases.isEmpty()) {
        locations << QStandardPaths::locate(d->mapTypeToQStandardPaths(type), filename, QStandardPaths::LocateFile);
    }

    Q_FOREACH (const QString &alias, aliases) {
        locations << QStandardPaths::locate(d->mapTypeToQStandardPaths(type),
                                            (alias.endsWith('/') ? alias : alias + '/') + filename, QStandardPaths::LocateFile);
    }
    //qDebug() << "locate: type" << type << "filename" << filename << "locations" << locations;
    if (locations.size() > 0) {
        return locations.first();
    }
    else {
        return "";
    }
}

QString KoResourcePaths::locateLocalInternal(const QString &type, const QString &filename, bool createDir)
{
    QString path = saveLocationInternal(type, "", createDir);
//    qDebug() << "locateLocal: type" << type << "filename" << filename << "CreateDir" << createDir << "path" << path;
    return path + '/' + filename;
}
