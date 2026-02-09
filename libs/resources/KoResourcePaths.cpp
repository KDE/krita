/*
 * SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
#include <QCoreApplication>
#include <QMutex>
#include <QRegularExpression>
#include "kis_debug.h"
#include "ksharedconfig.h"
#include "kconfiggroup.h"
#include "KisResourceLocator.h"
#include "KisWindowsPackageUtils.h"

Q_GLOBAL_STATIC(KoResourcePaths, s_instance)

QString KoResourcePaths::s_overrideAppDataLocation;

namespace {

static QString cleanup(const QString &path)
{
    return QDir::cleanPath(path);
}


static QStringList cleanup(const QStringList &pathList)
{
    QStringList cleanedPathList;

    bool getRidOfAppDataLocation = KoResourcePaths::getAppDataLocation() != QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString writableLocation = []() {
        QString location = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        // we have to ensure that the location has a trailing separator, because otherwise when we'll do startsWith
        // check it will skip paths that start have the same path but different directory name. E.g:
        // ~/.local/share/krita -> AppDataLocation
        // ~/.local/share/krita3 -> custom location, but this will be skipped in getRidOfAppDataLocation.
        if (location.back() == '/') {
            return location;
        } else {
            return QString(location + "/");
        }
    }();

     Q_FOREACH(const QString &path, pathList) {
        QString cleanPath = cleanup(path);
        if (getRidOfAppDataLocation && cleanPath.startsWith(writableLocation)) {
            continue;
        }
        cleanedPathList << cleanPath;
    }
    return cleanedPathList;
}

static QStringList cleanupDirs(const QStringList &pathList)
{
    QStringList cleanedPathList;

    bool getRidOfAppDataLocation = KoResourcePaths::getAppDataLocation() != QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    Q_FOREACH(const QString &path, pathList) {
        QString cleanPath = QDir::cleanPath(path) + '/';
        if (getRidOfAppDataLocation && cleanPath.startsWith(writableLocation)) {
            continue;
        }
        cleanedPathList << cleanPath;
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

#ifdef Q_OS_MACOS
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif

QString getInstallationPrefix() {
#ifdef Q_OS_MACOS
    QString appPath = qApp->applicationDirPath();

    dbgResources << "1" << appPath;
    appPath.chop(QString("MacOS/").length());
    dbgResources << "2" << appPath;

    bool makeInstall = QDir(appPath + "/../../../share/kritaplugins").exists();
    bool inBundle = QDir(appPath + "/Resources/kritaplugins").exists();

    QString bundlePath;

    if (inBundle) {
        bundlePath = appPath + "/";
    }
    else if (makeInstall) {
        appPath.chop(QString("Contents/").length());
        bundlePath = appPath + "/../../";
    }
    else {
        // This is needed as tests will not run outside of the
        // install directory without this
        // This needs krita to be installed.
        QString envInstallPath = qgetenv("KIS_TEST_PREFIX_PATH");
        if (!envInstallPath.isEmpty() && (
                    QDir(envInstallPath + "/share/kritaplugins").exists()
                    || QDir(envInstallPath + "/Resources/kritaplugins").exists() ))
        {
            bundlePath = envInstallPath;
        }
        else {
            qFatal("Cannot calculate the bundle path from the app path");
            qInfo() << "If running tests set KIS_TEST_PREFIX_PATH to krita install prefix";
        }
    }

    return bundlePath;
#elif defined(Q_OS_HAIKU)
	return qApp->applicationDirPath() + "/";
#elif defined(Q_OS_ANDROID)
    // qApp->applicationDirPath() isn't writable and android system won't allow
    // any files other than libraries
    // NOTE the subscript [1]. It points to the internal location.
    return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[1] + "/";
#else
    return qApp->applicationDirPath() + "/../";
#endif
}

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
        absolutesMutex.lock();
        if (absolutes.contains(type)) {
            a += absolutes[type];
        }
        absolutesMutex.unlock();

        return r + a;
    }

    QStandardPaths::StandardLocation mapTypeToQStandardPaths(const QString &type)
    {
        if (type == "appdata") {
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
        else if (type == "genericdata") {
            return QStandardPaths::GenericDataLocation;
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

QString KoResourcePaths::getAppDataLocation()
{
    if (!s_overrideAppDataLocation.isEmpty()) {
        return s_overrideAppDataLocation;
    }

    QString path;

    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    path = cfg.readEntry(KisResourceLocator::resourceLocationKey, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QFileInfo fi(path);

#if defined Q_OS_UNIX
    // Check that path is not a Windows path (e.g., "C:/", "D:/", etc.),
    // that can happen when a config file from Windows installation is
    // moved to a Linux system.
    QRegularExpression windowsPathPattern("^[A-Za-z]:/");
    if (windowsPathPattern.match(path).hasMatch()) {
        warnResources << "WARNING: KoResourcePaths::getAppDataLocation(): path appears to be a Windows path! Resetting to default..."
            << path
            << "->"
            << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        fi.setFile(path);
        cfg.writeEntry(KisResourceLocator::resourceLocationKey, path);
    }
#elif defined Q_OS_WIN
    // Check that path is not a Linux path, that can happen when a config
    // file from Linux installation is moved to a Windows system.
    QRegularExpression windowsPathPattern("^/[^/]");
    if (windowsPathPattern.match(path).hasMatch()) {
        warnResources << "WARNING: KoResourcePaths::getAppDataLocation(): path appears to be a Unix path! Resetting to default..."
            << path
            << "->"
            << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        fi.setFile(path);
        cfg.writeEntry(KisResourceLocator::resourceLocationKey, path);
    }
#endif /* Q_OS_UNIX */

    /**
     * Our code expects the resources location to be **absolute**. Otherwise functions like
     * makeStorageLocationRelative() or makeStorageLocationAbsolute() do not work.
     */
    if (fi.isRelative()) {
        warnResources << "WARNING: KoResourcePaths::getAppDataLocation(): resources location is not absolute! Fixing..." << path;
        path = fi.absoluteFilePath();
        fi.setFile(path);
        cfg.writeEntry(KisResourceLocator::resourceLocationKey, path);
    }

    // Check whether an existing location is writable
    if (fi.exists() && !fi.isWritable()) {
        path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    else if (!fi.exists()) {
        // Check whether a non-existing location can be created
        if (!QDir().mkpath(path)) {
            path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        }
        QDir().rmpath(path);
    }
    return path;


}

