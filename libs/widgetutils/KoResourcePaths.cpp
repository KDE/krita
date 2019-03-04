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
#include <QApplication>
#include <QMutex>
#include "kis_debug.h"


Q_GLOBAL_STATIC(KoResourcePaths, s_instance)

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

void appendResources(QStringList *dst, const QStringList &src, bool eliminateDuplicates)
{
    Q_FOREACH (const QString &resource, src) {
        QString realPath = QDir::cleanPath(resource);
        if (!eliminateDuplicates || !dst->contains(realPath)) {
            *dst << realPath;
        }
    }
}


#ifdef Q_OS_WIN
static const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
static const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

#ifdef Q_OS_OSX
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif

QString getInstallationPrefix() {
#ifdef Q_OS_OSX
    QString appPath = qApp->applicationDirPath();

    dbgResources << "1" << appPath;
    appPath.chop(QString("MacOS/").length());
    dbgResources << "2" << appPath;

    bool makeInstall = QDir(appPath + "/../../../share/kritaplugins").exists();
    bool inBundle = QDir(appPath + "/Resources/kritaplugins").exists();

    dbgResources << "3. After make install" << makeInstall;
    dbgResources << "4. In Bundle" << inBundle;

    QString bundlePath;

    if (inBundle) {
        bundlePath = appPath + "/";
    }
    else if (makeInstall) {
        appPath.chop(QString("Contents/").length());
        bundlePath = appPath + "/../../";
    }
    else {
        qFatal("Cannot calculate the bundle path from the app path");
    }

    dbgResources << ">>>>>>>>>>>" << bundlePath;
    return bundlePath;
#else
#ifdef Q_OS_QWIN
    QDir appdir(qApp->applicationDirPath());

    // Corrects for mismatched case errors in path (qtdeclarative fails to load)
    wchar_t buffer[1024];
    QString absolute = appdir.absolutePath();
    DWORD rv = ::GetShortPathName((wchar_t*)absolute.utf16(), buffer, 1024);
    rv = ::GetLongPathName(buffer, buffer, 1024);
    QString correctedPath((QChar *)buffer);
    appdir.setPath(correctedPath);
    appdir.cdUp();
    return appdir.canonicalPath();
#else
    return qApp->applicationDirPath() + "/../";
#endif
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
        dbgResources << "\trelatives" << r;
        absolutesMutex.lock();
        if (absolutes.contains(type)) {
            a += absolutes[type];
        }
        dbgResources << "\tabsolutes" << a;
        absolutesMutex.unlock();

        return r + a;
    }

    QStandardPaths::StandardLocation mapTypeToQStandardPaths(const QString &type)
    {
        if (type == "tmp") {
            return QStandardPaths::TempLocation;
        }
        else if (type == "appdata") {
            return QStandardPaths::AppDataLocation;
        }
        else if (type == "data") {
            return QStandardPaths::AppDataLocation;
        }
        else if (type == "cache") {
            return QStandardPaths::CacheLocation;
        }
        else if (type == "locale") {
            return QStandardPaths::AppDataLocation;
        }
        else {
            return QStandardPaths::AppDataLocation;
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

QStringList KoResourcePaths::findDirs(const char *type)
{
    return cleanupDirs(s_instance->findDirsInternal(QString::fromLatin1(type)));
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

    dbgResources << "addResourceType: type" << type << "basetype" << basetype << "relativename" << relativename << "priority" << priority << d->relatives[type];
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

    dbgResources << "addResourceDir: type" << type << "absdir" << absdir << "priority" << priority << d->absolutes[type];
}

QString KoResourcePaths::findResourceInternal(const QString &type, const QString &fileName)
{
    QStringList aliases = d->aliases(type);
    dbgResources<< "aliases" << aliases << getApplicationRoot();
    QString resource = QStandardPaths::locate(QStandardPaths::AppDataLocation, fileName, QStandardPaths::LocateFile);

    if (resource.isEmpty()) {
        Q_FOREACH (const QString &alias, aliases) {
            resource = QStandardPaths::locate(d->mapTypeToQStandardPaths(type), alias + '/' + fileName, QStandardPaths::LocateFile);
            dbgResources << "\t1" << resource;
            if (QFile::exists(resource)) {
                continue;
            }
        }
    }
    if (resource.isEmpty() || !QFile::exists(resource)) {
        QString approot = getApplicationRoot();
        Q_FOREACH (const QString &alias, aliases) {
            resource = approot + "/share/" + alias + '/' + fileName;
            dbgResources << "\t2" << resource;

            if (QFile::exists(resource)) {
                continue;
            }
        }
    }
    if (resource.isEmpty() || !QFile::exists(resource)) {
        QString approot = getApplicationRoot();
        Q_FOREACH (const QString &alias, aliases) {
            resource = approot + "/share/krita/" + alias + '/' + fileName;
            dbgResources << "\t3" << resource;
            if (QFile::exists(resource)) {
                continue;
            }
        }
    }

    if (resource.isEmpty() || !QFile::exists(resource)) {
        QString extraResourceDirs = qgetenv("EXTRA_RESOURCE_DIRS");
        if (!extraResourceDirs.isEmpty()) {
            Q_FOREACH(const QString &extraResourceDir, extraResourceDirs.split(':', QString::SkipEmptyParts)) {
                if (aliases.isEmpty()) {
                    resource = extraResourceDir + '/' + fileName;
                    dbgResources<< "\t4" << resource;
                    if (QFile::exists(resource)) {
                        continue;
                    }
                }
                else {
                    Q_FOREACH (const QString &alias, aliases) {
                        resource = extraResourceDir + '/' + alias + '/' + fileName;
                        dbgResources<< "\t4" << resource;
                        if (QFile::exists(resource)) {
                            continue;
                        }
                    }
                }
            }
        }
    }

    dbgResources<< "findResource: type" << type << "filename" << fileName << "resource" << resource;
    Q_ASSERT(!resource.isEmpty());
    return resource;
}


QStringList filesInDir(const QString &startdir, const QString & filter, bool recursive)
{
    dbgResources << "filesInDir: startdir" << startdir << "filter" << filter << "recursive" << recursive;
    QStringList result;

    // First the entries in this path
    QStringList nameFilters;
    nameFilters << filter;
    const QStringList fileNames = QDir(startdir).entryList(nameFilters, QDir::Files | QDir::CaseSensitive, QDir::Name);
    dbgResources << "\tFound:" << fileNames.size() << ":" << fileNames;
    Q_FOREACH (const QString &fileName, fileNames) {
        QString file = startdir + '/' + fileName;
        result << file;
    }

    // And then everything underneath, if recursive is specified
    if (recursive) {
        const QStringList entries = QDir(startdir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString &subdir, entries) {
            dbgResources << "\tGoing to look in subdir" << subdir << "of" << startdir;
            result << filesInDir(startdir + '/' + subdir, filter, recursive);
        }
    }
    return result;
}

QStringList KoResourcePaths::findDirsInternal(const QString &type)
{
    QStringList aliases = d->aliases(type);
    dbgResources << type << aliases << d->mapTypeToQStandardPaths(type);

    QStringList dirs;
    QStringList standardDirs =
            QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), "", QStandardPaths::LocateDirectory);
    appendResources(&dirs, standardDirs, true);

    Q_FOREACH (const QString &alias, aliases) {
        QStringList aliasDirs =
                QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias + '/', QStandardPaths::LocateDirectory);
        appendResources(&dirs, aliasDirs, true);

#ifdef Q_OS_OSX
        dbgResources << "MAC:" << getApplicationRoot();
        QStringList bundlePaths;
        bundlePaths << getApplicationRoot() + "/share/krita/" + alias;
        bundlePaths << getApplicationRoot() + "/../share/krita/" + alias;
        dbgResources << "bundlePaths" << bundlePaths;
        appendResources(&dirs, bundlePaths, true);
        Q_ASSERT(!dirs.isEmpty());
#endif

        QStringList fallbackPaths;
        fallbackPaths << getApplicationRoot() + "/share/" + alias;
        fallbackPaths << getApplicationRoot() + "/share/krita/" + alias;
        appendResources(&dirs, fallbackPaths, true);

    }
    dbgResources << "findDirs: type" << type << "resource" << dirs;
    return dirs;
}


