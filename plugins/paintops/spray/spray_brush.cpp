/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "spray_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>
#include <KoCompositeOp.h>
#include <KoMixColorsOp.h>

#include <brushengine/kis_paintop.h>

#include <QVariant>
#include <QHash>
#include <QTransform>
#include <QImage>

#include <kis_random_accessor_ng.h>
#include <kis_random_sub_accessor.h>

#include <kis_paint_device.h>

#include <kis_painter.h>
#include <brushengine/kis_paint_information.h>
#include <kis_fixed_paint_device.h>
#include <kis_cross_device_color_sampler.h>

#include "kis_spray_paintop_settings.h"

#include <cmath>
#include <ctime>

#include <QtGlobal>

SprayBrush::SprayBrush()
{
    m_painter = 0;
    m_transfo = 0;
}

SprayBrush::~SprayBrush()
{
    delete m_painter;
    delete m_transfo;
}

void SprayBrush::setProperties(KisSprayOptionProperties * properties,
                               KisColorProperties * colorProperties,
                               KisShapeProperties * shapeProperties,
                               KisShapeDynamicsProperties * shapeDynamicsProperties,
                               KisBrushSP brush)
{
    m_properties = properties;
    m_colorProperties = colorProperties;
    m_shapeProperties = shapeProperties;
    m_shapeDynamicsProperties = shapeDynamicsProperties;
    m_brush = brush;
    if (m_brush) {
        m_brush->notifyStrokeStarted();
    }
}

qreal SprayBrush::rotationAngle(KisRandomSourceSP randomSource)
{
    qreal rotation = 0.0;

    if (m_shapeDynamicsProperties->fixedRotation) {
        rotation = deg2rad(m_shapeDynamicsProperties->fixedAngle);
    }

    if (m_shapeDynamicsProperties->randomRotation) {

        qreal randomValue = 0.0;

        if (m_properties->gaussian) {
            randomValue = qBound<qreal>(0.0, randomSource->generateGaussian(0.0, 0.5), 1.0);
        } else {
            randomValue = randomSource->generateNormalized();
        }

        rotation =
            linearInterpolation(rotation ,
                                M_PI * 2.0 * randomValue,
                                m_shapeDynamicsProperties->randomRotationWeight);
    }

    return rotation;
}



