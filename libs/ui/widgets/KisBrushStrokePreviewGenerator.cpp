/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushStrokePreviewGenerator.h"

#include "KisBrushStrokePreviewCache.h"
#include "kis_preset_live_preview_view.h"

#include <QTimer>
#include <QThread>

namespace {

int previewPoolSize()
{
    const int minPoolSize = 1;
    const int maxPoolSize = 3;
    int ideal = QThread::idealThreadCount();
    if (ideal < minPoolSize) {
        ideal = minPoolSize;
    }
    return qBound(minPoolSize, ideal, maxPoolSize);
}

} // namespace

KisBrushStrokePreviewGenerator::KisBrushStrokePreviewGenerator(KisBrushStrokePreviewCache *cache)
    : QObject(cache)
    , m_cache(cache)
{
}

void KisBrushStrokePreviewGenerator::setSourceView(KisPresetLivePreviewView *view)
{
    if (!view) {
        clearPreviewPool(nullptr);
        m_livePreviewView = nullptr;
        return;
    }

    KoCanvasResourceProvider *resourceManager = view->resourceManager();
    if (!resourceManager) {
        return;
    }

    KisPresetLivePreviewView *previousView = m_livePreviewView.data();
    if (previousView && previousView->parent() != view) {
        clearPreviewPool(nullptr);
        previousView->deleteLater();
        previousView = nullptr;
    }

    if (!previousView) {
        clearPreviewPool(nullptr);
        KisPresetLivePreviewView *generatorView = new KisPresetLivePreviewView(view);
        generatorView->resize(view->size());
        generatorView->setVisible(false);
        generatorView->setup(resourceManager);
        generatorView->setCachePreviewMode(true);
        connect(generatorView, SIGNAL(sigPreviewImageReady(int,QImage)),
                m_cache, SLOT(slotLivePreviewImageReady(int,QImage)),
                Qt::UniqueConnection);
        connect(generatorView, SIGNAL(sigPreviewImageReady(int,QImage)),
                this, SLOT(slotPreviewGenerated(int,QImage)),
                Qt::UniqueConnection);
        m_livePreviewView = generatorView;
    }

    if (m_batchMode && m_livePreviewView) {
        rebuildPreviewPool();
    }
}

void KisBrushStrokePreviewGenerator::startBatch(const QList<KisPaintOpPresetSP> &presets)
{
    if (m_batchMode) {
        finalizeBatchGeneration();
    }

    m_startupPresets = presets;
    m_startupGenerationIndex = 0;
    m_batchMode = true;
    m_pendingPresetIds.clear();

    if (m_livePreviewView) {
        rebuildPreviewPool();
    }

    if (m_startupPresets.isEmpty()) {
        finalizeBatchGeneration();
        return;
    }

    QTimer::singleShot(0, this, SLOT(scheduleNextBatchGeneration()));
}

void KisBrushStrokePreviewGenerator::requestPreview(KisPaintOpPresetSP preset)
{
    if (!preset || !m_livePreviewView) {
        return;
    }

    const int presetId = preset->resourceId();
    if (presetId < 0) {
        return;
    }

    // Ignore on-demand requests during batch generation to avoid interruption/blank previews
    if (m_batchMode) {
        return;
    }

    m_livePreviewView->setCurrentPreset(preset);
    m_livePreviewView->requestUpdateStroke();
}

void KisBrushStrokePreviewGenerator::scheduleNextBatchGeneration()
{
    if (!m_batchMode) {
        return;
    }

    // If no live preview view is available, finalize batch generation
    // instead of retrying indefinitely
    if (!m_livePreviewView) {
        finalizeBatchGeneration();
        return;
    }

    if (m_previewPool.isEmpty()) {
        rebuildPreviewPool();
    }

    // If pool is still empty after rebuild, finalize instead of retrying
    if (m_previewPool.isEmpty()) {
        finalizeBatchGeneration();
        return;
    }

    bool startedAny = false;
    for (int i = 0; i < m_previewPool.size(); ++i) {
        KisPresetLivePreviewView *view = m_previewPool.at(i).data();
        if (!view || m_busyPreviewViews.contains(view)) {
            continue;
        }

        if (startNextPresetGeneration(view)) {
            startedAny = true;
        }

        if (m_startupGenerationIndex >= m_startupPresets.size()) {
            break;
        }
    }

    if (!startedAny && m_startupGenerationIndex >= m_startupPresets.size()
        && m_busyPreviewViews.isEmpty()) {
        finalizeBatchGeneration();
    }
}

