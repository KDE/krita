/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_COLOR_SAMPLER_STROKE_STRATEGY_H
#define __KIS_COLOR_SAMPLER_STROKE_STRATEGY_H

#include <QObject>
#include "kis_simple_stroke_strategy.h"
#include "kis_lod_transform.h"
#include "KoColor.h"

class KisCanvas2;

class KisColorSamplerStrokeStrategy : public QObject, public KisSimpleStrokeStrategy
{
    Q_OBJECT
public:
    class Data : public KisStrokeJobData {
    public:
        Data(KisPaintDeviceSP _dev, const QPoint _pt, KoColor _currentColor)
            : dev(_dev), pt(_pt), currentColor(_currentColor)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            KisLodTransform t(levelOfDetail);
            const QPoint realPoint = t.map(pt);

            return new Data(dev, realPoint, currentColor);
        }

        KisPaintDeviceSP dev;
        QPoint pt;
        KoColor currentColor; // Used for color sampler blending.
    };

    class FinalizeData : public KisStrokeJobData {
    public:
        FinalizeData()
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            Q_UNUSED(levelOfDetail);
            return new FinalizeData();
        }
    };

    class GenerateCanvasZoomPreviewData : public KisStrokeJobData {
    public:
        GenerateCanvasZoomPreviewData(KisCanvas2* _canvas, const QRect &_canvasPixelRect, const KoColorProfile *_colorProfile)
            : canvas(_canvas), canvasPixelRect(_canvasPixelRect), colorProfile(_colorProfile)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            Q_UNUSED(levelOfDetail);
            return new GenerateCanvasZoomPreviewData(canvas, canvasPixelRect, colorProfile);
        }

        KisCanvas2 *canvas;
        QRect canvasPixelRect;
        const KoColorProfile *colorProfile;
    };
public:
    KisColorSamplerStrokeStrategy(int radius, int blend, int lod = 0);
    ~KisColorSamplerStrokeStrategy() override;

    void doStrokeCallback(KisStrokeJobData *data) override;
    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

Q_SIGNALS:
    void sigColorUpdated(const KoColor &color);
    void sigFinalColorSelected(const KoColor &color);
    void sigCanvasZoomPreviewUpdated(const QImage &canvasImage, const QRect &canvasRect);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLOR_SAMPLER_STROKE_STRATEGY_H */
