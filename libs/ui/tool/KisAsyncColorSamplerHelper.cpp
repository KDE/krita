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
bool colorPreviewHasForegroundComparison(KisConfig::ColorSamplerPreviewStyle style)
{
    // Not all styles show the foreground color when sampling is pending. It
    // would probably be cleaner if that were its own action instead of being
    // tied to the color sampler activation and show the comparison *on* the
    // cursor instead of it being off to the side, but it'd take some effort
    // to implement the separation of that.
    switch (style) {
    case KisConfig::ColorSamplerPreviewStyle::RectangleLeft:
    case KisConfig::ColorSamplerPreviewStyle::RectangleRight:
        return true;
    default:
        return false;
    }
}

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

    QColor currentColor;
    QColor baseColor;

    QPixmap cache;
    qreal cacheRotation = 0.0;
    bool cacheMirror = false;

    KisStrokesFacade *strokesFacade() const {
        return canvas->image().data();
    }

    const KoViewConverter &converter() const {
        return *canvas->imageView()->viewConverter();
    }

    QRectF colorPreviewRectForRectangle(bool left)
    {
        constexpr qreal OFFSET = 32.0;
        constexpr qreal SIZE = PREVIEW_RECT_SIZE;

        bool mirrored = canvas->xAxisMirrored();
        if (mirrored) {
            left = !left;
        }

        qreal width = haveSample ? SIZE * 2.0 : SIZE;
        qreal x = left ? -(OFFSET + width) : OFFSET;
        qreal y = canvas->yAxisMirrored() ? -(OFFSET + SIZE) : OFFSET;
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
        return QRectF(-circlePreviewDiameter / 2.0, -circlePreviewDiameter / 2.0, circlePreviewDiameter, circlePreviewDiameter);
    }

    QRectF colorPreviewDocRect(const QPointF &outlineDocPoint)
    {
        QRectF colorPreviewViewRect;
        switch (style) {
        case KisConfig::ColorSamplerPreviewStyle::None:
            return QRectF();
        case KisConfig::ColorSamplerPreviewStyle::RectangleLeft:
            colorPreviewViewRect = colorPreviewRectForRectangle(true);
            break;
        case KisConfig::ColorSamplerPreviewStyle::RectangleRight:
            colorPreviewViewRect = colorPreviewRectForRectangle(false);
            break;
        default:
            if (!haveSample) {
                return QRectF();
            }
            colorPreviewViewRect = colorPreviewRectForCircle();
            break;
        }

        const QRectF colorPreviewDocumentRect = converter().viewToDocument(colorPreviewViewRect);
        return colorPreviewDocumentRect.translated(outlineDocPoint);
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


    if (colorPreviewHasForegroundComparison(m_d->style)) {
        m_d->activationDelayTimer.start();
    } else {
        activatePreview();
    }
}

void KisAsyncColorSamplerHelper::activateDelayedPreview()
{
    // the event may come after we have finished color
    // picking if the user is quick
    if (!m_d->isActive) return;

    activatePreview();

    Q_EMIT sigRequestUpdateOutline();
}

void KisAsyncColorSamplerHelper::activatePreview()
{
    m_d->showPreview = true;

    const KoColor currentColor =
        m_d->canvas->resourceManager()->koColorResource(m_d->sampleResourceId);
    const QColor previewColor = m_d->canvas->displayColorConverter()->toQColor(currentColor);

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
        paintRectangle(gc, viewRectF, currentColor, baseColor);
        break;
    default:
        paintCircle(gc, viewRectF, currentColor, baseColor);
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

    bool needsDualColor = currentColor != baseColor;
    if (needsNewCache || (needsDualColor && !qFuzzyCompare(m_d->cacheRotation, canvasRotationAngle))) {
        m_d->cacheRotation = canvasRotationAngle;

        QPainter cachePainter(&m_d->cache);
        cachePainter.setRenderHint(QPainter::Antialiasing);

        QColor backgroundColor = colorWithAlpha(qApp->palette().color(QPalette::Base), OPACITY_OPAQUE_U8 / 2 + 1);
        qreal penWidth = m_d->circlePreviewDiameter > 100 ? (2.0 * dpr) : (1.0 * dpr);
        QPen pen = QPen(backgroundColor, penWidth);
        if (m_d->circlePreviewOutlineEnabled) {
            cachePainter.setPen(pen);
        } else {
            cachePainter.setPen(Qt::NoPen);
        }

        QRectF cacheRect = m_d->cache.rect();
        QRectF outerRect = cacheRect.marginsRemoved(QMarginsF(penWidth, penWidth, penWidth, penWidth));

        QTransform tf;

        QPointF cacheCenter = cacheRect.center();
        tf.translate(cacheCenter.x(), cacheCenter.y());
        tf.rotate(-canvasRotationAngle);
        tf.translate(-cacheCenter.x(), -cacheCenter.y());


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

        cachePainter.setPen(Qt::NoPen);
        cachePainter.setCompositionMode(QPainter::CompositionMode_Clear);
        cachePainter.drawPath(tf.map(innerPath));

        if (m_d->circlePreviewOutlineEnabled) {
            cachePainter.setBrush(Qt::transparent);
            cachePainter.setPen(pen);
            cachePainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            cachePainter.drawPath(tf.map(innerPath));
        }
    }
    gc.drawPixmap(viewRectF.toRect(), m_d->cache);

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
            QColor color = referencesLayer->getPixel(imagePoint);
            if (color.isValid() && color.alpha() != 0) {
                slotColorSamplingFinished(KoColor(color, image->colorSpace()));
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

    const QColor previewColor = m_d->canvas->displayColorConverter()->toQColor(color);

    if (!m_d->haveSample || m_d->currentColor != previewColor) {
        m_d->haveSample = true;
        m_d->currentColor = previewColor;
        m_d->cache = QPixmap();
    }

    Q_EMIT sigRequestUpdateOutline();
}