QStringList KoResourcePaths::findAllResourcesInternal(const QString &type,
                                                      const QString &_filter,
                                                      SearchOptions options) const
{
    dbgResources << "=====================================================";
    dbgResources << type << _filter << QStandardPaths::standardLocations(d->mapTypeToQStandardPaths(type));

    bool recursive = options & KoResourcePaths::Recursive;

    dbgResources << "findAllResources: type" << type << "filter" << _filter << "recursive" << recursive;

    QStringList aliases = d->aliases(type);
    QString filter = _filter;

    // In cases where the filter  is like "color-schemes/*.colors" instead of "*.kpp", used with unregistered resource types
    if (filter.indexOf('*') > 0) {
        aliases << filter.split('*').first();
        filter = '*' + filter.split('*')[1];
        dbgResources << "Split up alias" << aliases << "filter" << filter;
    }

    QStringList resources;
    if (aliases.isEmpty()) {
        QStringList standardResources =
                QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type),
                                          filter, QStandardPaths::LocateFile);
        dbgResources << "standardResources" << standardResources;
        appendResources(&resources, standardResources, true);
        dbgResources << "1" << resources;
    }


    QString extraResourceDirs = qgetenv("EXTRA_RESOURCE_DIRS");
    dbgResources << "extraResourceDirs" << extraResourceDirs;
    if (!extraResourceDirs.isEmpty()) {
        Q_FOREACH(const QString &extraResourceDir, extraResourceDirs.split(':', QString::SkipEmptyParts)) {
            if (aliases.isEmpty()) {
                appendResources(&resources, filesInDir(extraResourceDir + '/' + type, filter, recursive), true);
            }
            else {
                Q_FOREACH (const QString &alias, aliases) {
                    appendResources(&resources, filesInDir(extraResourceDir + '/' + alias + '/', filter, recursive), true);
                }
            }
        }

    }

    dbgResources << "\tresources from qstandardpaths:" << resources.size();

    Q_FOREACH (const QString &alias, aliases) {
        dbgResources << "\t\talias:" << alias;
        QStringList dirs;

        QFileInfo dirInfo(alias);
        if (dirInfo.exists() && dirInfo.isDir() && dirInfo.isAbsolute()) {
            dirs << alias;
        } else {
            dirs << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias, QStandardPaths::LocateDirectory)
                 << getInstallationPrefix() + "share/" + alias + "/"
                 << getInstallationPrefix() + "share/krita/" + alias + "/";
        }

        Q_FOREACH (const QString &dir, dirs) {
            appendResources(&resources,
                            filesInDir(dir, filter, recursive),
                            true);
        }
    }

    dbgResources << "\tresources also from aliases:" << resources.size();

    // if the original filter is "input/*", we only want share/input/* and share/krita/input/* here, but not
    // share/*. therefore, use _filter here instead of filter which was split into alias and "*".
    QFileInfo fi(_filter);

    QStringList prefixResources;
    prefixResources << filesInDir(getInstallationPrefix() + "share/" + fi.path(), fi.fileName(), false);
    prefixResources << filesInDir(getInstallationPrefix() + "share/krita/" + fi.path(), fi.fileName(), false);
    appendResources(&resources, prefixResources, true);

    dbgResources << "\tresources from installation:" << resources.size();
    dbgResources << "=====================================================";

    return resources;
}

