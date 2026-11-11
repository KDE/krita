/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_STROKE_PREVIEW_GENERATOR_H
#define KIS_BRUSH_STROKE_PREVIEW_GENERATOR_H

#include <QObject>
#include <QPointer>
#include <kis_paintop_preset.h>

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
    void requestPreview(KisPaintOpPresetSP preset);

private Q_SLOTS:
    void slotPreviewFinished(int presetId);

private:
    KisBrushStrokePreviewCache *m_cache = nullptr;
    QPointer<KisPresetLivePreviewView> m_livePreviewView;
    QVector<KisPaintOpPresetSP> m_queue;
    bool m_generationInProgress = false;
};

#endif
