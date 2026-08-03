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
                                      QSize _outputSize = QSize(), QSharedPointer<boost::none_t> _cookie = nullptr)
            : canvasDev(_canvasDev), canvasPixelRect(_canvasPixelRect), colorConverter(_colorConverter),
            outputSize(_outputSize), strokeCookie(_cookie)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            KisLodTransform transform(levelOfDetail);
            QRect lodPixelRect = transform.map(canvasPixelRect);

            // The LOD pixel rect size should be odd so that the pixel sampled is in the center
            // Otherwise, even after scaling back, it will be wrong
            if (lodPixelRect.width() % 2 == 0) {
                int oddSize = lodPixelRect.width() - 1;
                if (oddSize < 3) oddSize = 3;
                lodPixelRect.setSize(QSize(oddSize, oddSize));
            }
            lodPixelRect.moveCenter(transform.map(canvasPixelRect.center()));

            // When instant preview mode is on, canvas is scaled down. Therefore, sample the canvas with the scaled LOD rect
            // But the data returned needs to be the original requested size
            GenerateCanvasZoomPreviewData *newData =
                new GenerateCanvasZoomPreviewData(canvasDev, lodPixelRect, colorConverter, canvasPixelRect.size());

            // When Lod is involved, swap the cookie to the new Lod clone to track execution
            // The original object seems to leaks
            newData->swapCookie(strokeCookie);

            return newData;
        }

        QWeakPointer<boost::none_t> cookie() {
            strokeCookie.reset(new boost::none_t(boost::none));
            return strokeCookie;
        }

        void swapCookie(QSharedPointer<boost::none_t> &newCookie) {
            strokeCookie.swap(newCookie);
        }

        KisPaintDeviceSP canvasDev;
        QRect canvasPixelRect;
        KisDisplayColorConverter *colorConverter;
        QSize outputSize; // Useful for LOD
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
