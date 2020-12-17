/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoUniqueNumberForIdServer.h"

#include <QHash>
#include <QGlobalStatic>

Q_GLOBAL_STATIC(KoUniqueNumberForIdServer, s_instance)

struct Q_DECL_HIDDEN KoUniqueNumberForIdServer::Private {
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
