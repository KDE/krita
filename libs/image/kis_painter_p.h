/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPAINTERPRIVATE_H
#define KISPAINTERPRIVATE_H

#include <KoColorSpace.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>
#include <KoUpdater.h>
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_painter.h"
#include "kis_paintop_preset.h"
#include <KisFakeRunnableStrokeJobsExecutor.h>

struct Q_DECL_HIDDEN KisPainter::Private {
    Private(KisPainter *_q) : q(_q) {}
    Private(KisPainter *_q, const KoColorSpace *cs)
        : q(_q), paintColor(cs), backgroundColor(cs) {}

    KisPainter *q {nullptr};

    KisPaintDeviceSP            device;
    KisSelectionSP              selection;
    KisTransaction*             transaction {nullptr};
    KoUpdater*                  progressUpdater {nullptr};

    QVector<QRect>              dirtyRects;
    KisPaintOp*                 paintOp {nullptr};
    KoColor                     paintColor;
    KoColor                     backgroundColor;
    KoColor                     customColor;
    KisFilterConfigurationSP    generator;
    KisPaintLayer*              sourceLayer {nullptr};
    FillStyle                   fillStyle {FillStyleNone};
    StrokeStyle                 strokeStyle {StrokeStyleBrush};
    bool                        antiAliasPolygonFill {true};
    KoPatternSP                 pattern;
    QPointF                     duplicateOffset;
    quint32                     pixelSize {0};
    const KoColorSpace*         colorSpace {nullptr};
    KoColorProfile*             profile {nullptr};
    const KoCompositeOp*        compositeOp {nullptr};
    KoAbstractGradientSP        gradient;
    KisPaintOpPresetSP          paintOpPreset;
    QImage                      polygonMaskImage;
    QPainter*                   maskPainter {nullptr};
    KisFillPainter*             fillPainter {nullptr};
    KisPaintDeviceSP            polygon;
    qint32                      maskImageWidth {255};
    qint32                      maskImageHeight {255};
    QPointF                     axesCenter;
    bool                        mirrorHorizontally {false};
    bool                        mirrorVertically {false};
    bool                        isOpacityUnit {true}; // TODO: move into ParameterInfo
    KoCompositeOp::ParameterInfo paramInfo;
    KoColorConversionTransformation::Intent renderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;
    KisRunnableStrokeJobsInterface *runnableStrokeJobsInterface {nullptr};
    QScopedPointer<KisRunnableStrokeJobsInterface> fakeRunnableStrokeJobsInterface;
    QTransform                  patternTransform;

    bool tryReduceSourceRect(const KisPaintDevice *srcDev,
                             QRect *srcRect,
                             qint32 *srcX,
                             qint32 *srcY,
                             qint32 *srcWidth,
                             qint32 *srcHeight,
                             qint32 *dstX,
                             qint32 *dstY);

    void fillPainterPathImpl(const QPainterPath& path, const QRect &requestedRect);

    void applyDevice(const QRect &applyRect,
                     const KisRenderedDab &dab,
                     KisRandomAccessorSP dstIt,
                     const KoColorSpace *srcColorSpace,
                     KoCompositeOp::ParameterInfo &localParamInfo);

    void applyDeviceWithSelection(const QRect &applyRect,
                                  const KisRenderedDab &dab,
                                  KisRandomAccessorSP dstIt,
                                  KisRandomConstAccessorSP maskIt,
                                  const KoColorSpace *srcColorSpace,
                                  KoCompositeOp::ParameterInfo &localParamInfo);

    template<class T> QVector<T> calculateMirroredObjects(const T &object);

};

#endif // KISPAINTERPRIVATE_H
