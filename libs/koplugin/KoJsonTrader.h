/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     2007       David Faure <faure@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOJSONTRADER_H
#define KOJSONTRADER_H

#include <QList>
#include <QString>
#include "kritaplugin_export.h"

class QPluginLoader;

/**
 * Support class to fetch a list of relevant plugins
 *
 * Static singleton
 */
class KRITAPLUGIN_EXPORT KoJsonTrader
{
public:

    /**
     * Returns the instance of this class.
     */
    static KoJsonTrader *instance();

    /**
     * The main function in the KoJsonTrader class. It tries to automatically
     * locate the base path containing Krita plugins. It attempts to do so in
     * the current application directory qApp->applicationDirPath().
     *
     * The environment variable KRITA_PLUGIN_PATH overrides the automatic search
     * path when the algorithm is insufficient. Try setting this if the
     * "LittleCMS color management plugin is not installed" error appears.
     *
     * A better algorithm or another solution could be a welcome alternative.
     * One thing that might help would be to build all Krita plugins in a single
     * `plugins` folder, so that an installation step is unnecessary to put them
     * together in a single folder here. Another solution might be to construct
     * several QPluginLoaders.
     *
     * @param servicetype A service type like 'KMyApp/Plugin' or 'KFilePlugin'.
     * @param mimetype    A MimeType to constrain the search.
     *
     *
     * @return A list of QPluginLoader that satisfy the query
     */
     QList<QPluginLoader *> query(const QString &servicetype, const QString &mimetype);


     // Note: this should not be used
     KoJsonTrader();


     // Note: this should not be used
     KoJsonTrader();

private:
     QString m_pluginPath;
};

#endif
