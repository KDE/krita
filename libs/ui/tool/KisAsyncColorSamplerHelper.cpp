/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAsyncColorSamplerHelper.h"

#include <QPainter>
#include "kis_signal_compressor_with_param.h"
#include "KoCanvasResourcesIds.h"
#include <KoViewConverter.h>
#include "kis_image_interfaces.h"
#include <strokes/kis_color_sampler_stroke_strategy.h>
#include "kis_config.h"

#include "kis_canvas2.h"
#include "KisDocument.h"
#include "KisReferenceImagesLayer.h"
#include "KisReferenceImagesDecoration.h"
#include "KoCanvasResourceProvider.h"
#include "KisViewManager.h"
#include "KoIcon.h"
#include "kis_display_color_converter.h"
#include <kis_cursor.h>

namespace {
std::pair<QRectF,QRectF> colorPreviewDocRectImpl(const QPointF &outlineDocPoint, bool colorPreviewShowComparePlate, const KoViewConverter *converter)
{
    KisConfig cfg(true);
    const QRectF colorPreviewViewRect = cfg.colorPreviewRect();

    const QRectF colorPreviewBaseColorViewRect =
        colorPreviewShowComparePlate ?
            colorPreviewViewRect.translated(colorPreviewViewRect.width(), 0) :
            QRectF();

    const QRectF colorPreviewDocumentRect = converter->viewToDocument(colorPreviewViewRect);
    const QRectF colorPreviewBaseColorDocumentRect =
        converter->viewToDocument(colorPreviewBaseColorViewRect);

    return std::make_pair(colorPreviewDocumentRect.translated(outlineDocPoint),
                          colorPreviewBaseColorDocumentRect.translated(outlineDocPoint));
}
}

struct KisAsyncColorSamplerHelper::Private
{
    Private(KisCanvas2 *_canvas)
        : canvas(_canvas)
    {}

    KisCanvas2 *canvas;
    bool isActive {false};
    bool showPreview {false};
    KisStrokeId samplerStrokeId;
    int samplingResource {0};
    bool sampleCurrentLayer {true};
    bool colorPreviewShowComparePlate {false};
    typedef KisSignalCompressorWithParam<QPointF> SamplingCompressor;
    QScopedPointer<SamplingCompressor> colorSamplingCompressor;

    QTimer colorSamplerDelayTimer;

    QRectF colorPreviewDocRect;
    QRectF colorPreviewBaseColorDocRect;

    QColor colorPreviewCurrentColor;
    QColor colorPreviewBaseColor;

    KisStrokesFacade *strokesFacade() {
        return canvas->image().data();
    }

    const KoViewConverter &converter() {
        return *canvas->imageView()->viewConverter();
    }

};

KisAsyncColorSamplerHelper::KisAsyncColorSamplerHelper(KisCanvas2 *canvas)
    : m_d(new Private(canvas))
{
    using namespace std::placeholders; // For _1 placeholder
    std::function<void(QPointF)> callback =
        std::bind(&KisAsyncColorSamplerHelper::slotAddSamplingJob, this, _1);
    m_d->colorSamplingCompressor.reset(
        new Private::SamplingCompressor(100, callback, KisSignalCompressor::FIRST_ACTIVE));

    m_d->colorSamplerDelayTimer.setInterval(100);
    m_d->colorSamplerDelayTimer.setSingleShot(true);
    connect(&m_d->colorSamplerDelayTimer, SIGNAL(timeout()), this, SLOT(activateDelayedPreview()));
}

KisAsyncColorSamplerHelper::~KisAsyncColorSamplerHelper()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->samplerStrokeId);
}

bool KisAsyncColorSamplerHelper::isActive() const
{
    return m_d->isActive;
}

void KisAsyncColorSamplerHelper::activate(bool sampleCurrentLayer, bool pickFgColor)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_d->isActive);
    m_d->isActive = true;

    m_d->samplingResource =
        pickFgColor ?
            KoCanvasResource::ForegroundColor :
            KoCanvasResource::BackgroundColor;

    m_d->sampleCurrentLayer = sampleCurrentLayer;
    m_d->colorPreviewShowComparePlate = false;

    m_d->colorSamplerDelayTimer.start();
}

