
/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2025 Carsten Hartenfels <carsten.hartenfels@pm.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAsyncColorSamplerHelper.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QTransform>

#include "KoCanvasResourcesIds.h"
#include "KoCanvasResourceProvider.h"
#include "KoShapeManager.h"
#include "KoViewConverter.h"
#include "KoIcon.h"
#include "kis_cursor.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_image_interfaces.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "KisReferenceImagesLayer.h"
#include "KisReferenceImagesDecoration.h"
#include "kis_display_color_converter.h"
#include "strokes/kis_color_sampler_stroke_strategy.h"


namespace {
QColor colorWithAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}
}

struct KisAsyncColorSamplerHelper::Private
{
    static constexpr qreal PREVIEW_RECT_SIZE = 48.0;

    Private(KisCanvas2 *_canvas)
        : canvas(_canvas)
    {}

    KisCanvas2 *canvas;

    int sampleResourceId {0};
    bool sampleCurrentLayer {true};
    bool updateGlobalColor {true};

    bool isActive {false};
    bool showPreview {false};
    bool haveSample {false};

    KisStrokeId strokeId;
    typedef KisSignalCompressorWithParam<QPointF> SamplingCompressor;
    QScopedPointer<SamplingCompressor> samplingCompressor;

    QTimer activationDelayTimer;

    KisConfig::ColorSamplerPreviewStyle style = KisConfig::ColorSamplerPreviewStyle::Circle;
    int circlePreviewDiameter {180};
    qreal circlePreviewThickness {0.12};
    bool circlePreviewOutlineEnabled {true};
    bool circlePreviewExtraCircles {true};
    QRectF previewDocRect;

    bool circleZoomPreviewEnabled {false};
    qreal circleZoomPreviewScale {5};
    KisConfig::ColorSamplerPreviewCirclePosition circlePreviewPosition = KisConfig::ColorSamplerPreviewCirclePosition::Center;
    QPointF sampleDocPoint;

    QPainterPath cacheCircleInnerClip;
    QRect cacheCanvasPreviewRect;
    QImage cacheCanvasPreviewImage;
    QPainterPath cacheCrosshairPath;
    int cacheCirclePreviewDiameter;

    QColor currentColor;
    QColor baseColor;
    QColor backgroundColor;

    QPixmap cache;
    qreal cacheRotation = 0.0;
    bool cacheMirror = false;

    KisStrokesFacade *strokesFacade() const {
        return canvas->image().data();
    }

    const KoViewConverter &converter() const {
        return *canvas->imageView()->viewConverter();
    }

    QRectF colorPreviewRectForRectangle() const
    {
        // Offsetting to the sides is both vertical and horizontal, when
        // offsetting above it's only vertical, so it needs a bit more space.
        constexpr qreal OFFSET = 32.0;
        constexpr qreal OFFSET_ABOVE = OFFSET * 1.5;
        constexpr qreal SIZE = PREVIEW_RECT_SIZE;

        bool mirrored = canvas->xAxisMirrored();
        bool flipped = canvas->yAxisMirrored();

        KisConfig::ColorSamplerPreviewStyle effectiveStyle;
        if (mirrored && style == KisConfig::ColorSamplerPreviewStyle::RectangleLeft) {
            effectiveStyle = KisConfig::ColorSamplerPreviewStyle::RectangleRight;
        } else if (mirrored && style == KisConfig::ColorSamplerPreviewStyle::RectangleRight) {
            effectiveStyle = KisConfig::ColorSamplerPreviewStyle::RectangleLeft;
        } else {
            effectiveStyle = style;
        }

        qreal width = haveSample ? SIZE * 2.0 : SIZE;

        qreal x, y;
        switch (effectiveStyle) {
        case KisConfig::ColorSamplerPreviewStyle::RectangleLeft:
            x = -(OFFSET + width);
            y = flipped ? -(OFFSET + SIZE) : OFFSET;
            break;
        case KisConfig::ColorSamplerPreviewStyle::RectangleRight:
            x = OFFSET;
            y = flipped ? -(OFFSET + SIZE) : OFFSET;
            break;
        default:
            x = width / -2.0;
            y = flipped ? OFFSET_ABOVE : -(OFFSET_ABOVE + SIZE);
            break;
        }

        QRectF rect(x, y, width, SIZE);

        qreal canvasRotationAngle = canvas->rotationAngle();
        if (!qFuzzyIsNull(canvasRotationAngle)) {
            QTransform tf;
            tf.rotate(mirrored ? canvasRotationAngle : -canvasRotationAngle);
            rect = tf.mapRect(rect);
        }

        return rect;
    }

