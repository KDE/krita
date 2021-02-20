/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_fill_painter.h"

#include <stdlib.h>
#include <string.h>
#include <cfloat>
#include <stack>

#include <QFontInfo>
#include <QFontMetrics>
#include <QPen>
#include <QMatrix>
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QRect>
#include <QString>

#include <klocalizedstring.h>

#include <KoUpdater.h>

#include "generator/kis_generator.h"
#include "filter/kis_filter_configuration.h"
#include "generator/kis_generator_registry.h"
#include "kis_processing_information.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include <resources/KoPattern.h>
#include "KoColorSpace.h"
#include "kis_transaction.h"
#include "kis_pixel_selection.h"
#include <KoCompositeOpRegistry.h>
#include <floodfill/kis_scanline_fill.h>
#include "kis_selection_filters.h"
#include <kis_perspectivetransform_worker.h>

KisFillPainter::KisFillPainter()
        : KisPainter()
{
    initFillPainter();
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device)
        : KisPainter(device)
{
    initFillPainter();
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device, KisSelectionSP selection)
        : KisPainter(device, selection)
{
    initFillPainter();
}

void KisFillPainter::initFillPainter()
{
    m_width = m_height = -1;
    m_careForSelection = false;
    m_sizemod = 0;
    m_feather = 0;
    m_useCompositioning = false;
    m_threshold = 0;
    m_useSelectionAsBoundary = false;
}

void KisFillPainter::fillSelection(const QRect &rc, const KoColor &color)
{
    KisPaintDeviceSP fillDevice = new KisPaintDevice(device()->colorSpace());
    fillDevice->setDefaultPixel(color);

    bitBlt(rc.topLeft(), fillDevice, rc);
}

// 'regular' filling
// XXX: This also needs renaming, since filling ought to keep the opacity and the composite op in mind,
//      this is more eraseToColor.
void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KoColor& kc, quint8 opacity)
{
    if (w > 0 && h > 0) {
        // Make sure we're in the right colorspace

        KoColor kc2(kc); // get rid of const
        kc2.convertTo(device()->colorSpace());
        quint8 * data = kc2.data();
        device()->colorSpace()->setOpacity(data, opacity, 1);

        device()->fill(x1, y1, w, h, data);

        addDirtyRect(QRect(x1, y1, w, h));
    }
}

void KisFillPainter::fillRect(const QRect &rc, const KoPatternSP pattern, const QPoint &offset)
{
    fillRect(rc.x(), rc.y(), rc.width(), rc.height(), pattern, offset);
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KoPatternSP pattern, const QPoint &offset)
{
    if (!pattern) return;
    if (!pattern->valid()) return;
    if (!device()) return;
    if (w < 1) return;
    if (h < 1) return;

    KisPaintDeviceSP patternLayer = new KisPaintDevice(device()->compositionSourceColorSpace(), pattern->name());
    patternLayer->convertFromQImage(pattern->pattern(), 0);

    if (!offset.isNull()) {
        patternLayer->moveTo(offset);
    }

    fillRect(x1, y1, w, h, patternLayer, QRect(offset.x(), offset.y(), pattern->width(), pattern->height()));
}

void KisFillPainter::fillRectNoCompose(const QRect &rc, const KoPatternSP pattern, const QTransform transform)
{
    if (!pattern) return;
    if (!pattern->valid()) return;
    if (!device()) return;
    if (rc.width() < 1) return;
    if (rc.height() < 1) return;

    KisPaintDeviceSP patternLayer = new KisPaintDevice(device()->colorSpace(), pattern->name());
    patternLayer->convertFromQImage(pattern->pattern(), 0);

    fillRectNoCompose(rc.x(), rc.y(), rc.width(), rc.height(), patternLayer, QRect(0, 0, pattern->width(), pattern->height()), transform);
}

