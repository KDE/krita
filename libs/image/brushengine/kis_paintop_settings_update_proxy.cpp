/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_paintop_settings_update_proxy.h"

#include "kis_signal_compressor.h"

#include <kis_paintop_preset.h>

struct KisPaintopSettingsUpdateProxy::Private
{
    Private()
        : updatesCompressor(100, KisSignalCompressor::FIRST_ACTIVE),
          updatesBlocked(0),
          numUpdatesWhileBlocked(0)
    {
    }

    KisSignalCompressor updatesCompressor;
    int updatesBlocked;
    int numUpdatesWhileBlocked;
};

KisPaintopSettingsUpdateProxy::KisPaintopSettingsUpdateProxy(QObject *parent)
    : QObject(parent),
      m_d(new Private)
{
    connect(&m_d->updatesCompressor, SIGNAL(timeout()), SLOT(slotDeliverSettingsChanged()));
}

KisPaintopSettingsUpdateProxy::~KisPaintopSettingsUpdateProxy()
{
}

void KisPaintopSettingsUpdateProxy::setDirty(bool dirty)
{
    ProxyParent *proxyParent = qobject_cast<ProxyParent*>(parent());
    if (proxyParent){
        KisPaintOpPreset *preset = proxyParent->m_preset;
        if (preset) {
            preset->setDirty(dirty);
        }
    }
}

void KisPaintopSettingsUpdateProxy::notifySettingsChanged()
{
    m_d->updatesCompressor.start();
}

void KisPaintopSettingsUpdateProxy::notifyUniformPropertiesChanged()
{
    emit sigUniformPropertiesChanged();
}

void KisPaintopSettingsUpdateProxy::postponeSettingsChanges()
{
    m_d->updatesBlocked++;
}

void KisPaintopSettingsUpdateProxy::unpostponeSettingsChanges()
{
    m_d->updatesBlocked--;

    if (!m_d->updatesBlocked && m_d->numUpdatesWhileBlocked) {
        m_d->numUpdatesWhileBlocked = 0;
        emit sigSettingsChanged();
    }
}

void KisPaintopSettingsUpdateProxy::slotDeliverSettingsChanged()
{
    if (m_d->updatesBlocked) {
        m_d->numUpdatesWhileBlocked++;
    } else {
        emit sigSettingsChanged();
    }
}