void KoResourcePaths::getAllUserResourceFoldersLocationsForWindowsStore(QString &standardLocation, QString &privateLocation)
{
    standardLocation = "";
    privateLocation = "";
    QString resourcePath = QDir(KisResourceLocator::instance()->resourceLocationBase()).absolutePath();
#ifndef Q_OS_WIN
    // not Windows, no problem
    standardLocation = resourcePath;
    return;
#else
    if (!KisWindowsPackageUtils::isRunningInPackage()) {
        standardLocation = resourcePath; // Windows, but not Windows Store, so no problem
        return;
    }

    // running inside Windows Store
    const QDir resourceDir(resourcePath);
    QDir appDataGeneralDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    appDataGeneralDir.cdUp();
    const QString appDataGeneralDirPath = appDataGeneralDir.path();
    if (resourceDir.absolutePath().contains(appDataGeneralDirPath, Qt::CaseInsensitive)) {
        // resource folder location is inside appdata, so it can cause issues
        // from inside of Krita, we can't determine whether it uses genuine %AppData% or the private Windows Store one
        // so, half of the time, a custom folder inside %AppData% wouldn't work
        // we can't fix that, we can only inform users about it or prevent them from choosing such folder
        // in any case, here we need to return both folders: inside normal appdata and the private one
        // (note that this case also handles the default resource folder called "krita" inside the appdata)


        const QString folderName = QFileInfo(resourcePath).fileName();

        const QString privateAppData = KisWindowsPackageUtils::getPackageRoamingAppDataLocation();
        const QDir privateResourceDir(QDir::fromNativeSeparators(privateAppData) + '/' + folderName);

        standardLocation = resourcePath;

        if (privateResourceDir.exists()) {
            privateLocation = privateResourceDir.absolutePath();
        }

        return;

    } else {
        standardLocation = resourcePath; // custom folder not inside AppData, so no problem (hopefully)
        return;
    }

#endif
}

void KoResourcePaths::addAssetType(const QString &type, const char *basetype,
                                      const QString &relativeName, bool priority)
{
    s_instance->addResourceTypeInternal(type, QString::fromLatin1(basetype), relativeName, priority);
}

void KoResourcePaths::addAssetDir(const QString &type, const QString &dir, bool priority)
{
    s_instance->addResourceDirInternal(type, dir, priority);
}