void KisFillPainter::fillRectNoCompose(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisPaintDeviceSP device, const QRect& deviceRect, const QTransform transform)
{
    /**
     * Since this function doesn't do any kind of compostiting, so the pixel size
     * of the source and destination devices must be exactly the same. The color
     * space should ideally be also the same.
     */
    KIS_SAFE_ASSERT_RECOVER_RETURN(device->pixelSize() == this->device()->pixelSize());
    KIS_SAFE_ASSERT_RECOVER_NOOP(*device->colorSpace() == *this->device()->colorSpace());

    KisPaintDeviceSP wrapped = device;
    KisDefaultBoundsBaseSP oldBounds = wrapped->defaultBounds();
    wrapped->setDefaultBounds(new KisWrapAroundBoundsWrapper(oldBounds, deviceRect));

    KisPerspectiveTransformWorker worker = KisPerspectiveTransformWorker(this->device(), transform, this->progressUpdater());
    worker.runPartialDst(device, this->device(), QRect(x1, y1, w, h));

    addDirtyRect(QRect(x1, y1, w, h));
    wrapped->setDefaultBounds(oldBounds);
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisPaintDeviceSP device, const QRect& deviceRect)
{
    const QRect &patternRect = deviceRect;
    const QRect fillRect(x1, y1, w, h);

    auto toPatternLocal = [](int value, int offset, int width) {
        const int normalizedValue = value - offset;
        return offset + (normalizedValue >= 0 ?
                         normalizedValue % width :
                         width - (-normalizedValue - 1) % width - 1);
    };

    int dstY = fillRect.y();
    while (dstY <= fillRect.bottom()) {
        const int dstRowsRemaining = fillRect.bottom() - dstY + 1;

        const int srcY = toPatternLocal(dstY, patternRect.y(), patternRect.height());
        const int height = qMin(patternRect.height() - srcY + patternRect.y(), dstRowsRemaining);

        int dstX = fillRect.x();
        while (dstX <= fillRect.right()) {
            const int dstColumnsRemaining = fillRect.right() - dstX + 1;

            const int srcX = toPatternLocal(dstX, patternRect.x(), patternRect.width());
            const int width = qMin(patternRect.width() - srcX  + patternRect.x(), dstColumnsRemaining);

            bitBlt(dstX, dstY, device, srcX, srcY, width, height);

            dstX += width;
        }
        dstY += height;
    }

    addDirtyRect(QRect(x1, y1, w, h));
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisFilterConfigurationSP generator)
{
    if (!generator) return;
    KisGeneratorSP g = KisGeneratorRegistry::instance()->value(generator->name());
    if (!device()) return;
    if (w < 1) return;
    if (h < 1) return;

    QRect tmpRc(x1, y1, w, h);

    KisProcessingInformation dstCfg(device(), tmpRc.topLeft(), 0);

    g->generate(dstCfg, tmpRc.size(), generator);

    addDirtyRect(tmpRc);
}

// flood filling

void KisFillPainter::fillColor(int startX, int startY, KisPaintDeviceSP sourceDevice)
{
    if (!m_useCompositioning) {
        if (m_sizemod || m_feather ||
            compositeOp()->id() != COMPOSITE_OVER ||
            opacity() != MAX_SELECTED ||
            sourceDevice != device()) {

            warnKrita << "WARNING: Fast Flood Fill (no compositioning mode)"
                       << "does not support compositeOps, opacity, "
                       << "selection enhancements and separate source "
                       << "devices";
        }

        QRect fillBoundsRect(0, 0, m_width, m_height);
        QPoint startPoint(startX, startY);

        if (!fillBoundsRect.contains(startPoint)) return;

        KisScanlineFill gc(device(), startPoint, fillBoundsRect);
        gc.setThreshold(m_threshold);
        gc.fillColor(paintColor());

    } else {
        genericFillStart(startX, startY, sourceDevice);

        // Now create a layer and fill it
        KisPaintDeviceSP filled = device()->createCompositionSourceDevice();
        Q_CHECK_PTR(filled);
        KisFillPainter painter(filled);
        painter.fillRect(0, 0, m_width, m_height, paintColor());
        painter.end();

        genericFillEnd(filled);
    }
}

void KisFillPainter::fillPattern(int startX, int startY, KisPaintDeviceSP sourceDevice, QTransform patternTransform)
{
    genericFillStart(startX, startY, sourceDevice);

    // Now create a layer and fill it
    KisPaintDeviceSP filled = device()->createCompositionSourceDevice();
    Q_CHECK_PTR(filled);
    KisFillPainter painter(filled);
    painter.fillRectNoCompose(QRect(0, 0, m_width, m_height), pattern(), patternTransform);
    painter.end();

    genericFillEnd(filled);
}

