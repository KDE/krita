/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushStrokePreviewGenerator.h"
#include "KisBrushStrokePreviewCache.h"
#include "kis_preset_live_preview_view.h"

KisBrushStrokePreviewGenerator::KisBrushStrokePreviewGenerator(KisBrushStrokePreviewCache *cache)
    : QObject(cache)
    , m_cache(cache)
{
}

void KisBrushStrokePreviewGenerator::setSourceView(KisPresetLivePreviewView *view)
{
    if (!view) {
        m_livePreviewView = nullptr;
        return;
    }

    KoCanvasResourceProvider *resourceManager = view->resourceManager();
    if (!resourceManager) {
        return;
    }

    KisPresetLivePreviewView *previousView = m_livePreviewView.data();
    if (previousView && previousView->parent() != view) {
        previousView->deleteLater();
        previousView = nullptr;
    }

    if (!previousView) {
        KisPresetLivePreviewView *generatorView = new KisPresetLivePreviewView(view);
        generatorView->resize(view->size());
        generatorView->setVisible(false);
        generatorView->setup(resourceManager);
        generatorView->setCachePreviewMode(true);
        connect(generatorView, &KisPresetLivePreviewView::sigPreviewImageReady,
                m_cache, &KisBrushStrokePreviewCache::slotLivePreviewImageReady,
                Qt::UniqueConnection);

        connect(generatorView, &KisPresetLivePreviewView::sigPreviewImageReady,
                this, &KisBrushStrokePreviewGenerator::slotPreviewFinished,
                Qt::UniqueConnection);

        m_livePreviewView = generatorView;
    }
}

void KisBrushStrokePreviewGenerator::slotPreviewFinished(int finishedPresetId)
{
    // Only now remove the item from the queue, since it actually Finished, so that there are no duplicate requests
    if (finishedPresetId > 0) {
        m_queue.removeIf([finishedPresetId](KisPaintOpPresetSP preset) {
            return preset->resourceId() == finishedPresetId;
        });
    }

    if (!m_queue.isEmpty()) {
        KisPaintOpPresetSP preset = m_queue.first();

        m_livePreviewView->setCurrentPreset(preset);
        m_livePreviewView->requestUpdateStroke();
        return;
    }

    m_generationInProgress = false;
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

    if (m_queue.contains(preset)) {
        // Preset already queued for generation, so ignore it
        return;
    }

    if (!m_generationInProgress) {
        m_livePreviewView->setCurrentPreset(preset);
        m_livePreviewView->requestUpdateStroke();
        m_generationInProgress = true;
    } else {
        m_queue.push_back( preset);
    }
}