    QRectF colorPreviewRectForCircle()
    {
        QRectF circleRect = QRectF(-circlePreviewDiameter / 2.0, -circlePreviewDiameter / 2.0, circlePreviewDiameter, circlePreviewDiameter);

        // If zoom preview enabled, it's possible to offset the color sampler preview and still be accurate
        // This can help when sampling color using finger gesture
        if (circleZoomPreviewEnabled) {
            if (circlePreviewPosition == KisConfig::ColorSamplerPreviewCirclePosition::Center) return circleRect;

            constexpr qreal OFFSET = 10.0;
            constexpr qreal OFFSET_ABOVE = 20.0;
            bool mirrored = canvas->xAxisMirrored();
            bool flipped = canvas->yAxisMirrored();
            KisConfig::ColorSamplerPreviewCirclePosition effectivePos;
            if (mirrored && circlePreviewPosition == KisConfig::ColorSamplerPreviewCirclePosition::TopLeft) {
                effectivePos = KisConfig::ColorSamplerPreviewCirclePosition::TopRight;
            }
            else if (mirrored && circlePreviewPosition == KisConfig::ColorSamplerPreviewCirclePosition::TopRight) {
                effectivePos = KisConfig::ColorSamplerPreviewCirclePosition::TopLeft;
            }
            else effectivePos = circlePreviewPosition;

            qreal x = 0;
            qreal y = flipped ? circlePreviewDiameter / 2.0 + OFFSET_ABOVE : -circlePreviewDiameter / 2.0 - OFFSET_ABOVE;

            switch (effectivePos) {
            case KisConfig::ColorSamplerPreviewCirclePosition::TopLeft:
                x = -circlePreviewDiameter / 2.0 - OFFSET;
                break;
            case KisConfig::ColorSamplerPreviewCirclePosition::TopRight:
                x = circlePreviewDiameter / 2.0 + OFFSET;
                break;
            default:
                x = 0;
                break;
            }

            QTransform tf;

            qreal canvasRotationAngle = canvas->rotationAngle();
            if (!qFuzzyIsNull(canvasRotationAngle)) {
                tf.rotate(mirrored ? canvasRotationAngle : -canvasRotationAngle);
            }

            QPointF offset = tf.map(QPointF(x,y));

            circleRect.translate(offset.x(), offset.y());
        }

        return circleRect;
    }

    QRectF colorPreviewDocRect(const QPointF &outlineDocPoint)
    {
        QRectF colorPreviewViewRect;
        switch (style) {
        case KisConfig::ColorSamplerPreviewStyle::None:
            return QRectF();
        case KisConfig::ColorSamplerPreviewStyle::RectangleLeft:
        case KisConfig::ColorSamplerPreviewStyle::RectangleRight:
        case KisConfig::ColorSamplerPreviewStyle::RectangleAbove:
            colorPreviewViewRect = colorPreviewRectForRectangle();
            break;
        default:
            // Showing a preview without sampling a color (by just holding a
            // modifier) is used to compare the foreground color with the
            // canvas. The circle doesn't work well for that purpose, so we
            // use the handedness-independent rectangle above instead.
            if (haveSample) {
                colorPreviewViewRect = colorPreviewRectForCircle();
            } else {
                colorPreviewViewRect = colorPreviewRectForRectangle();
            }
            break;
        }

        const QRectF colorPreviewDocumentRect = converter().viewToDocument(colorPreviewViewRect);
        return colorPreviewDocumentRect.translated(outlineDocPoint);
    }

    QRect standardizeZoomPreviewPixelRect(QRect pixelRect) {
        // If width and height get rounded to different number, the center will be stuck between pixel border
        if (pixelRect.width() != pixelRect.height()) {
            if (pixelRect.width() % 2 == 0) pixelRect.setWidth(pixelRect.height());
            else pixelRect.setHeight(pixelRect.width());
        }
        // If width and height is even, the center will be stuck in pixel corner
        else if (pixelRect.width() % 2 == 0) {
            pixelRect.setWidth(pixelRect.width() - 1);
            pixelRect.setHeight(pixelRect.height() - 1);
        }

        // Minimum sample size should be 3 to be meaningful
        if (pixelRect.width() < 3 || pixelRect.height() < 3) pixelRect.setSize(QSize(3, 3));

        return pixelRect;
    }

