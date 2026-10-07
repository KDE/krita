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
        m_livePreviewView = generatorView;
    }
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

    m_livePreviewView->setCurrentPreset(preset);
    m_livePreviewView->requestUpdateStroke();
}
