/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPresetShadowUpdater.h"

#include <KisViewManager.h>

#include <KoCanvasResourcesIds.h>

#include <kis_paintop_preset.h>
#include <KisPaintOpPresetUpdateProxy.h>
#include <kis_signal_auto_connection.h>
#include <kis_signal_compressor.h>

#include <kis_image.h>
#include <kis_spontaneous_job.h>

namespace {
struct ShadowUpdatePresetJob : KisSpontaneousJob
{
    ShadowUpdatePresetJob(KisPaintOpPresetSP preset)
        : m_preset(preset)
    {
    }


    void run() override {
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_preset);
        m_preset->coldInitInBackground();
    }

    bool overrides(const KisSpontaneousJob *_otherJob) override {
        const ShadowUpdatePresetJob *otherJob =
            dynamic_cast<const ShadowUpdatePresetJob*>(_otherJob);

        return otherJob;
    }

    int levelOfDetail() const override {
        return 0;
    }

    QString debugName() const override {
        QString result;
        QDebug dbg(&result);
        dbg << "ShadowUpdatePresetJob" << m_preset;
        return result;
    }

private:
    KisPaintOpPresetSP m_preset;
};
}

struct KisPresetShadowUpdater::Private
{
    Private()
        : updateStartCompressor(1500, KisSignalCompressor::POSTPONE)
    {
    }

    KisViewManager *view;

    KisPaintOpPresetSP currentPreset;
    QPointer<KisPaintOpPresetUpdateProxy> currentUpdateProxy;

    KisSignalAutoConnectionsStore proxyConnections;
    KisSignalCompressor updateStartCompressor;

};

KisPresetShadowUpdater::KisPresetShadowUpdater(KisViewManager *view)
    : m_d(new Private)
{
    m_d->view = view;
    connect(&m_d->updateStartCompressor, SIGNAL(timeout()),
            this, SLOT(slotStartPresetPreparation()));
}

KisPresetShadowUpdater::~KisPresetShadowUpdater()
{
}

void KisPresetShadowUpdater::slotCanvasResourceChanged(int key, const QVariant &value)
{
    if (key == KoCanvasResource::CurrentPaintOpPreset) {
        m_d->currentPreset = value.value<KisPaintOpPresetSP>();

        m_d->proxyConnections.clear();

        if (m_d->currentPreset) {
            m_d->currentUpdateProxy = m_d->currentPreset->updateProxy();
            m_d->proxyConnections.addConnection(
                m_d->currentUpdateProxy, SIGNAL(sigSettingsChanged()),
                this, SLOT(slotPresetChanged()));
            slotPresetChanged();
        }
    }
}

void KisPresetShadowUpdater::slotPresetChanged()
{
    m_d->updateStartCompressor.start();
}

void KisPresetShadowUpdater::slotStartPresetPreparation()
{
    if (m_d->currentPreset && m_d->currentPreset->needsColdInitInBackground()) {
        KisImageSP image = m_d->view->image();
        if (image) {
            image->addSpontaneousJob(new ShadowUpdatePresetJob(m_d->currentPreset));
        } else {
            /// a fallback solution when a preset is selected on
            /// Krita loading, when there is no image present
            m_d->currentPreset->coldInitInBackground();
        }
    }
}