QStringList KoResourcePaths::resourceDirsInternal(const QString &type)
{
    QStringList resourceDirs;
    QStringList aliases = d->aliases(type);

    Q_FOREACH (const QString &alias, aliases) {
        QStringList aliasDirs;

        aliasDirs << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias, QStandardPaths::LocateDirectory);

        aliasDirs << getInstallationPrefix() + "share/" + alias + "/"
                  << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias, QStandardPaths::LocateDirectory);
        aliasDirs << getInstallationPrefix() + "share/krita/" + alias + "/"
                  << QStandardPaths::locateAll(d->mapTypeToQStandardPaths(type), alias, QStandardPaths::LocateDirectory);

        appendResources(&resourceDirs, aliasDirs, true);
    }

    dbgResources << "resourceDirs: type" << type << resourceDirs;

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
        path = QStandardPaths::writableLocation(d->mapTypeToQStandardPaths(type));
        if (!path.endsWith("krita")) {
            path += "/krita";
        }
        if (!suffix.isEmpty()) {
            path += "/" + suffix;
        }
    }

    QDir d(path);

    if (!d.exists() && create) {
        d.mkpath(path);
    }
    dbgResources << "saveLocation: type" << type << "suffix" << suffix << "create" << create << "path" << path;

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
    dbgResources << "locate: type" << type << "filename" << filename << "locations" << locations;
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
    dbgResources << "locateLocal: type" << type << "filename" << filename << "CreateDir" << createDir << "path" << path;
    return path + '/' + filename;
}
