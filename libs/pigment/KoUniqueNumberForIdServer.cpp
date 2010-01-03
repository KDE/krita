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

#include "KoUniqueNumberForIdServer.h"

#include <QHash>
#ifdef Q_CC_MSVC
#include <iso646.h>
#endif

#include <kglobal.h>

struct KoUniqueNumberForIdServer::Private {
    Private()
            : currentNumber(0) {}

    QHash<QString, quint32 > id2Number;
    quint32 currentNumber;
};

KoUniqueNumberForIdServer::KoUniqueNumberForIdServer()
        : d(new Private)
{
}

KoUniqueNumberForIdServer::~KoUniqueNumberForIdServer()
{
    delete d;
}

KoUniqueNumberForIdServer* KoUniqueNumberForIdServer::instance()
{
    K_GLOBAL_STATIC(KoUniqueNumberForIdServer, s_instance);
    return s_instance;
}

quint32 KoUniqueNumberForIdServer::numberForId(const QString& _id)
{
    QHash<QString, quint32>::iterator it = d->id2Number.find(_id);
    if (it != d->id2Number.end()) {
        return it.value();
    }
    quint32 number = ++d->currentNumber;
    d->id2Number[ _id ] = number;
    return number;
}