QString KoResourcePaths::findAsset(const QString &type, const QString &fileName)
{
    return cleanup(s_instance->findResourceInternal(type, fileName));
}

QStringList KoResourcePaths::findDirs(const QString &type)
{
    return cleanupDirs(s_instance->findDirsInternal(type));
}

QStringList KoResourcePaths::findAllAssets(const QString &type,
                                              const QString &filter,
                                              SearchOptions options)
{
    return cleanup(s_instance->findAllResourcesInternal(type, filter, options));
}

QStringList KoResourcePaths::assetDirs(const QString &type)
{
    return cleanupDirs(s_instance->resourceDirsInternal(type));
}

QString KoResourcePaths::saveLocation(const QString &type, const QString &suffix, bool create)
{
    return QDir::cleanPath(s_instance->saveLocationInternal(type, suffix, create)) + '/';
}

QString KoResourcePaths::locate(const QString &type, const QString &filename)
{
    return cleanup(s_instance->locateInternal(type, filename));
}

QString KoResourcePaths::locateLocal(const QString &type, const QString &filename, bool createDir)
{
    return cleanup(s_instance->locateLocalInternal(type, filename, createDir));
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
            if (QFile::exists(resource)) {
                break;
            }
        }
    }
    if (resource.isEmpty() || !QFile::exists(resource)) {
        QString approot = getApplicationRoot();
        Q_FOREACH (const QString &alias, aliases) {
            resource = approot + "/share/" + alias + '/' + fileName;
            if (QFile::exists(resource)) {
                break;
            }
        }
    }
    if (resource.isEmpty() || !QFile::exists(resource)) {
        QString approot = getApplicationRoot();
        Q_FOREACH (const QString &alias, aliases) {
            resource = approot + "/share/krita/" + alias + '/' + fileName;
            if (QFile::exists(resource)) {
                break;
            }
        }
    }

    if (resource.isEmpty() || !QFile::exists(resource)) {
        QStringList extraResourceDirs = findExtraResourceDirs();

        if (!extraResourceDirs.isEmpty()) {
            Q_FOREACH(const QString &extraResourceDir, extraResourceDirs) {
                if (aliases.isEmpty()) {
                    resource = extraResourceDir + '/' + fileName;
                    dbgResources<< "\t4" << resource;
                    if (QFile::exists(resource)) {
                        break;
                    }
                }
                else {
                    Q_FOREACH (const QString &alias, aliases) {
                        resource = extraResourceDir + '/' + alias + '/' + fileName;
                        dbgResources<< "\t4" << resource;
                        if (QFile::exists(resource)) {
                            break;
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

#ifdef Q_OS_MACOS
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

    QStringList saveLocationList;
    saveLocationList << saveLocation(type, QString(), true);
    appendResources(&dirs, saveLocationList, true);

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

    QStringList extraResourceDirs = findExtraResourceDirs();

    if (!extraResourceDirs.isEmpty()) {
        Q_FOREACH(const QString &extraResourceDir, extraResourceDirs) {
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
    QString path;

    bool useStandardLocation = false;
    const QStringList aliases = d->aliases(type);
    const QStandardPaths::StandardLocation location = d->mapTypeToQStandardPaths(type);

    if (location == QStandardPaths::AppDataLocation) {
        KConfigGroup cfg(KSharedConfig::openConfig(), "");
        path = cfg.readEntry(KisResourceLocator::resourceLocationKey, "");
    }

    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(location);
        useStandardLocation = true;
    }

#ifndef Q_OS_ANDROID
    // on Android almost all config locations we save to are app specific,
    // and don't end with "krita".
    if (!path.endsWith("krita") && useStandardLocation) {
        path += "/krita";
    }
#endif

    if (!aliases.isEmpty()) {
        path += '/' + aliases.first();
    } else {

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

QStringList KoResourcePaths::findExtraResourceDirs() const
{
    QStringList extraResourceDirs =
        QString::fromUtf8(qgetenv("EXTRA_RESOURCE_DIRS"))
            .split(';', Qt::SkipEmptyParts);

    const KConfigGroup cfg(KSharedConfig::openConfig(), "");
    const QString customPath =
        cfg.readEntry(KisResourceLocator::resourceLocationKey, "");
    if (!customPath.isEmpty()) {
        extraResourceDirs << customPath;
    }

    if (getAppDataLocation() != QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)) {
        extraResourceDirs << getAppDataLocation();
    }

    return extraResourceDirs;
}
