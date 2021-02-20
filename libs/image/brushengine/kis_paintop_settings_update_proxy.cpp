/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