    QPainterPath crosshairForOffsettedCircle() {
        if (!cacheCrosshairPath.isEmpty() && cacheCirclePreviewDiameter == circlePreviewDiameter) return cacheCrosshairPath;

        cacheCirclePreviewDiameter = circlePreviewDiameter;

        int gap = 5; // Should be odd so it have a center point
        int crosshairRadius = circlePreviewDiameter / 2 * 0.3;
        cacheCrosshairPath = QPainterPath(QPointF(-crosshairRadius, 0));

        cacheCrosshairPath.lineTo(-gap, 0);
        cacheCrosshairPath.moveTo(gap, 0);
        cacheCrosshairPath.lineTo(crosshairRadius, 0);

        QTransform tf;
        tf.translate(0, 0);
        tf.rotate(90);

        cacheCrosshairPath.moveTo(0,0);

        cacheCrosshairPath.addPath(tf.map(cacheCrosshairPath));

        return cacheCrosshairPath;
    }
};

KisAsyncColorSamplerHelper::KisAsyncColorSamplerHelper(KisCanvas2 *canvas)
    : m_d(new Private(canvas))
{
    using namespace std::placeholders; // For _1 placeholder
    std::function<void(QPointF)> callback =
        std::bind(&KisAsyncColorSamplerHelper::slotAddSamplingJob, this, _1);
    m_d->samplingCompressor.reset(
        new Private::SamplingCompressor(100, callback, KisSignalCompressor::FIRST_ACTIVE));

    m_d->activationDelayTimer.setInterval(100);
    m_d->activationDelayTimer.setSingleShot(true);
    connect(&m_d->activationDelayTimer, SIGNAL(timeout()), this, SLOT(activateDelayedPreview()));
    connect(m_d->canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()), this, SLOT(slotUpdateBgColor()));
    slotUpdateBgColor();
}

KisAsyncColorSamplerHelper::~KisAsyncColorSamplerHelper()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->strokeId);
}

bool KisAsyncColorSamplerHelper::isActive() const
{
    return m_d->isActive;
}

void KisAsyncColorSamplerHelper::activate(bool sampleCurrentLayer, bool pickFgColor)
{

    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_d->isActive);
    m_d->isActive = true;

    m_d->sampleResourceId =
        pickFgColor ?
            KoCanvasResource::ForegroundColor :
            KoCanvasResource::BackgroundColor;

    m_d->sampleCurrentLayer = sampleCurrentLayer;
    m_d->haveSample = false;


    KisConfig cfg(true);
    m_d->style = cfg.colorSamplerPreviewStyle();

    m_d->circlePreviewDiameter = cfg.colorSamplerPreviewCircleDiameter();
    m_d->circlePreviewThickness = cfg.colorSamplerPreviewCircleThickness()/100.0; // saved in percentages
    m_d->circlePreviewOutlineEnabled = cfg.colorSamplerPreviewCircleOutlineEnabled();
    m_d->circlePreviewExtraCircles = cfg.colorSamplerPreviewCircleExtraCirclesEnabled();
    m_d->circleZoomPreviewEnabled = cfg.colorSamplerZoomPreviewEnabled();
    m_d->circleZoomPreviewScale = cfg.colorSamplerZoomPreviewScale();
    m_d->circlePreviewPosition = cfg.colorSamplerPreviewCirclePosition();

    m_d->activationDelayTimer.start();
}

void KisAsyncColorSamplerHelper::activateDelayedPreview()
{
    // the event may come after we have started or even
    // finished color picking if the user is quick
    if (!m_d->isActive || m_d->showPreview) {
        return;
    }

    activatePreview();

    Q_EMIT sigRequestUpdateOutline();
}

void KisAsyncColorSamplerHelper::activatePreview()
{
    m_d->activationDelayTimer.stop();
    m_d->showPreview = true;

    const KoColor currentColor =
        m_d->canvas->resourceManager()->koColorResource(m_d->sampleResourceId);
    const QColor previewColor = m_d->canvas->displayColorConverter()->convertColorToDisplayColorSpace(currentColor, true);

    m_d->currentColor = previewColor;
    m_d->baseColor = previewColor;
    m_d->cache = QPixmap();

    updateCursor(m_d->sampleCurrentLayer, m_d->sampleResourceId == KoCanvasResource::ForegroundColor);
}

