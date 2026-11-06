/*
 *  SPDX-FileCopyrightText: 2026 Tenzin Rangdol <tenzindraws@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_STROKE_PREVIEW_CACHE_H
#define KIS_BRUSH_STROKE_PREVIEW_CACHE_H

#include <QImage>
#include <QObject>
#include <QScopedPointer>
#include <QSize>
#include <QString>
#include <QVariant>

#include <kis_paintop_preset.h>
#include <kritaui_export.h>

class KisPresetLivePreviewView;

/**
 * Thread-safe LRU cache for brush stroke preview images.
 * Previews are generated via KisPresetLivePreviewView and persisted to disk.
 */
class KRITAUI_EXPORT KisBrushStrokePreviewCache : public QObject
{
    Q_OBJECT

public:
    static KisBrushStrokePreviewCache* instance();

    /// True if paint engine needs striped background (colorsmudge, deform, filter).
    static bool needsStripedBackground(const QString &paintOpId);

    /// True if paint engine has no preview support.
    static bool isNoPreviewEngine(const QString &paintOpId);

    /// Get cached preview scaled to size, or placeholder if not cached.
    QImage getPreview(int presetId, const QSize &size, int pixelRatio);

    /// Invalidate cache for a preset when its settings change.
    void invalidatePreset(int presetId);

    /// Register live preview view for generating stroke previews.
    void registerLivePreviewView(KisPresetLivePreviewView *view);

Q_SIGNALS:
    /// Emitted when a preview is ready; triggers UI repaint.
    void sigPreviewReady(int presetId);

public Q_SLOTS:
    /// Receive preview image from live preview view.
    void slotLivePreviewImageReady(int presetId, QImage previewImage);

    void slotPreviewImageReady(int presetId, QImage previewImage);

    /// Handle preset settings change.
    void slotPresetSettingsChanged(KisPaintOpPresetSP preset);

    /// Track current preset for settings change detection.
    void slotSetCurrentPreset(KisPaintOpPresetSP preset);

private Q_SLOTS:
    void slotCurrentPresetSettingsChanged();

private:
    KisBrushStrokePreviewCache();
    ~KisBrushStrokePreviewCache() override;
    Q_DISABLE_COPY(KisBrushStrokePreviewCache)

    QString generateCacheKey(int presetId) const;
    QString generateSizedCacheKey(int presetId, const QSize &size) const;
    /// Make sure that we have under >= 500 items cached, if there are more remove the oldest ones
    void evictIfNeeded();
    void updateAccessOrder(const QString &key);
    QImage generatePlaceholder(const QSize &size, int pixelRatio) const;
    QImage generateNoPreviewPlaceholder(const QSize &size, int pixelRatio) const;
    QString generateSettingsFingerprint(KisPaintOpPresetSP preset) const;
    QImage scalePreviewToSize(const QImage &source, const QSize &targetSize) const;

    // Disk cache
    void initDiskCache();
    QString getDiskCacheFilename(int presetId) const;
    QImage loadFromDiskCache(int presetId);
    void saveToDiskCache(int presetId, QImage image);
    bool hasDiskCache(int presetId) const;
    void removeDiskCache(int presetId);

    void requestPreviewGeneration(int presetId);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KIS_BRUSH_STROKE_PREVIEW_CACHE_H
