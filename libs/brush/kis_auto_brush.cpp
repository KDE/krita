/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_auto_brush.h"

#include <kis_debug.h>
#include <math.h>

#include <QRect>
#include <QDomElement>
#include <QtConcurrentMap>
#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_datamanager.h>
#include <kis_fixed_paint_device.h>
#include <kis_paint_device.h>
#include <brushengine/kis_paint_information.h>
#include <kis_mask_generator.h>
#include <kis_boundary.h>
#include <brushengine/kis_paintop_lod_limitations.h>
#include <kis_brush_mask_applicator_base.h>


#if defined(_WIN32) || defined(_WIN64)
#include <stdlib.h>
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

struct KisAutoBrush::Private {
    Private()
        : randomness(0), density(1.0), idealThreadCountCached(1) {}

    Private(const Private &rhs)
        : shape(rhs.shape->clone()),
          randomness(rhs.randomness),
          density(rhs.density),
          idealThreadCountCached(rhs.idealThreadCountCached)
    {
    }

    QScopedPointer<KisMaskGenerator> shape;
    qreal randomness;
    qreal density;
    int idealThreadCountCached;
};

KisAutoBrush::KisAutoBrush(KisMaskGenerator* as, qreal angle, qreal randomness, qreal density)
    : KisBrush(),
      d(new Private)
{
    d->shape.reset(as);
    d->randomness = randomness;
    d->density = density;
    d->idealThreadCountCached = QThread::idealThreadCount();
    setBrushType(MASK);
    setWidth(qMax(qreal(1.0), d->shape->width()));
    setHeight(qMax(qreal(1.0), d->shape->height()));

    QImage image = createBrushPreview();
    setBrushTipImage(image);

    // Set angle here so brush tip image is generated unrotated
    setAngle(angle);

    image = createBrushPreview();
    setImage(image);
}

KisAutoBrush::~KisAutoBrush()
{
}

qreal KisAutoBrush::userEffectiveSize() const
{
    return d->shape->diameter();
}

void KisAutoBrush::setUserEffectiveSize(qreal value)
{
    d->shape->setDiameter(value);
}

KisAutoBrush::KisAutoBrush(const KisAutoBrush& rhs)
    : KisBrush(rhs),
      d(new Private(*rhs.d))
{
}

KisBrush* KisAutoBrush::clone() const
{
    return new KisAutoBrush(*this);
}

/* It's difficult to predict the mask height when exaclty when there are
 * more than 2 spikes, so we return an upperbound instead. */
static KisDabShape lieAboutDabShape(KisDabShape const& shape)
{
    return KisDabShape(shape.scale(), 1.0, shape.rotation());
}

qint32 KisAutoBrush::maskHeight(KisDabShape const& shape,
    qreal subPixelX, qreal subPixelY, const KisPaintInformation& info) const
{
    return KisBrush::maskHeight(
        lieAboutDabShape(shape), subPixelX, subPixelY, info);
}

qint32 KisAutoBrush::maskWidth(KisDabShape const& shape,
    qreal subPixelX, qreal subPixelY, const KisPaintInformation& info) const
{
    return KisBrush::maskWidth(
        lieAboutDabShape(shape), subPixelX, subPixelY, info);
}

QSizeF KisAutoBrush::characteristicSize(KisDabShape const& shape) const
{
    return KisBrush::characteristicSize(lieAboutDabShape(shape));
}


inline void fillPixelOptimized_4bytes(quint8 *color, quint8 *buf, int size)
{
    /**
     * This version of filling uses low granularity of data transfers
     * (32-bit chunks) and internal processor's parallelism. It reaches
     * 25% better performance in KisStrokeBenchmark in comparison to
     * per-pixel memcpy version (tested on Sandy Bridge).
     */

    int block1 = size / 8;
    int block2 = size % 8;

    quint32 *src = reinterpret_cast<quint32*>(color);
    quint32 *dst = reinterpret_cast<quint32*>(buf);

    // check whether all buffers are 4 bytes aligned
    // (uncomment if experience some problems)
    // Q_ASSERT(((qint64)src & 3) == 0);
    // Q_ASSERT(((qint64)dst & 3) == 0);

    for (int i = 0; i < block1; i++) {
        *dst = *src;
        *(dst + 1) = *src;
        *(dst + 2) = *src;
        *(dst + 3) = *src;
        *(dst + 4) = *src;
        *(dst + 5) = *src;
        *(dst + 6) = *src;
        *(dst + 7) = *src;

        dst += 8;
    }

    for (int i = 0; i < block2; i++) {
        *dst = *src;
        dst++;
    }
}