void KisAsyncColorSamplerHelper::updateCursor(bool sampleCurrentLayer, bool pickFgColor)
{
    const int sampleResourceId =
            pickFgColor ?
                KoCanvasResource::ForegroundColor :
                KoCanvasResource::BackgroundColor;

    QCursor cursor;

    if (sampleCurrentLayer) {
        if (sampleResourceId == KoCanvasResource::ForegroundColor) {
            cursor = KisCursor::samplerLayerForegroundCursor();
        } else {
            cursor = KisCursor::samplerLayerBackgroundCursor();
        }
    } else {
        if (sampleResourceId == KoCanvasResource::ForegroundColor) {
            cursor = KisCursor::samplerImageForegroundCursor();
        } else {
            cursor = KisCursor::samplerImageBackgroundCursor();
        }
    }

    Q_EMIT sigRequestCursor(cursor);
}

void KisAsyncColorSamplerHelper::setUpdateGlobalColor(bool value)
{
    m_d->updateGlobalColor = value;
}

bool KisAsyncColorSamplerHelper::updateGlobalColor() const
{
    return m_d->updateGlobalColor;
}

void KisAsyncColorSamplerHelper::deactivate()
{
    KIS_SAFE_ASSERT_RECOVER(!m_d->strokeId) {
        endAction();
    }

    m_d->activationDelayTimer.stop();

    m_d->showPreview = false;
    m_d->haveSample = false;

    m_d->previewDocRect = QRectF();
    m_d->currentColor = QColor();
    m_d->baseColor = QColor();
    m_d->cache = QPixmap();

    // Reset the cached zoom preview image and rect
    m_d->cacheCanvasPreviewImage = QImage();
    m_d->cacheCanvasPreviewRect = QRect();

    m_d->isActive = false;

    Q_EMIT sigRequestCursorReset();
    Q_EMIT sigRequestUpdateOutline();
}

void KisAsyncColorSamplerHelper::startAction(const QPointF &docPoint, int radius, int blend)
{
    KisColorSamplerStrokeStrategy *strategy = new KisColorSamplerStrokeStrategy(radius, blend);
    connect(strategy, &KisColorSamplerStrokeStrategy::sigColorUpdated,
            this, &KisAsyncColorSamplerHelper::slotColorSamplingFinished);
    connect(strategy, &KisColorSamplerStrokeStrategy::sigFinalColorSelected,
            this, &KisAsyncColorSamplerHelper::sigFinalColorSelected);

    activatePreview();
    m_d->haveSample = true;
    m_d->strokeId = m_d->strokesFacade()->startStroke(strategy);
    m_d->samplingCompressor->start(docPoint);
}

void KisAsyncColorSamplerHelper::continueAction(const QPointF &docPoint)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->strokeId);
    m_d->samplingCompressor->start(docPoint);
}

void KisAsyncColorSamplerHelper::endAction()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->strokeId);

    m_d->strokesFacade()->addJob(m_d->strokeId,
        new KisColorSamplerStrokeStrategy::FinalizeData());

    m_d->strokesFacade()->endStroke(m_d->strokeId);
    m_d->strokeId.clear();
}

QRectF KisAsyncColorSamplerHelper::colorPreviewDocRect(const QPointF &docPoint)
{
    if (!m_d->showPreview) return QRectF();

    m_d->sampleDocPoint = docPoint;

    KisConfig cfg(true);
    m_d->style = cfg.colorSamplerPreviewStyle();
    m_d->previewDocRect = m_d->colorPreviewDocRect(docPoint);
    return m_d->previewDocRect;
}

void KisAsyncColorSamplerHelper::paint(QPainter &gc, const KoViewConverter &converter)
{
    if (!m_d->showPreview) {
        return;
    }

    QRectF viewRectF = converter.documentToView(m_d->previewDocRect);
    QColor currentColor = colorWithAlpha(m_d->currentColor, OPACITY_OPAQUE_U8);
    QColor baseColor = m_d->haveSample ? colorWithAlpha(m_d->baseColor, OPACITY_OPAQUE_U8) : currentColor;

    switch (m_d->style) {
    case KisConfig::ColorSamplerPreviewStyle::RectangleLeft:
    case KisConfig::ColorSamplerPreviewStyle::RectangleRight:
    case KisConfig::ColorSamplerPreviewStyle::RectangleAbove:
        paintRectangle(gc, viewRectF, currentColor, baseColor);
        break;
    default:
        // See comment in colorPreviewDocRect.
        if (m_d->haveSample) {
            paintCircle(gc, viewRectF, currentColor, baseColor);
        } else {
            paintRectangle(gc, viewRectF, currentColor, baseColor);
        }
        break;
    }
}