void SprayBrush::paint(KisPaintDeviceSP dab, KisPaintDeviceSP source,
                       const KisPaintInformation& info,
                       qreal rotation, qreal scale,
                       qreal additionalScale,
                       const KoColor &color, const KoColor &bgColor)
{
    KisRandomSourceSP randomSource = info.randomSource();

    // initializing painter
    if (!m_painter) {
        m_painter = new KisPainter(dab);
        m_painter->setFillStyle(KisPainter::FillStyleForegroundColor);
        m_painter->setMaskImageSize(m_shapeProperties->width, m_shapeProperties->height);
        m_dabPixelSize = dab->colorSpace()->pixelSize();
        if (m_colorProperties->useRandomHSV) {
            m_transfo = dab->colorSpace()->createColorTransformation("hsv_adjustment", QHash<QString, QVariant>());
        }

        m_brushQImage = m_shapeProperties->image;
        if (!m_brushQImage.isNull()) {
            m_brushQImage = m_brushQImage.scaled(m_shapeProperties->width, m_shapeProperties->height);
        }
        m_imageDevice = new KisPaintDevice(dab->colorSpace());
    }


    qreal x = info.pos().x();
    qreal y = info.pos().y();
    KisRandomAccessorSP accessor = dab->createRandomAccessorNG();

    Q_ASSERT(color.colorSpace()->pixelSize() == dab->pixelSize());
    m_inkColor = color;
    KisCrossDeviceColorSampler colorSampler(source, m_inkColor);

    // apply size sensor
    m_radius = m_properties->radius() * scale * additionalScale;

    // jitter movement
    if (m_properties->jitterMovement) {
        x = x + ((2 * m_radius * randomSource->generateNormalized()) - m_radius) * m_properties->amount;
        y = y + ((2 * m_radius * randomSource->generateNormalized()) - m_radius) * m_properties->amount;
    }

    // this is wrong for every shape except pixel and anti-aliased pixel


    if (m_properties->useDensity) {
        m_particlesCount = (m_properties->coverage * (M_PI * pow2(m_radius)) / pow2(additionalScale));
    }
    else {
        m_particlesCount = m_properties->particleCount;
    }

    QHash<QString, QVariant> params;
    qreal nx, ny;
    int ix, iy;

    qreal angle;
    qreal length;
    qreal rotationZ = 0.0;
    qreal particleScale = 1.0;

    bool shouldColor = true;
    if (m_colorProperties->fillBackground) {
        m_painter->setPaintColor(bgColor);
        paintCircle(m_painter, x, y, m_radius);
    }

    QTransform m;
    m.reset();
    m.rotateRadians(-rotation + deg2rad(m_properties->brushRotation));
    m.scale(m_properties->scale, m_properties->scale);

    for (quint32 i = 0; i < m_particlesCount; i++) {
        // generate random angle
        angle = randomSource->generateNormalized() * M_PI * 2;

        // generate random length
        if (m_properties->gaussian) {
            length = randomSource->generateGaussian(0.0, 0.5);
        }
        else {
            length = randomSource->generateNormalized();
        }

        if (m_shapeDynamicsProperties->enabled) {
            // rotation
            rotationZ = rotationAngle(randomSource);

            if (m_shapeDynamicsProperties->followCursor) {

                rotationZ = linearInterpolation(rotationZ, angle, m_shapeDynamicsProperties->followCursorWeigth);
            }


            if (m_shapeDynamicsProperties->followDrawingAngle) {

                rotationZ = linearInterpolation(rotationZ, info.drawingAngle(), m_shapeDynamicsProperties->followDrawingAngleWeight);
            }

            // random size - scale
            if (m_shapeDynamicsProperties->randomSize) {
                particleScale = randomSource->generateNormalized();
            }
        }
        // generate polar coordinate
        nx = (m_radius * cos(angle)  * length);
        ny = (m_radius * sin(angle)  * length);

        // compute the height of the ellipse
        ny *= m_properties->aspect;

        // transform
        m.map(nx, ny, &nx, &ny);

        // color transformation

        if (shouldColor) {
            if (m_colorProperties->sampleInputColor) {
                colorSampler.sampleOldColor(nx + x, ny + y, m_inkColor.data());
            }

            // mix the color with background color
            if (m_colorProperties->mixBgColor) {
                KoMixColorsOp * mixOp = dab->colorSpace()->mixColorsOp();

                const quint8 *colors[2];
                colors[0] = m_inkColor.data();
                colors[1] = bgColor.data();

                qint16 colorWeights[2];
                int MAX_16BIT = 255;
                qreal blend = info.pressure();

                colorWeights[0] = static_cast<quint16>(blend * MAX_16BIT);
                colorWeights[1] = static_cast<quint16>((1.0 - blend) * MAX_16BIT);
                mixOp->mixColors(colors, colorWeights, 2, m_inkColor.data());
            }

            if (m_colorProperties->useRandomHSV && m_transfo) {
                params["h"] = (m_colorProperties->hue / 180.0) * randomSource->generateNormalized();
                params["s"] = (m_colorProperties->saturation / 100.0) * randomSource->generateNormalized();
                params["v"] = (m_colorProperties->value / 100.0) * randomSource->generateNormalized();
                m_transfo->setParameters(params);
                m_transfo->setParameter(3, 1);//sets the type to HSV. For some reason 0 is not an option.
                m_transfo->setParameter(4, false);//sets the colorize to false.
                m_transfo->transform(m_inkColor.data(), m_inkColor.data() , 1);
            }

            if (m_colorProperties->useRandomOpacity) {
                quint8 alpha = qRound(randomSource->generateNormalized() * OPACITY_OPAQUE_U8);
                m_inkColor.setOpacity(alpha);
                m_painter->setOpacity(alpha);
            }

            if (!m_colorProperties->colorPerParticle) {
                shouldColor = false;
            }

            m_painter->setPaintColor(m_inkColor);
        }

        qreal jitteredWidth = qMax(1.0 * additionalScale, m_shapeProperties->width * particleScale * additionalScale);
        qreal jitteredHeight = qMax(1.0 * additionalScale, m_shapeProperties->height * particleScale * additionalScale);

        if (m_shapeProperties->enabled){
        switch (m_shapeProperties->shape){
            // ellipse
            case 0:
            {
                if (m_shapeProperties->width == m_shapeProperties->height){
                    paintCircle(m_painter, nx + x, ny + y, jitteredWidth * 0.5);
                }
                else {
                    paintEllipse(m_painter, nx + x, ny + y, jitteredWidth * 0.5 , jitteredHeight * 0.5, rotationZ);
                }
                break;
            }
            // rectangle
            case 1:
            {
                paintRectangle(m_painter, nx + x, ny + y, qRound(jitteredWidth) , qRound(jitteredHeight), rotationZ);
                break;
            }
            // wu-particle
            case 2: {
                paintParticle(accessor, m_inkColor, nx + x, ny + y);
                break;
            }
            // pixel
            case 3: {
                ix = qRound(nx + x);
                iy = qRound(ny + y);
                accessor->moveTo(ix, iy);
                memcpy(accessor->rawData(), m_inkColor.data(), m_dabPixelSize);
                break;
            }
            case 4: {
                if (!m_brushQImage.isNull()) {

                    QTransform m;
                    m.rotate(rad2deg(rotationZ));
                    m.scale(additionalScale, additionalScale);

                    if (m_shapeDynamicsProperties->randomSize) {
                        m.scale(particleScale, particleScale);
                    }
                    m_transformed = m_brushQImage.transformed(m, Qt::SmoothTransformation);
                    m_imageDevice->convertFromQImage(m_transformed, 0);
                    KisRandomAccessorSP ac = m_imageDevice->createRandomAccessorNG();
                    QRect rc = m_transformed.rect();

                    if (m_colorProperties->useRandomHSV && m_transfo) {

                        for (int y = rc.y(); y < rc.y() + rc.height(); y++) {
                            for (int x = rc.x(); x < rc.x() + rc.width(); x++) {
                                ac->moveTo(x, y);
                                m_transfo->transform(ac->rawData(), ac->rawData() , 1);
                            }
                        }
                    }

                    ix = qRound(nx + x - rc.width() * 0.5);
                    iy = qRound(ny + y - rc.height() * 0.5);
                    m_painter->bitBlt(QPoint(ix, iy), m_imageDevice, rc);
                    m_imageDevice->clear();
                    break;
                }
            }
            }
            // Auto-brush
        }
        else {
            KisDabShape shape(particleScale * additionalScale, 1.0, -rotationZ);
            QPointF hotSpot = m_brush->hotSpot(shape, info);
            QPointF pos(nx + x, ny + y);
            QPointF pt = pos - hotSpot;

            qint32 ix;
            qreal xFraction;
            qint32 iy;
            qreal yFraction;

            KisPaintOp::splitCoordinate(pt.x(), &ix, &xFraction);
            KisPaintOp::splitCoordinate(pt.y(), &iy, &yFraction);

            m_brush->prepareForSeqNo(info, m_dabSeqNo);

            //KisFixedPaintDeviceSP dab;
            if (m_brush->brushApplication() == IMAGESTAMP) {
                m_fixedDab = m_brush->paintDevice(m_fixedDab->colorSpace(),
                          shape, info, xFraction, yFraction);

                if (m_colorProperties->useRandomHSV && m_transfo) {
                    quint8 * dabPointer = m_fixedDab->data();
                    int pixelCount = m_fixedDab->bounds().width() * m_fixedDab->bounds().height();
                    m_transfo->transform(dabPointer, dabPointer, pixelCount);
                }

            }
            else {
                m_brush->mask(m_fixedDab, m_inkColor, shape,
                              info, xFraction, yFraction);
            }
            m_painter->bltFixed(QPoint(ix, iy), m_fixedDab, m_fixedDab->bounds());
        }
        if (m_colorProperties->colorPerParticle){
            m_inkColor=color;//reset color//
        }
    }
    // recover from jittering of color,
    // m_inkColor.opacity is recovered with every paint
}