inline void fillPixelOptimized_general(quint8 *color, quint8 *buf, int size, int pixelSize)
{
    /**
     * This version uses internal processor's parallelism and gives
     * 20% better performance in KisStrokeBenchmark in comparison to
     * per-pixel memcpy version (tested on Sandy Bridge (+20%) and
     * on Merom (+10%)).
     */

    int block1 = size / 8;
    int block2 = size % 8;

    for (int i = 0; i < block1; i++) {
        quint8 *d1 = buf;
        quint8 *d2 = buf + pixelSize;
        quint8 *d3 = buf + 2 * pixelSize;
        quint8 *d4 = buf + 3 * pixelSize;
        quint8 *d5 = buf + 4 * pixelSize;
        quint8 *d6 = buf + 5 * pixelSize;
        quint8 *d7 = buf + 6 * pixelSize;
        quint8 *d8 = buf + 7 * pixelSize;

        for (int j = 0; j < pixelSize; j++) {
            *(d1 + j) = color[j];
            *(d2 + j) = color[j];
            *(d3 + j) = color[j];
            *(d4 + j) = color[j];
            *(d5 + j) = color[j];
            *(d6 + j) = color[j];
            *(d7 + j) = color[j];
            *(d8 + j) = color[j];
        }

        buf += 8 * pixelSize;
    }

    for (int i = 0; i < block2; i++) {
        memcpy(buf, color, pixelSize);
        buf += pixelSize;
    }
}

void KisAutoBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
        KisBrush::ColoringInformation* coloringInformation,
        KisDabShape const& shape,
        const KisPaintInformation& info,
        double subPixelX , double subPixelY, qreal softnessFactor) const
{
    Q_UNUSED(info);

    // Generate the paint device from the mask
    const KoColorSpace* cs = dst->colorSpace();
    quint32 pixelSize = cs->pixelSize();

    // mask dimension methods already includes KisBrush::angle()
    int dstWidth = maskWidth(shape, subPixelX, subPixelY, info);
    int dstHeight = maskHeight(shape, subPixelX, subPixelY, info);
    QPointF hotSpot = this->hotSpot(shape, info);

    // mask size and hotSpot function take the KisBrush rotation into account
    qreal angle = shape.rotation() + KisBrush::angle();

    // if there's coloring information, we merely change the alpha: in that case,
    // the dab should be big enough!
    if (coloringInformation) {
        // new bounds. we don't care if there is some extra memory occcupied.
        dst->setRect(QRect(0, 0, dstWidth, dstHeight));
        dst->lazyGrowBufferWithoutInitialization();
    }
    else {
        KIS_SAFE_ASSERT_RECOVER_RETURN(dst->bounds().width() >= dstWidth &&
                                       dst->bounds().height() >= dstHeight);
    }

    quint8* dabPointer = dst->data();

    quint8* color = 0;
    if (coloringInformation) {
        if (dynamic_cast<PlainColoringInformation*>(coloringInformation)) {
            color = const_cast<quint8*>(coloringInformation->color());
        }
    }

    double centerX = hotSpot.x() - 0.5 + subPixelX;
    double centerY = hotSpot.y() - 0.5 + subPixelY;

    d->shape->setSoftness(softnessFactor); // softness must be set first
    d->shape->setScale(shape.scaleX(), shape.scaleY());

    if (coloringInformation) {
        if (color && pixelSize == 4) {
            fillPixelOptimized_4bytes(color, dabPointer, dstWidth * dstHeight);
        }
        else if (color) {
            fillPixelOptimized_general(color, dabPointer, dstWidth * dstHeight, pixelSize);
        }
        else {
            for (int y = 0; y < dstHeight; y++) {
                for (int x = 0; x < dstWidth; x++) {
                    memcpy(dabPointer, coloringInformation->color(), pixelSize);
                    coloringInformation->nextColumn();
                    dabPointer += pixelSize;
                }
                coloringInformation->nextRow();
            }
        }
    }

    MaskProcessingData data(dst, cs, d->randomness, d->density,
                            centerX, centerY,
                            angle);

    KisBrushMaskApplicatorBase *applicator = d->shape->applicator();
    applicator->initializeData(&data);

    int jobs = d->idealThreadCountCached;
    if (threadingAllowed() && dstHeight > 100 && jobs >= 4) {
        int splitter = dstHeight / jobs;
        QVector<QRect> rects;
        for (int i = 0; i < jobs - 1; i++) {
            rects << QRect(0, i * splitter, dstWidth, splitter);
        }
        rects << QRect(0, (jobs - 1)*splitter, dstWidth, dstHeight - (jobs - 1)*splitter);
        OperatorWrapper wrapper(applicator);
        QtConcurrent::blockingMap(rects, wrapper);
    }
    else {
        QRect rect(0, 0, dstWidth, dstHeight);
        applicator->process(rect);
    }
}


void KisAutoBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    QDomElement shapeElt = doc.createElement("MaskGenerator");
    d->shape->toXML(doc, shapeElt);
    e.appendChild(shapeElt);
    e.setAttribute("type", "auto_brush");
    e.setAttribute("spacing", QString::number(spacing()));
    e.setAttribute("useAutoSpacing", QString::number(autoSpacingActive()));
    e.setAttribute("autoSpacingCoeff", QString::number(autoSpacingCoeff()));
    e.setAttribute("angle", QString::number(KisBrush::angle()));
    e.setAttribute("randomness", QString::number(d->randomness));
    e.setAttribute("density", QString::number(d->density));
    KisBrush::toXML(doc, e);
}

QImage KisAutoBrush::createBrushPreview()
{
    int width = maskWidth(KisDabShape(), 0.0, 0.0, KisPaintInformation());
    int height = maskHeight(KisDabShape(), 0.0, 0.0, KisPaintInformation());

    KisPaintInformation info(QPointF(width * 0.5, height * 0.5), 0.5, 0, 0, angle(), 0, 0, 0, 0);

    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    fdev->setRect(QRect(0, 0, width, height));
    fdev->initialize();

    mask(fdev, KoColor(Qt::black, fdev->colorSpace()), KisDabShape(), info);
    return fdev->convertToQImage(0);
}


const KisMaskGenerator* KisAutoBrush::maskGenerator() const
{
    return d->shape.data();
}

qreal KisAutoBrush::density() const
{
    return d->density;
}

qreal KisAutoBrush::randomness() const
{
    return d->randomness;
}

QPainterPath KisAutoBrush::outline() const
{
    bool simpleOutline = (d->density < 1.0);
    if (simpleOutline) {
        QPainterPath path;
        QRectF brushBoundingbox(0, 0, width(), height());
        if (maskGenerator()->type() == KisMaskGenerator::CIRCLE) {
            path.addEllipse(brushBoundingbox);
        }
        else { // if (maskGenerator()->type() == KisMaskGenerator::RECTANGLE)
            path.addRect(brushBoundingbox);
        }

        return path;
    }

    return KisBrush::boundary()->path();
}

void KisAutoBrush::lodLimitations(KisPaintopLodLimitations *l) const
{
    KisBrush::lodLimitations(l);

    if (!qFuzzyCompare(density(), 1.0)) {
        l->limitations << KoID("auto-brush-density", i18nc("PaintOp instant preview limitation", "Brush Density recommended value 100.0"));
    }

    if (!qFuzzyCompare(randomness(), 0.0)) {
        l->limitations << KoID("auto-brush-randomness", i18nc("PaintOp instant preview limitation", "Brush Randomness recommended value 0.0"));
    }
}