void KisAsyncColorSamplerHelper::paintRectangle(QPainter &gc,
                                                const QRectF &viewRectF,
                                                const QColor &currentColor,
                                                const QColor &baseColor)
{
    qreal dpr = gc.device()->devicePixelRatioF();
    QSizeF cacheSizeF = viewRectF.size() * dpr;
    QSize cacheSize(qCeil(cacheSizeF.width()), qCeil(cacheSizeF.height()));
    bool needsNewCache = m_d->cache.isNull() || m_d->cache.size() != cacheSize;
    if (needsNewCache) {
        m_d->cache = QPixmap(cacheSize);
        m_d->cache.fill(Qt::transparent);
    }

    qreal canvasRotationAngle = m_d->canvas->rotationAngle();
    bool canvasMirror = m_d->canvas->xAxisMirrored();
    if (needsNewCache || !qFuzzyCompare(canvasRotationAngle, m_d->cacheRotation) || canvasMirror != m_d->cacheMirror) {
        m_d->cacheRotation = canvasRotationAngle;
        m_d->cacheMirror = canvasMirror;

        QPainter cachePainter(&m_d->cache);
        cachePainter.setRenderHint(QPainter::Antialiasing);

        qreal size = Private::PREVIEW_RECT_SIZE * dpr;
        QRectF rect(0.0, 0.0, m_d->haveSample ? size * 2.0 : size, size);
        rect.moveTopLeft(-rect.center());

        QTransform tf;
        QPointF offset = QRectF(m_d->cache.rect()).center();
        tf.translate(offset.x(), offset.y());
        tf.rotate(canvasMirror ? canvasRotationAngle : -canvasRotationAngle);
        cachePainter.setTransform(tf);

        if (m_d->haveSample) {
            qreal centerX = rect.center().x();
            QRectF currentRect(rect.topLeft(), QPointF(centerX + 1.0, rect.bottom()));
            QRectF baseRect(QPointF(centerX, rect.top()), rect.bottomRight());
            if (m_d->canvas->xAxisMirrored()) {
                std::swap(currentRect, baseRect);
            }
            cachePainter.fillRect(currentRect, currentColor);
            cachePainter.fillRect(baseRect, baseColor);
        } else {
            cachePainter.fillRect(rect, currentColor);
        }
    }

    gc.drawPixmap(viewRectF.toRect(), m_d->cache);
}

