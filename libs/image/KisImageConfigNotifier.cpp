/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisImageConfigNotifier.h"

#include <QGlobalStatic>

#include <kis_debug.h>
#include "kis_signal_compressor.h"

Q_GLOBAL_STATIC(KisImageConfigNotifier, s_instance)

struct KisImageConfigNotifier::Private
{
    Private() : updateCompressor(300, KisSignalCompressor::FIRST_ACTIVE) {}

    KisSignalCompressor updateCompressor;
};

KisImageConfigNotifier::KisImageConfigNotifier()
    : m_d(new Private)
{
    connect(&m_d->updateCompressor, SIGNAL(timeout()), SIGNAL(configChanged()));
}

KisImageConfigNotifier::~KisImageConfigNotifier()
{
}

KisImageConfigNotifier *KisImageConfigNotifier::instance()
{
    return s_instance;
}

void KisImageConfigNotifier::notifyConfigChanged()
{
    m_d->updateCompressor.start();
}