void SprayBrush::paintParticle(KisRandomAccessorSP &writeAccessor, const KoColor &color, qreal rx, qreal ry)
{
    // opacity top left, right, bottom left, right
    KoColor pcolor(color);
    //int opacity = pcolor.opacityU8();

    int ipx = int (rx);
    int ipy = int (ry);
    qreal fx = rx - ipx;
    qreal fy = ry - ipy;

    qreal btl = (1 - fx) * (1 - fy);
    qreal btr = (fx)  * (1 - fy);
    qreal bbl = (1 - fx) * (fy);
    qreal bbr = (fx)  * (fy);

    // this version overwrite pixels, e.g. when it sprays two particle next
    // to each other, the pixel with lower opacity can override other pixel.
    // Maybe some kind of compositing using here would be cool

    pcolor.setOpacity(btl);
    writeAccessor->moveTo(ipx  , ipy);
    memcpy(writeAccessor->rawData(), pcolor.data(), m_dabPixelSize);

    pcolor.setOpacity(btr);
    writeAccessor->moveTo(ipx + 1, ipy);
    memcpy(writeAccessor->rawData(), pcolor.data(), m_dabPixelSize);

    pcolor.setOpacity(bbl);
    writeAccessor->moveTo(ipx, ipy + 1);
    memcpy(writeAccessor->rawData(), pcolor.data(), m_dabPixelSize);

    pcolor.setOpacity(bbr);
    writeAccessor->moveTo(ipx + 1, ipy + 1);
    memcpy(writeAccessor->rawData(), pcolor.data(), m_dabPixelSize);
}

