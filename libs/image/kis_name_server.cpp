/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_name_server.h"

KisNameServer::KisNameServer(qint32 seed) : m_generator(seed)
{ }

qint32 KisNameServer::currentSeed() const
{
    return m_generator;
}

qint32 KisNameServer::number()
{
    return m_generator++;
}

void KisNameServer::rollback()
{
    m_generator--;
}

