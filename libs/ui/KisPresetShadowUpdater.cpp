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

#include <KisGlobalResourcesInterface.h>
#include <kis_canvas_resource_provider.h>
#include <KoCanvasResourceProvider.h>
#include <KoCanvasResourcesInterface.h>
#include <KoResourceCacheStorage.h>


class ShadowUpdatePresetJob : public QObject, public KisSpontaneousJob
{
    Q_OBJECT
public:

    ShadowUpdatePresetJob(KisPaintOpPresetSP preset, int sequenceNumber)
        : m_preset(preset),
          m_sequenceNumber(sequenceNumber)
    {
    }

    void run() override {
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_preset);

        KoResourceCacheInterfaceSP cacheInterface =
            toQShared(new KoResourceCacheStorage());

        m_preset->regenerateResourceCache(cacheInterface);

        Q_EMIT sigCacheGenerationFinished(m_sequenceNumber, cacheInterface);
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

Q_SIGNALS:
    void sigCacheGenerationFinished(int sequenceNumber,
                                    KoResourceCacheInterfaceSP cacheInterface);
private:
    KisPaintOpPresetSP m_preset;
    const int m_sequenceNumber;
};

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

    int sequenceNumber = 0;

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
        } else {
            m_d->view->canvasResourceProvider()->resourceManager()->
                    setResource(KoCanvasResource::CurrentPaintOpPresetCache,
                                QVariant::fromValue(KoResourceCacheInterfaceSP()));
        }
    } else if (m_d->currentPreset) {
        /// if the current preset depends on the canvas resources,
        /// we should reset its cache and restart cache generation

        if (m_d->currentPreset->requiredCanvasResources().contains(key)) {
            slotPresetChanged();
        }
    }
}

void KisPresetShadowUpdater::slotPresetChanged()
{
    m_d->sequenceNumber++;
    m_d->updateStartCompressor.start();

    m_d->view->canvasResourceProvider()->resourceManager()->
            setResource(KoCanvasResource::CurrentPaintOpPresetCache,
                        QVariant::fromValue(KoResourceCacheInterfaceSP()));

}

void KisPresetShadowUpdater::slotStartPresetPreparation()
{
    if (m_d->currentPreset) {
        KisImageSP image = m_d->view->image();
        if (image) {
            KisPaintOpPresetSP preset =
                    m_d->currentPreset->cloneWithResourcesSnapshot(
                        KisGlobalResourcesInterface::instance(),
                        m_d->view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface(),
                        nullptr);

            ShadowUpdatePresetJob *job = new ShadowUpdatePresetJob(preset, m_d->sequenceNumber);

            connect(job, SIGNAL(sigCacheGenerationFinished(int, KoResourceCacheInterfaceSP)),
                    this, SLOT(slotCacheGenerationFinished(int, KoResourceCacheInterfaceSP)));

            image->addSpontaneousJob(job);
        } else {
            /// a fallback solution when a preset is selected on
            /// Krita loading, when there is no image present

            KoResourceCacheInterfaceSP cacheInterface =
                toQShared(new KoResourceCacheStorage());

            m_d->currentPreset->regenerateResourceCache(cacheInterface);
            slotCacheGenerationFinished(m_d->sequenceNumber, cacheInterface);
        }
    }
}

void KisPresetShadowUpdater::slotCacheGenerationFinished(int sequenceNumber, KoResourceCacheInterfaceSP cacheInterface)
{
    if (sequenceNumber == m_d->sequenceNumber) {
        m_d->view->canvasResourceProvider()->resourceManager()->
                setResource(KoCanvasResource::CurrentPaintOpPresetCache,
                            QVariant::fromValue(cacheInterface));
    }
}

#include "KisPresetShadowUpdater.moc"