void KisFillPainter::genericFillStart(int startX, int startY, KisPaintDeviceSP sourceDevice)
{
    Q_ASSERT(m_width > 0);
    Q_ASSERT(m_height > 0);

    // Create a selection from the surrounding area

    KisPixelSelectionSP pixelSelection = createFloodSelection(startX, startY, sourceDevice,
                                                              (selection().isNull() ? 0 : selection()->pixelSelection()));
    KisSelectionSP newSelection = new KisSelection(pixelSelection->defaultBounds());
    newSelection->pixelSelection()->applySelection(pixelSelection, SELECTION_REPLACE);
    m_fillSelection = newSelection;
}

void KisFillPainter::genericFillEnd(KisPaintDeviceSP filled)
{
    if (progressUpdater() && progressUpdater()->interrupted()) {
        m_width = m_height = -1;
        return;
    }

//  TODO: filling using the correct bound of the selection would be better, *but*
//  the selection is limited to the exact bound of a layer, while in reality, we don't
//  want that, since we want a transparent layer to be completely filled
//     QRect rc = m_fillSelection->selectedExactRect();


    /**
     * Apply the real selection to a filled one
     */
    KisSelectionSP realSelection = selection();

    if (realSelection) {
        m_fillSelection->pixelSelection()->applySelection(
            realSelection->projection(), SELECTION_INTERSECT);
    }

    setSelection(m_fillSelection);
    bitBlt(0, 0, filled, 0, 0, m_width, m_height);
    setSelection(realSelection);

    if (progressUpdater()) progressUpdater()->setProgress(100);

    m_width = m_height = -1;
}

KisPixelSelectionSP KisFillPainter::createFloodSelection(int startX, int startY, KisPaintDeviceSP sourceDevice,
                                                         KisPaintDeviceSP existingSelection)
{
    KisPixelSelectionSP newSelection = new KisPixelSelection(new KisSelectionDefaultBounds(device()));
    return createFloodSelection(newSelection, startX, startY, sourceDevice, existingSelection);
}

KisPixelSelectionSP KisFillPainter::createFloodSelection(KisPixelSelectionSP pixelSelection, int startX, int startY,
                                                         KisPaintDeviceSP sourceDevice, KisPaintDeviceSP existingSelection)
{

    if (m_width < 0 || m_height < 0) {
        if (selection() && m_careForSelection) {
            QRect rc = selection()->selectedExactRect();
            m_width = rc.width() - (startX - rc.x());
            m_height = rc.height() - (startY - rc.y());
        }
    }
    dbgImage << "Width: " << m_width << " Height: " << m_height;
    // Otherwise the width and height should have been set
    Q_ASSERT(m_width > 0 && m_height > 0);

    QRect fillBoundsRect(0, 0, m_width, m_height);
    QPoint startPoint(startX, startY);

    if (!fillBoundsRect.contains(startPoint)) {
        return pixelSelection;
    }

    KisScanlineFill gc(sourceDevice, startPoint, fillBoundsRect);
    gc.setThreshold(m_threshold);
    if (m_useSelectionAsBoundary && !pixelSelection.isNull()) {
        gc.fillSelectionWithBoundary(pixelSelection, existingSelection);
    } else {
        gc.fillSelection(pixelSelection);
    }

    if (m_sizemod > 0) {
        KisGrowSelectionFilter biggy(m_sizemod, m_sizemod);
        biggy.process(pixelSelection, pixelSelection->selectedRect().adjusted(-m_sizemod, -m_sizemod, m_sizemod, m_sizemod));
    }
    else if (m_sizemod < 0) {
        KisShrinkSelectionFilter tiny(-m_sizemod, -m_sizemod, false);
        tiny.process(pixelSelection, pixelSelection->selectedRect());
    }
    if (m_feather > 0) {
        KisFeatherSelectionFilter feathery(m_feather);
        feathery.process(pixelSelection, pixelSelection->selectedRect().adjusted(-m_feather, -m_feather, m_feather, m_feather));
    }

    return pixelSelection;
}
