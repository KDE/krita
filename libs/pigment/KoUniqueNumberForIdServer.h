/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_UNIQUE_NUMBER_FOR_ID_SERVER_H_
#define _KO_UNIQUE_NUMBER_FOR_ID_SERVER_H_

#include <QtGlobal>
#include "pigment_export.h"

class QString;

/**
 * This class is used to provide an unique number for a given \ref QString .
 * This is useful for fast comparison of Ids, but the number *should* remains private
 * especially considering that it changes from one running instance to an other.
 */
class PIGMENTCMS_EXPORT KoUniqueNumberForIdServer
{
private:
    KoUniqueNumberForIdServer();
    ~KoUniqueNumberForIdServer();
public:
    static KoUniqueNumberForIdServer* instance();
    /**
     * @return an unique number for the given \p _id , for two different call to this function
     *         with the same \p id the function will always return the same value.
     *
     * @code
     *  KoUniqueNumberForIdServer::instance()->numberForId( "rgb" ) == KoUniqueNumberForIdServer::instance()->numberForId( "rgb" );
     * KoUniqueNumberForIdServer::instance()->numberForId( "rgb" ) != KoUniqueNumberForIdServer::instance()->numberForId( "cmyk" );
     * @endcode
     */
    quint32 numberForId(const QString&);
private:
    struct Private;
    Private* const d;

};

#endif
