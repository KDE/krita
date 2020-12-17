/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_UNIQUE_NUMBER_FOR_ID_SERVER_H_
#define _KO_UNIQUE_NUMBER_FOR_ID_SERVER_H_

#include <QtGlobal>
#include "kritapigment_export.h"

class QString;

/**
 * This class is used to provide an unique number for a given \ref QString .
 * This is useful for fast comparison of Ids, but the number *should* remains private
 * especially considering that it changes from one running instance to an other.
 */
class KRITAPIGMENT_EXPORT KoUniqueNumberForIdServer
{
public:
    KoUniqueNumberForIdServer();
    ~KoUniqueNumberForIdServer();

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
