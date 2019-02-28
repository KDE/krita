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
#ifndef KORESOURCEPATHS_H
#define KORESOURCEPATHS_H

#include <QScopedPointer>
#include <QString>
#include <QStringList>

#include <kritaresources_export.h>


/**
 * DEBUGGING KoResourcePaths:
 *
 * The usual place to look for resources is Qt's AppDataLocation.
 * This corresponds to XDG_DATA_DIRS on Linux. To ensure your installation and
 * path are configured correctly, ensure your files are located in the directories
 * contained in this variable:
 *
 * QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
 *
 * There are many debug lines that can be uncommented for more specific installation
 * checks. In the future these should be converted to qloggingcategory to enable
 * convenient enable/disable functionality.
 */
class KRITARESOURCES_EXPORT KoResourcePaths
{
public:

    KoResourcePaths();
    virtual ~KoResourcePaths();

    enum SearchOption { NoSearchOptions = 0,
                        Recursive = 1,
                        IgnoreExecBit = 4
                      };
    Q_DECLARE_FLAGS(SearchOptions, SearchOption)

    static QString getApplicationRoot();

    /**
     * Adds suffixes for types.
     *
     * You may add as many as you need, but it is advised that there
     * is exactly one to make writing definite.
     *
     * The later a suffix is added, the higher its priority. Note, that the
     * suffix should end with / but doesn't have to start with one (as prefixes
     * should end with one). So adding a suffix for app_pics would look
     * like KoStandardPaths::addResourceType("app_pics", "data", "app/pics");
     *
     * @param type Specifies a short descriptive string to access
     * files of this type.
     * @param basetype Specifies an already known type, or 0 if none
     * @param relativename Specifies a directory relative to the basetype
     * @param priority if true, the directory is added before any other,
     * otherwise after
     */
    static void addResourceType(const QString &type, const char *basetype,
                                const QString &relativeName, bool priority = true);


    /**
     * Adds absolute path at the beginning of the search path for
     * particular types (for example in case of icons where
     * the user specifies extra paths).
     *
     * You shouldn't need this function in 99% of all cases besides
     * adding user-given paths.
     *
     * @param type Specifies a short descriptive string to access files
     * of this type.
     * @param absdir Points to directory where to look for this specific
     * type. Non-existent directories may be saved but pruned.
     * @param priority if true, the directory is added before any other,
     * otherwise after
     */
    static void addResourceDir(const QString &type, const QString &dir, bool priority = true);

    /**
     * Tries to find a resource in the following order:
     * @li All PREFIX/\<relativename> paths (most recent first).
     * @li All absolute paths (most recent first).
     *
     * The filename should be a filename relative to the base dir
     * for resources. So is a way to get the path to libkdecore.la
     * to findResource("lib", "libkdecore.la"). KStandardDirs will
     * then look into the subdir lib of all elements of all prefixes
     * ($KDEDIRS) for a file libkdecore.la and return the path to
     * the first one it finds (e.g. /opt/kde/lib/libkdecore.la).
     *
     * Example:
     * @code
     * QString iconfilename = KStandardPaths::findResource("icon",QString("oxygen/22x22/apps/ktip.png"));
     * @endcode
     *
     * @param type The type of the wanted resource
     * @param filename A relative filename of the resource.
     *
     * @return A full path to the filename specified in the second
     *         argument, or QString() if not found.
     */

    static QString findResource(const QString &type, const QString &fileName);

    /**
     * Tries to find all directories whose names consist of the
     * specified type and a relative path. So
     * findDirs("xdgdata-apps", "Settings") would return
     * @li /home/joe/.local/share/applications/Settings/
     * @li /usr/share/applications/Settings/
     *
     * (from the most local to the most global)
     *
     * Note that it appends @c / to the end of the directories,
     * so you can use this right away as directory names.
     *
     * @param type The type of the base directory.
     * @param reldir Relative directory.
     *
     * @return A list of matching directories, or an empty
     *         list if the resource specified is not found.
     */
    static QStringList findDirs(const QString &type);

    /**
     * Tries to find all resources with the specified type.
     *
     * The function will look into all specified directories
     * and return all filenames in these directories.
     *
     * The "most local" files are returned before the "more global" files.
     *
     * @param type The type of resource to locate directories for.
     * @param filter Only accept filenames that fit to filter. The filter
     *        may consist of an optional directory and a QRegExp
     *        wildcard expression. E.g. <tt>"images\*.jpg"</tt>.
     *        Use QString() if you do not want a filter.
     * @param options if the flags passed include Recursive, subdirectories
     *        will also be search.
     *
     * @return List of all the files whose filename matches the
     *         specified filter.
     */
    static QStringList findAllResources(const QString &type,
                                        const QString &filter = QString(),
                                        SearchOptions options = NoSearchOptions);

    /**
     * @param type The type of resource
     * @return The list of possible directories for the specified @p type.
     * The function updates the cache if possible.  If the resource
     * type specified is unknown, it will return an empty list.
     * Note, that the directories are assured to exist beside the save
     * location, which may not exist, but is returned anyway.
     */
    static QStringList resourceDirs(const QString &type);

    /**
     * Finds a location to save files into for the given type
     * in the user's home directory.
     *
     * @param type The type of location to return.
     * @param suffix A subdirectory name.
     *             Makes it easier for you to create subdirectories.
     *   You can't pass filenames here, you _have_ to pass
     *       directory names only and add possible filename in
     *       that directory yourself. A directory name always has a
     *       trailing slash ('/').
     * @param create If set, saveLocation() will create the directories
     *        needed (including those given by @p suffix).
     *
     * @return A path where resources of the specified type should be
     *         saved, or QString() if the resource type is unknown.
     */
    static QString saveLocation(const QString &type, const QString &suffix = QString(), bool create = true);

    /**
     * This function is just for convenience. It simply calls
     * KoResourcePaths::findResource((type, filename).
     *
     * @param type   The type of the wanted resource, see KStandardDirs
     * @param filename   A relative filename of the resource
     *
     * @return A full path to the filename specified in the second
     *         argument, or QString() if not found
     **/
    static QString locate(const QString &type, const QString &filename);

    /**
     * This function is much like locate. However it returns a
     * filename suitable for writing to. No check is made if the
     * specified @p filename actually exists. Missing directories
     * are created. If @p filename is only a directory, without a
     * specific file, @p filename must have a trailing slash.
     *
     * @param type   The type of the wanted resource, see KStandardDirs
     * @param filename   A relative filename of the resource
     *
     * @return A full path to the filename specified in the second
     *         argument, or QString() if not found
     **/
    static QString locateLocal(const QString &type, const QString &filename, bool createDir = false);

private:

    void addResourceTypeInternal(const QString &type, const QString &basetype,
                                 const QString &relativeName, bool priority);

    void addResourceDirInternal(const QString &type, const QString &absdir, bool priority);

    QString findResourceInternal(const QString &type, const QString &fileName);

    QStringList findDirsInternal(const QString &type);

    QStringList findAllResourcesInternal(const QString &type,
                                         const QString &filter = QString(),
                                         SearchOptions options = NoSearchOptions) const;

    QStringList resourceDirsInternal(const QString &type);

    QString saveLocationInternal(const QString &type, const QString &suffix = QString(), bool create = true);

    QString locateInternal(const QString &type, const QString &filename);

    QString locateLocalInternal(const QString &type, const QString &filename, bool createDir = false);

    class Private;
    QScopedPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoResourcePaths::SearchOptions)

#endif // KORESOURCEPATHS_H
