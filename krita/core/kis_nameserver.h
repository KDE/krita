/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_NAMESERVER_H_
#define KIS_NAMESERVER_H_

#include <qstring.h>
#include "kis_global.h"

class KisNameServer {
public:
    KisNameServer(const QString& prefix, Q_INT32 seed = 1);
    ~KisNameServer();

    QString name();
    Q_INT32 number();
    Q_INT32 currentSeed() const;
    void rollback();

private:
    Q_INT32 m_generator;
    QString m_prefix;
};

#endif // KIS_NAMESERVER_H_

