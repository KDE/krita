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
#include "koplugin_export.h"

class QPluginLoader;

/**
 *  Support class to fetch a list of relevant plugins
 */
class KOPLUGIN_EXPORT KoJsonTrader
{

private:
    explicit KoJsonTrader();

public:
    static KoJsonTrader *self();

    /**
     * The main function in the KoJsonTrader class.
     *
     * It will return a list of QPluginLoader that match your
     * specifications.  The only required parameter is the service
     * type.  This is something like 'text/plain' or 'text/html'.  The
     * constraint parameter is used to limit the possible choices
     * returned based on the constraints you give it.
     *
     * The @p constraint language is rather full.  The most common
     * keywords are AND, OR, NOT, IN, and EXIST, all used in an
     * almost spoken-word form.  An example is:
     * \code
     * (Type == 'Service') and (('KParts/ReadOnlyPart' in ServiceTypes) or (exist Exec))
     * \endcode
     *
     * The keys used in the query (Type, ServiceType, Exec) are all
     * fields found in the .desktop files.
     *
     * @param servicetype A service type like 'KMyApp/Plugin' or 'KFilePlugin'.
     * @param constraint  A constraint to limit the choices returned, QString() to
     *                    get all services of the given @p servicetype
     *
     * @return A list of QPluginLoader that satisfy the query
     * @see http://techbase.kde.org/Development/Tutorials/Services/Traders#The_KTrader_Query_Language
     */
     QList<QPluginLoader *> query(const QString &servicetype, const QString &constraint);

private:
     KoJsonTrader *m_service;
     QString m_pluginPath;
};

#endif
