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

class KisDisplayColorConverter;

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
        GenerateCanvasZoomPreviewData(KisPaintDeviceSP _canvasDev, const QRect &_canvasPixelRect, KisDisplayColorConverter *_colorConverter,
                                      int _levelOfDetail = 0, QSharedPointer<boost::none_t> _cookie = nullptr)
            : canvasDev(_canvasDev), canvasPixelRect(_canvasPixelRect), colorConverter(_colorConverter), levelOfDetail(_levelOfDetail), strokeCookie(_cookie)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            KisLodTransform transform(levelOfDetail);
            QRect lodPixelRect = transform.map(canvasPixelRect);
            GenerateCanvasZoomPreviewData *newData =
                new GenerateCanvasZoomPreviewData(canvasDev, lodPixelRect, colorConverter, levelOfDetail);
            // When Lod is involved, swap the cookie to the new Lod clone to track execution
            // The original object seems to leaks
            newData->strokeCookie.swap(strokeCookie);

            return newData;
        }

        QWeakPointer<boost::none_t> cookie() {
            strokeCookie.reset(new boost::none_t(boost::none));
            return strokeCookie;
        }

        KisPaintDeviceSP canvasDev;
        QRect canvasPixelRect;
        KisDisplayColorConverter *colorConverter;
        int levelOfDetail;
        QSharedPointer<boost::none_t> strokeCookie;
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
