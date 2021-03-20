/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_NAMESERVER_H_
#define KIS_NAMESERVER_H_

#include <QtGlobal>
#include <kritaimage_export.h>

class KRITAIMAGE_EXPORT KisNameServer
{
public:
    KisNameServer(qint32 seed = 1);

    qint32 number();
    qint32 currentSeed() const;
    void rollback();

private:
    qint32 m_generator;
};

#endif // KIS_NAMESERVER_H_