void KisAsyncColorSamplerHelper::activateDelayedPreview()
{
    // the event may come after we have finished color
    // picking if the user is quick
    if (!m_d->isActive) return;

    m_d->showPreview = true;

    const KoColor currentColor =
        m_d->canvas->resourceManager()->koColorResource(m_d->samplingResource);
    const QColor previewColor = m_d->canvas->displayColorConverter()->toQColor(currentColor);

    m_d->colorPreviewCurrentColor = previewColor;
    m_d->colorPreviewBaseColor = previewColor;

    QCursor cursor;

    if (m_d->sampleCurrentLayer) {
        if (m_d->samplingResource == KoCanvasResource::ForegroundColor) {
            cursor = KisCursor::samplerLayerForegroundCursor();
        } else {
            cursor = KisCursor::samplerLayerBackgroundCursor();
        }
    } else {
        if (m_d->samplingResource == KoCanvasResource::ForegroundColor) {
            cursor = KisCursor::samplerImageForegroundCursor();
        } else {
            cursor = KisCursor::samplerImageBackgroundCursor();
        }
    }

    Q_EMIT sigRequestCursor(cursor);
    Q_EMIT sigRequestUpdateOutline();
}

void KisAsyncColorSamplerHelper::deactivate()
{
    KIS_SAFE_ASSERT_RECOVER(!m_d->samplerStrokeId) {
        endAction();
    }

    m_d->colorSamplerDelayTimer.stop();

    m_d->showPreview = false;
    m_d->colorPreviewShowComparePlate = false;

    m_d->colorPreviewDocRect = QRectF();
    m_d->colorPreviewCurrentColor = QColor();
    m_d->colorPreviewBaseColor = QColor();
    m_d->colorPreviewBaseColorDocRect = QRectF();

    m_d->isActive = false;

    Q_EMIT sigRequestCursorReset();
    Q_EMIT sigRequestUpdateOutline();
}

void KisAsyncColorSamplerHelper::startAction(const QPointF &docPoint)
{
    KisColorSamplerStrokeStrategy *strategy = new KisColorSamplerStrokeStrategy();
    connect(strategy, &KisColorSamplerStrokeStrategy::sigColorUpdated,
            this, &KisAsyncColorSamplerHelper::slotColorSamplingFinished);

    m_d->samplerStrokeId = m_d->strokesFacade()->startStroke(strategy);
    m_d->colorSamplingCompressor->start(docPoint);
}

void KisAsyncColorSamplerHelper::continueAction(const QPointF &docPoint)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->samplerStrokeId);
    m_d->colorSamplingCompressor->start(docPoint);
}

void KisAsyncColorSamplerHelper::endAction()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->samplerStrokeId);
    m_d->strokesFacade()->endStroke(m_d->samplerStrokeId);
    m_d->samplerStrokeId.clear();
}

QRectF KisAsyncColorSamplerHelper::colorPreviewDocRect(const QPointF &docPoint)
{
    if (!m_d->showPreview) return QRectF();

    std::tie(m_d->colorPreviewDocRect, m_d->colorPreviewBaseColorDocRect) =
            colorPreviewDocRectImpl(docPoint, m_d->colorPreviewShowComparePlate, &m_d->converter());

    return m_d->colorPreviewDocRect | m_d->colorPreviewBaseColorDocRect;
}

void KisAsyncColorSamplerHelper::paint(QPainter &gc, const KoViewConverter &converter)
{
    if (!m_d->showPreview) return;

    const QRectF viewRect = converter.documentToView(m_d->colorPreviewDocRect);
    gc.fillRect(viewRect, m_d->colorPreviewCurrentColor);

    if (m_d->colorPreviewShowComparePlate) {
        const QRectF baseColorRect = converter.documentToView(m_d->colorPreviewBaseColorDocRect);
        gc.fillRect(baseColorRect, m_d->colorPreviewBaseColor);
    }
}

void KisAsyncColorSamplerHelper::slotAddSamplingJob(const QPointF &docPoint)
{
    /**
     * The actual sampling is delayed by a compressor, so we can get this
     * event when the stroke is already closed
     */
    if (!m_d->samplerStrokeId) return;

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
            m_d->canvas->resourceManager()->koColorResource(m_d->samplingResource);

        m_d->strokesFacade()->addJob(m_d->samplerStrokeId,
            new KisColorSamplerStrokeStrategy::Data(device, imagePoint, currentColor));
    } else {
        QString message = i18n("Color sampler does not work on this layer.");
        m_d->canvas->viewManager()->showFloatingMessage(message, koIcon("object-locked"));
    }
}

void KisAsyncColorSamplerHelper::slotColorSamplingFinished(const KoColor &_color)
{
    KoColor color(_color);

    color.setOpacity(OPACITY_OPAQUE_U8);
    m_d->canvas->resourceManager()->setResource(m_d->samplingResource, color);

    if (!m_d->showPreview) return;

    const QColor previewColor = m_d->canvas->displayColorConverter()->toQColor(color);

    m_d->colorPreviewShowComparePlate = true;
    m_d->colorPreviewCurrentColor = previewColor;

    Q_EMIT sigRequestUpdateOutline();
}
