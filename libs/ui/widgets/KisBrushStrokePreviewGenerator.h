/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_STROKE_PREVIEW_GENERATOR_H
#define KIS_BRUSH_STROKE_PREVIEW_GENERATOR_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QVector>

#include <kis_paintop_preset.h>

class QImage;
class KisBrushStrokePreviewCache;
class KisPresetLivePreviewView;

/**
 * @brief Manages brush stroke preview generation and batch scheduling.
 */
class KisBrushStrokePreviewGenerator : public QObject
{
    Q_OBJECT

public:
    explicit KisBrushStrokePreviewGenerator(KisBrushStrokePreviewCache *cache);

    void setSourceView(KisPresetLivePreviewView *view);
    void startBatch(const QList<KisPaintOpPresetSP> &presets);
    void requestPreview(KisPaintOpPresetSP preset);

private Q_SLOTS:
    void scheduleNextBatchGeneration();
    /**
     * @brief Handle completion for a presetId preview.
     */
    void slotPreviewGenerated(int presetId, const QImage &previewImage);

private:
    bool startNextPresetGeneration(KisPresetLivePreviewView *view);
    void finalizeBatchGeneration();
    void clearPreviewPool(KisPresetLivePreviewView *keepView);
    void rebuildPreviewPool();

    KisBrushStrokePreviewCache *m_cache = nullptr;
    QPointer<KisPresetLivePreviewView> m_livePreviewView;
    QVector<QPointer<KisPresetLivePreviewView>> m_previewPool;
    QSet<KisPresetLivePreviewView*> m_busyPreviewViews;
    QSet<int> m_pendingPresetIds;
    QList<KisPaintOpPresetSP> m_startupPresets;
    int m_startupGenerationIndex = 0;
    bool m_batchMode = false;
};

#endif