void SprayBrush::paintCircle(KisPainter* painter, qreal x, qreal y, qreal radius)
{
    QPainterPath path;
    path.addEllipse(QPointF(x,y),radius,radius);
    painter->fillPainterPath(path);
}


void SprayBrush::paintEllipse(KisPainter* painter, qreal x, qreal y, qreal a, qreal b, qreal angle)
{
    QPainterPath path;
    path.addEllipse(QPointF(), a, b);
    QTransform t;
    t.translate(x, y);
    t.rotateRadians(angle);
    path = t.map(path);
    painter->fillPainterPath(path);
}

void SprayBrush::paintRectangle(KisPainter* painter, qreal x, qreal y, qreal width, qreal height, qreal angle)
{
    QPainterPath path;
    path.addRect(QRectF(-0.5 * width, -0.5 * height, width, height));
    QTransform t;
    t.translate(x, y);
    t.rotateRadians(angle);
    path = t.map(path);
    painter->fillPainterPath(path);
}


void SprayBrush::paintOutline(KisPaintDeviceSP dev , const KoColor &outlineColor, qreal posX, qreal posY, qreal radius)
{
    QList<QPointF> antiPixels;
    KisRandomAccessorSP accessor = dev->createRandomAccessorNG();

    for (int y = -radius + posY; y <= radius + posY; y++) {
        for (int x = -radius + posX; x <= radius + posX; x++) {
            accessor->moveTo(x, y);
            qreal alpha = dev->colorSpace()->opacityU8(accessor->rawData());

            if (alpha != 0) {
                // top left
                accessor->moveTo(x - 1, y - 1);
                if (dev->colorSpace()->opacityU8(accessor->rawData()) == 0) {
                    antiPixels.append(QPointF(x - 1, y - 1));
                    //continue;
                }

                // top
                accessor->moveTo(x, y - 1);
                if (dev->colorSpace()->opacityU8(accessor->rawData()) == 0) {
                    antiPixels.append(QPointF(x, y - 1));
                    //continue;
                }

                // top right
                accessor->moveTo(x + 1, y - 1);
                if (dev->colorSpace()->opacityU8(accessor->rawData()) == 0) {
                    antiPixels.append(QPointF(x + 1, y - 1));
                    //continue;
                }

                //left
                accessor->moveTo(x - 1, y);
                if (dev->colorSpace()->opacityU8(accessor->rawData()) == 0) {
                    antiPixels.append(QPointF(x - 1, y));
                    //continue;
                }

                //right
                accessor->moveTo(x + 1, y);
                if (dev->colorSpace()->opacityU8(accessor->rawData()) == 0) {
                    antiPixels.append(QPointF(x + 1, y));
                    //continue;
                }

                // bottom left
                accessor->moveTo(x - 1, y + 1);
                if (dev->colorSpace()->opacityU8(accessor->rawData()) == 0) {
                    antiPixels.append(QPointF(x - 1, y + 1));
                    //continue;
                }

                // bottom
                accessor->moveTo(x, y + 1);
                if (dev->colorSpace()->opacityU8(accessor->rawData()) == 0) {
                    antiPixels.append(QPointF(x, y + 1));
                    //continue;
                }

                // bottom right
                accessor->moveTo(x + 1, y + 1);
                if (dev->colorSpace()->opacityU8(accessor->rawData()) == 0) {
                    antiPixels.append(QPointF(x + 1, y + 1));
                    //continue;
                }
            }

        }
    }

    // anti-alias it
    int size = antiPixels.size();
    for (int i = 0; i < size; i++) {
        accessor->moveTo(antiPixels[i].x(), antiPixels[i].y());
        memcpy(accessor->rawData(), outlineColor.data(), dev->colorSpace()->pixelSize());
    }
}

void SprayBrush::setFixedDab(KisFixedPaintDeviceSP dab)
{
    m_fixedDab = dab;
}