void KisAsyncColorSamplerHelper::paintCircle(QPainter &gc,
                                             const QRectF &viewRectF,
                                             const QColor &currentColor,
                                             const QColor &baseColor)
{
    if (!m_d->haveSample) {
        return;
    }

    gc.save();

    qreal dpr = gc.device()->devicePixelRatioF();
    QSizeF cacheSizeF = viewRectF.size() * dpr;
    QSize cacheSize(qCeil(cacheSizeF.width()), qCeil(cacheSizeF.height()));
    bool needsNewCache = m_d->cache.isNull() || m_d->cache.size() != cacheSize;
    if (needsNewCache) {
        m_d->cache = QPixmap(cacheSize);
        m_d->cache.fill(Qt::transparent);
    }

    qreal canvasRotationAngle = m_d->canvas->rotationAngle();
    if (m_d->canvas->xAxisMirrored()) {
        canvasRotationAngle = -canvasRotationAngle;
    }

    QPainter cachePainter(&m_d->cache);
    cachePainter.setRenderHint(QPainter::Antialiasing);

    QRectF cacheRect = m_d->cache.rect();

    // The color sampler preview is an outline and those rotate along
    // with the canvas. That's undesirable for the sampler preview
    // though, so we calculate the transformation to counter the rotation here
    QPointF cacheCenter = cacheRect.center();
    QTransform tf;
    tf.translate(cacheCenter.x(), cacheCenter.y());
    tf.rotate(-canvasRotationAngle);
    tf.translate(-cacheCenter.x(), -cacheCenter.y());

    bool needsDualColor = currentColor != baseColor;
    if (needsNewCache || (needsDualColor && !qFuzzyCompare(m_d->cacheRotation, canvasRotationAngle))) {
        m_d->cacheRotation = canvasRotationAngle;

        QColor backgroundColor = colorWithAlpha(m_d->backgroundColor, OPACITY_OPAQUE_U8 / 2 + 1);
        qreal penWidth = m_d->circlePreviewDiameter > 100 ? (2.0 * dpr) : (1.0 * dpr);
        QPen pen = QPen(backgroundColor, penWidth);
        if (m_d->circlePreviewOutlineEnabled) {
            cachePainter.setPen(pen);
        } else {
            cachePainter.setPen(Qt::NoPen);
        }

        QRectF outerRect = cacheRect.marginsRemoved(QMarginsF(penWidth, penWidth, penWidth, penWidth));

        if (needsDualColor) {
            // The color sampler preview is an outline and those rotate along
            // with the canvas. That's undesirable for the sampler preview
            // though, so we un-rotate its contents here accordingly.

            QPainterPath clipPath;
            clipPath.addPolygon(tf.map(QPolygonF(QRectF(0, 0, cacheRect.width(), cacheRect.height() / 2.0 + 1.0))));
            cachePainter.setClipPath(clipPath);

            bool flipped = m_d->canvas->yAxisMirrored();
            cachePainter.setBrush(flipped ? baseColor : currentColor);
            cachePainter.drawEllipse(outerRect);

            cachePainter.setBrush(baseColor);
            clipPath.clear();
            clipPath.addPolygon(
                tf.map(QRectF(0, cacheRect.height() / 2.0, cacheRect.width(), cacheRect.height() / 2.0)));
            cachePainter.setClipPath(clipPath);

            cachePainter.setBrush(flipped ? currentColor : baseColor);
            cachePainter.drawEllipse(outerRect);

            cachePainter.setClipPath(QPainterPath(), Qt::NoClip);
        } else {
            cachePainter.setBrush(currentColor);
            cachePainter.drawEllipse(outerRect);
        }

        qreal innerX = cacheRect.width() * (1.0 - m_d->circlePreviewThickness);
        qreal innerY = cacheRect.height() * (1.0 - m_d->circlePreviewThickness);
        QRectF innerRect = cacheRect.marginsRemoved(QMarginsF(innerX, innerY, innerX, innerY));
        QPainterPath innerEllipse;
        innerEllipse.addEllipse(innerRect);

        QPainterPath innerPath;
        innerPath.addPath(innerEllipse);

        if (m_d->circlePreviewThickness < 0.5 && m_d->circlePreviewExtraCircles) {
            qreal extraMargin = 0.1*m_d->circlePreviewThickness*innerRect.width(); // looks better
            QPointF leftCenter = QPointF(innerRect.left() - extraMargin, innerRect.top() + innerRect.height()/2.0);
            QPointF rightCenter = QPointF(innerRect.right() + extraMargin, innerRect.top() + innerRect.height()/2.0);

            innerPath.setFillRule(Qt::OddEvenFill);
            innerPath.addEllipse(leftCenter, m_d->circlePreviewThickness*cacheRect.width(), m_d->circlePreviewThickness*cacheRect.width());
            innerPath.addEllipse(rightCenter, m_d->circlePreviewThickness*cacheRect.width(), m_d->circlePreviewThickness*cacheRect.width());

            innerPath = innerPath.intersected(innerEllipse);
        }

        m_d->cacheCircleInnerClip = innerPath;

        // Draw inner circle outline if enabled
        if (m_d->circlePreviewOutlineEnabled) {
            cachePainter.setBrush(Qt::transparent);
            cachePainter.setPen(pen);
            cachePainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            cachePainter.drawPath(tf.map(m_d->cacheCircleInnerClip));
        }
    }

    // Clear the center if no zoom
    // Or fill with a solid color to hide the underneath view when zooming at edge of canvas
    cachePainter.setPen(Qt::NoPen);
    // If the cache update don't run, no brush is set. So set it
    cachePainter.setBrush(m_d->backgroundColor);
    if (m_d->circleZoomPreviewEnabled){
        cachePainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
    else {
        cachePainter.setCompositionMode(QPainter::CompositionMode_Clear);
    }
    cachePainter.drawPath(tf.map(m_d->cacheCircleInnerClip));

    // Draw zoom preview
    if (m_d->circleZoomPreviewEnabled) {
        QRectF zoomDocRectF = m_d->previewDocRect;

        zoomDocRectF.setSize(zoomDocRectF.size() / m_d->circleZoomPreviewScale);
        zoomDocRectF.moveCenter(m_d->sampleDocPoint);

        paintCircleCanvasPreview(cachePainter, cacheRect, zoomDocRectF, tf.map(m_d->cacheCircleInnerClip));

        paintCircleReferenceImagePreview(cachePainter, cacheRect, zoomDocRectF, tf.map(m_d->cacheCircleInnerClip));


        // Draw crosshair if preview is offseted
        if (m_d->circlePreviewPosition != KisConfig::ColorSamplerPreviewCirclePosition::Center) {
            QColor crosshairColor = Qt::black;
            // Apparently this fomular is outdated and inaccurate. But the accurate version require calculating power, probably overkill anyway
            qreal luminance = (0.299 * currentColor.redF() + 0.587 * currentColor.greenF() + 0.114 * currentColor.blueF());
            if (luminance < 0.5) crosshairColor = Qt::white;

            cachePainter.save();

            cachePainter.setPen(crosshairColor);
            QTransform tf;
            tf.translate(cacheCenter.x(), cacheCenter.y());
            tf.rotate(-canvasRotationAngle);
            cachePainter.drawPath(tf.map(m_d->crosshairForOffsettedCircle()));

            cachePainter.restore();
        }
    }

    gc.drawPixmap(viewRectF.toRect(), m_d->cache);

    gc.restore();
}

QImage KisAsyncColorSamplerHelper::cacheCanvasImage(QRect &canvasPixelRect) {
    KisImageWSP canvasImage = m_d->canvas->image();

    if (!canvasImage->bounds().intersects(canvasPixelRect)) {
        return QImage();
    }

    // If already cached the whole canvas, just use it from now on
    if (m_d->cacheCanvasPreviewRect == canvasImage->bounds()) {
        return m_d->cacheCanvasPreviewImage;
    }

    if (m_d->cacheCanvasPreviewRect.isEmpty() || !m_d->cacheCanvasPreviewRect.contains(canvasPixelRect)) {
        // Cache an area larger than the needed preview area to avoid rapid small dynamic allocations
        // Attempt to reduce cache size when sample size is big to reduce delay
        qreal cacheScale = 1;
        if (canvasPixelRect.width() < 250) cacheScale = 4;
        else if (canvasPixelRect.width() < 500) cacheScale = 2;

        QRect cacheCanvasRect = canvasPixelRect;
        cacheCanvasRect.setSize(canvasPixelRect.size() * cacheScale);
        cacheCanvasRect.moveCenter(canvasPixelRect.center());

        // if cache larger than canvas, then just copy whole canvas
        if (cacheCanvasRect.width() >= canvasImage->width() || cacheCanvasRect.height() >= canvasImage->height()) {
            cacheCanvasRect = canvasImage->bounds();
        }

        m_d->cacheCanvasPreviewImage = canvasImage->convertToQImage(cacheCanvasRect, canvasImage->profile());

        m_d->cacheCanvasPreviewRect = cacheCanvasRect;
    }

    canvasPixelRect.translate(-m_d->cacheCanvasPreviewRect.topLeft());

    return m_d->cacheCanvasPreviewImage;
}

void KisAsyncColorSamplerHelper::paintCircleCanvasPreview(QPainter &gc, const QRectF &viewRectF, const QRectF &zoomDocRectF, const QPainterPath &clip) {
    KisImageWSP image = m_d->canvas->image();

    QRect canvasPixelRect = image->documentToPixel(zoomDocRectF).toRect();
    canvasPixelRect = m_d->standardizeZoomPreviewPixelRect(canvasPixelRect);

    // Make sure the center is the pixel currently sampled (in case of rounding errors)
    canvasPixelRect.moveCenter(image->documentToImagePixelFloored(zoomDocRectF.center()));

    QImage cachedImage = cacheCanvasImage(canvasPixelRect);
    if (cachedImage.isNull()) return;

    gc.save();
    gc.setCompositionMode(QPainter::CompositionMode_SourceOver);

    gc.setClipPath(clip);

    // QtDoc: The image is scaled to fit the rectangle, if both the image and rectangle size disagree.
    // Since the piece of canvas is (zoomPreviewScale) times smaller than cacheRect
    // drawImage will scale it up that many times, thus achieving the zoom effect
    gc.drawImage(viewRectF, cachedImage, canvasPixelRect);

    gc.restore();
}

// Won't show opacity because the color sampled doesn't respect opacity either
// TODO: Test if reference image still show when not visible, opacity = 0. Maybe we need to cover that too
void KisAsyncColorSamplerHelper::paintCircleReferenceImagePreview(QPainter &gc, const QRectF &viewRectF, const QRectF &zoomDocRectF, const QPainterPath &clip) {
    KisDocument *document = m_d->canvas->viewManager()->document();
    if (!document || !document->referenceImagesLayer()) return;

    gc.save();

    gc.setCompositionMode(QPainter::CompositionMode_SourceOver);
    gc.setClipPath(clip);

    QList<KoShape*> shapeList = document->referenceImagesLayer()->shapeManager()->shapesAt(zoomDocRectF);
    if (shapeList.size() > 1) std::sort(shapeList.begin(), shapeList.end(), KoShape::compareShapeZIndex);

    Q_FOREACH(KoShape *shape, shapeList) {
        KisReferenceImage *refImage = dynamic_cast<KisReferenceImage*>(shape);
        if (!refImage) continue;

        gc.save();

        QImage image = refImage->getCachedImage();

        QPoint refPixelPoint = refImage->documentToPixelFloored(zoomDocRectF.center());

        // Avoid using KisReferenceImage::documentToPixel because it use absoluteTransformation.invert()
        // Which will take rotation into account and cause some quirks
        qreal xScale = refImage->boundingRect().width() / image.width();
        qreal yScale = refImage->boundingRect().height() / image.height();

        QRect refPixelRect = QRect(refPixelPoint, QSize(zoomDocRectF.width() / xScale, zoomDocRectF.height() / yScale));
        refPixelRect = m_d->standardizeZoomPreviewPixelRect(refPixelRect);

        refPixelRect.moveCenter(refPixelPoint);

        // Rotate the painter, draw, rotate back is seemingly easier than
        // trying to rotate the image and the shenanighens that follow
        gc.translate(viewRectF.center());
        gc.rotate(refImage->rotation());
        gc.translate(-viewRectF.center());

        gc.drawImage(viewRectF, image, refPixelRect);

        gc.restore();
    }

    gc.restore();
}

void KisAsyncColorSamplerHelper::slotAddSamplingJob(const QPointF &docPoint)
{
    /**
     * The actual sampling is delayed by a compressor, so we can get this
     * event when the stroke is already closed
     */
    if (!m_d->strokeId) return;

    KisImageSP image = m_d->canvas->image();
    const QPoint imagePoint = image->documentToImagePixelFloored(docPoint);

    if (!m_d->sampleCurrentLayer) {
        KisSharedPtr<KisReferenceImagesLayer> referencesLayer = m_d->canvas->imageView()->document()->referenceImagesLayer();
        if (referencesLayer && m_d->canvas->referenceImagesDecoration()->visible()) {
            KoColor color = referencesLayer->getPixel(docPoint);
            if (color.opacityU8() > 0) {
                slotColorSamplingFinished(color);
                return;
            }
        }
    }

    KisPaintDeviceSP device = m_d->sampleCurrentLayer ?
        m_d->canvas->imageView()->currentNode()->colorSampleSourceDevice() :
        image->projection();

    if (device) {
        // Used for color sampler blending.
        const KoColor currentColor =
            m_d->canvas->resourceManager()->koColorResource(m_d->sampleResourceId);

        m_d->strokesFacade()->addJob(m_d->strokeId,
            new KisColorSamplerStrokeStrategy::Data(device, imagePoint, currentColor));
    } else {
        QString message = i18n("Color sampler does not work on this layer.");
        m_d->canvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
    }
}

void KisAsyncColorSamplerHelper::slotColorSamplingFinished(const KoColor &rawColor)
{
    KoColor color(rawColor);

    color.setOpacity(OPACITY_OPAQUE_U8);

    if (m_d->updateGlobalColor) {
        m_d->canvas->resourceManager()->setResource(m_d->sampleResourceId, color);
    }

    Q_EMIT sigRawColorSelected(rawColor);
    Q_EMIT sigColorSelected(color);

    if (!m_d->showPreview) return;

    const QColor previewColor = m_d->canvas->displayColorConverter()->convertColorToDisplayColorSpace(color, true);

    if (!m_d->haveSample || m_d->currentColor != previewColor) {
        m_d->haveSample = true;
        m_d->currentColor = previewColor;
        m_d->cache = QPixmap();
    }

    Q_EMIT sigRequestUpdateOutline();
}

void KisAsyncColorSamplerHelper::slotUpdateBgColor()
{
    KoColor bgColor;
    bgColor.fromQColor(qApp->palette().color(QPalette::Base));
    m_d->backgroundColor = m_d->canvas->displayColorConverter()->convertColorToDisplayColorSpace(bgColor);
}