bool KisBrushStrokePreviewGenerator::startNextPresetGeneration(KisPresetLivePreviewView *view)
{
    if (!view) {
        return false;
    }

    while (m_startupGenerationIndex < m_startupPresets.size()) {
        KisPaintOpPresetSP preset = m_startupPresets.at(m_startupGenerationIndex);
        ++m_startupGenerationIndex;

        if (!preset) {
            continue;
        }

        const int presetId = preset->resourceId();
        if (presetId < 0) {
            continue;
        }

        if (m_pendingPresetIds.contains(presetId)) {
            continue;
        }
        m_pendingPresetIds.insert(presetId);

        m_busyPreviewViews.insert(view);
        view->setCurrentPreset(preset);
        view->requestUpdateStrokeImmediate();
        return true;
    }

    return false;
}

void KisBrushStrokePreviewGenerator::finalizeBatchGeneration()
{
    m_startupPresets.clear();
    m_startupGenerationIndex = 0;
    m_busyPreviewViews.clear();
    m_pendingPresetIds.clear();

    if (!m_batchMode) {
        return;
    }

    m_batchMode = false;
    if (!m_previewPool.isEmpty()) {
        Q_FOREACH (const QPointer<KisPresetLivePreviewView> &viewPtr, m_previewPool) {
            if (viewPtr) {
                viewPtr->setBatchPreviewMode(false);
            }
        }
    } else if (m_livePreviewView) {
        m_livePreviewView->setBatchPreviewMode(false);
    }
}

void KisBrushStrokePreviewGenerator::clearPreviewPool(KisPresetLivePreviewView *keepView)
{
    Q_FOREACH (const QPointer<KisPresetLivePreviewView> &viewPtr, m_previewPool) {
        KisPresetLivePreviewView *view = viewPtr.data();
        if (view && view != keepView) {
            view->deleteLater();
        }
    }
    m_previewPool.clear();
    m_busyPreviewViews.clear();
}

void KisBrushStrokePreviewGenerator::rebuildPreviewPool()
{
    if (!m_livePreviewView) {
        clearPreviewPool(nullptr);
        return;
    }

    for (int i = m_previewPool.size() - 1; i >= 0; --i) {
        if (!m_previewPool.at(i)) {
            m_previewPool.removeAt(i);
        }
    }

    bool hasLiveView = false;
    Q_FOREACH (const QPointer<KisPresetLivePreviewView> &viewPtr, m_previewPool) {
        if (viewPtr && viewPtr == m_livePreviewView) {
            hasLiveView = true;
            break;
        }
    }

    if (!hasLiveView) {
        clearPreviewPool(m_livePreviewView.data());
    }

    if (m_previewPool.isEmpty()) {
        m_previewPool.append(m_livePreviewView.data());
    }

    Q_FOREACH (const QPointer<KisPresetLivePreviewView> &viewPtr, m_previewPool) {
        if (!viewPtr) {
            continue;
        }

        viewPtr->setBatchPreviewMode(m_batchMode);
        viewPtr->setCachePreviewMode(true);
        connect(viewPtr.data(), SIGNAL(sigPreviewImageReady(int,QImage)),
                m_cache, SLOT(slotLivePreviewImageReady(int,QImage)),
                Qt::UniqueConnection);
        connect(viewPtr.data(), SIGNAL(sigPreviewImageReady(int,QImage)),
                this, SLOT(slotPreviewGenerated(int,QImage)),
                Qt::UniqueConnection);
    }

    const int desired = previewPoolSize();
    if (m_previewPool.size() >= desired) {
        return;
    }

    KoCanvasResourceProvider *resourceManager = m_livePreviewView->resourceManager();
    if (!resourceManager) {
        return;
    }

    m_previewPool.reserve(desired);
    while (m_previewPool.size() < desired) {
        KisPresetLivePreviewView *view = new KisPresetLivePreviewView(m_livePreviewView);
        view->resize(m_livePreviewView->size());
        view->setVisible(false);
        view->setup(resourceManager);
        view->setBatchPreviewMode(m_batchMode);
        view->setCachePreviewMode(true);
        connect(view, SIGNAL(sigPreviewImageReady(int,QImage)),
                m_cache, SLOT(slotLivePreviewImageReady(int,QImage)),
                Qt::UniqueConnection);
        connect(view, SIGNAL(sigPreviewImageReady(int,QImage)),
                this, SLOT(slotPreviewGenerated(int,QImage)),
                Qt::UniqueConnection);
        m_previewPool.append(view);
    }
}

void KisBrushStrokePreviewGenerator::slotPreviewGenerated(int presetId, const QImage &previewImage)
{
    Q_UNUSED(previewImage);

    if (presetId >= 0) {
        m_pendingPresetIds.remove(presetId);
    }

    if (!m_batchMode) {
        return;
    }

    KisPresetLivePreviewView *senderView =
        qobject_cast<KisPresetLivePreviewView*>(sender());
    if (!senderView || !m_busyPreviewViews.contains(senderView)) {
        return;
    }

    m_busyPreviewViews.remove(senderView);
    if (startNextPresetGeneration(senderView)) {
        return;
    }

    if (m_startupGenerationIndex >= m_startupPresets.size()
        && m_busyPreviewViews.isEmpty()) {
        finalizeBatchGeneration();
    }
}
