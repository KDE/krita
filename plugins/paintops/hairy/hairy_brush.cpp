/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "hairy_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>
#include <KoCompositeOpRegistry.h>

#include <QVariant>
#include <QHash>
#include <QVector>

#include <kis_types.h>
#include <kis_random_accessor_ng.h>
#include <kis_cross_device_color_picker.h>
#include <kis_fixed_paint_device.h>


#include <cmath>
#include <ctime>


HairyBrush::HairyBrush()
{
    m_counter = 0;
    m_lastAngle = 0.0;
    m_oldPressure = 1.0f;

    m_saturationId = -1;
    m_transfo = 0;
}

HairyBrush::~HairyBrush()
{
    delete m_transfo;
    qDeleteAll(m_bristles.begin(), m_bristles.end());
    m_bristles.clear();
}


void HairyBrush::initAndCache()
{
    m_compositeOp = m_dab->colorSpace()->compositeOp(COMPOSITE_OVER);
    m_pixelSize = m_dab->colorSpace()->pixelSize();

    if (m_properties->useSaturation) {
        m_transfo = m_dab->colorSpace()->createColorTransformation("hsv_adjustment", m_params);
        if (m_transfo) {
            m_saturationId = m_transfo->parameterId("s");
        }
    }
}

void HairyBrush::fromDabWithDensity(KisFixedPaintDeviceSP dab, qreal density)
{
    int width = dab->bounds().width();
    int height = dab->bounds().height();

    int centerX = width * 0.5;
    int centerY = height * 0.5;

    // make mask
    Bristle * bristle = 0;
    qreal alpha;

    quint8 * dabPointer = dab->data();
    quint8 pixelSize = dab->pixelSize();
    const KoColorSpace * cs = dab->colorSpace();
    KoColor bristleColor(cs);

    KisRandomSource randomSource(0);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            alpha =  cs->opacityF(dabPointer);
            if (alpha != 0.0) {
                if (density == 1.0 || randomSource.generateNormalized() <= density) {
                    memcpy(bristleColor.data(), dabPointer, pixelSize);

                    bristle = new Bristle(x - centerX, y - centerY, alpha); // using value from image as length of bristle
                    bristle->setColor(bristleColor);

                    m_bristles.append(bristle);
                }
            }
            dabPointer += pixelSize;
        }
    }
}


void HairyBrush::paintLine(KisPaintDeviceSP dab, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2, qreal scale, qreal rotation)
{
    m_counter++;

    qreal x1 = pi1.pos().x();
    qreal y1 = pi1.pos().y();

    qreal x2 = pi2.pos().x();
    qreal y2 = pi2.pos().y();

    qreal dx = x2 - x1;
    qreal dy = y2 - y1;

    // TODO:this angle is different from the drawing angle in sensor (info.angle()). The bug is caused probably due to
    // not computing the drag vector properly in paintBezierLine when smoothing is used
    //qreal angle = atan2(dy, dx);
    qreal angle = rotation;

    qreal mousePressure = 1.0;
    if (m_properties->useMousePressure) { // want pressure from mouse movement
        qreal distance = sqrt(dx * dx + dy * dy);
        mousePressure = (1.0 - computeMousePressure(distance));
        scale *= mousePressure;
    }
    // this pressure controls shear and ink depletion
    qreal pressure = mousePressure * (pi2.pressure() * 2);

    Bristle *bristle = 0;
    KoColor bristleColor(dab->colorSpace());

    m_dabAccessor = dab->createRandomAccessorNG((int)x1, (int)y1);

    m_dab = dab;

    // initialization block
    if (firstStroke()) {
        initAndCache();
    }

    /*If this is first time the brush touches the canvas and
    we are using soak ink while ink depletion is enabled...*/
    if (m_properties->inkDepletionEnabled &&
            firstStroke() && m_properties->useSoakInk) {
        if (layer) {
            colorifyBristles(layer, pi1.pos());
        }
        else {
            dbgKrita << "Can't soak the ink from the layer";
        }
    }

    KisRandomSourceSP randomSource = pi2.randomSource();

    qreal fx1, fy1, fx2, fy2;
    qreal randomX, randomY;
    qreal shear;

    float inkDeplation = 0.0;
    int inkDepletionSize = m_properties->inkDepletionCurve.size();
    int bristleCount = m_bristles.size();
    int bristlePathSize;
    qreal threshold = 1.0 - pi2.pressure();
    for (int i = 0; i < bristleCount; i++) {

        if (!m_bristles.at(i)->enabled()) continue;
        bristle = m_bristles[i];

        randomX = (randomSource->generateNormalized() * 2 - 1.0) * m_properties->randomFactor;
        randomY = (randomSource->generateNormalized() * 2 - 1.0) * m_properties->randomFactor;

        shear = pressure * m_properties->shearFactor;

        m_transform.reset();
        m_transform.rotateRadians(-angle);
        m_transform.scale(scale, scale);
        m_transform.translate(randomX, randomY);
        m_transform.shear(shear, shear);

        if (firstStroke() || (!m_properties->connectedPath)) {
            // transform start dab
            m_transform.map(bristle->x(), bristle->y(), &fx1, &fy1);
            // transform end dab
            m_transform.map(bristle->x(), bristle->y(), &fx2, &fy2);
        }
        else {
            // continue the path of the bristle from the previous position
            fx1 = bristle->prevX();
            fy1 = bristle->prevY();
            m_transform.map(bristle->x(), bristle->y(), &fx2, &fy2);
        }
        // remember the end point
        bristle->setPrevX(fx2);
        bristle->setPrevY(fy2);

        // all coords relative to device position
        fx1 += x1;
        fy1 += y1;

        fx2 += x2;
        fy2 += y2;

        if (m_properties->threshold && (bristle->length() < threshold)) continue;
        // paint between first and last dab
        const QVector<QPointF> bristlePath = m_trajectory.getLinearTrajectory(QPointF(fx1, fy1), QPointF(fx2, fy2), 1.0);
        bristlePathSize = m_trajectory.size();

        memcpy(bristleColor.data(), bristle->color().data() , m_pixelSize);
        for (int i = 0; i < bristlePathSize ; i++) {

            if (m_properties->inkDepletionEnabled) {
                inkDeplation = fetchInkDepletion(bristle, inkDepletionSize);

                if (m_properties->useSaturation && m_transfo != 0) {
                    saturationDepletion(bristle, bristleColor, pressure, inkDeplation);
                }

                if (m_properties->useOpacity) {
                    opacityDepletion(bristle, bristleColor, pressure, inkDeplation);
                }

            }
            else {
                if (bristleColor.opacityU8() != 0) {
                    bristleColor.setOpacity(bristle->length());
                }
            }

            addBristleInk(bristle, bristlePath.at(i), bristleColor);
            bristle->setInkAmount(1.0 - inkDeplation);
            bristle->upIncrement();
        }

    }
    m_dab = 0;
    m_dabAccessor = 0;
}


inline qreal HairyBrush::fetchInkDepletion(Bristle* bristle, int inkDepletionSize)
{
    if (bristle->counter() >= inkDepletionSize - 1) {
        return m_properties->inkDepletionCurve[inkDepletionSize - 1];
    } else {
        return m_properties->inkDepletionCurve[bristle->counter()];
    }
}


void HairyBrush::saturationDepletion(Bristle * bristle, KoColor &bristleColor, qreal pressure, qreal inkDeplation)
{
    qreal saturation;
    if (m_properties->useWeights) {
        // new weighted way (experiment)
        saturation = (
                         (pressure * m_properties->pressureWeight) +
                         (bristle->length() * m_properties->bristleLengthWeight) +
                         (bristle->inkAmount() * m_properties->bristleInkAmountWeight) +
                         ((1.0 - inkDeplation) * m_properties->inkDepletionWeight)) - 1.0;
    }
    else {
        // old way of computing saturation
        saturation = (
                         pressure *
                         bristle->length() *
                         bristle->inkAmount() *
                         (1.0 - inkDeplation)) - 1.0;

    }
    m_transfo->setParameter(m_transfo->parameterId("h"), 0.0);
    m_transfo->setParameter(m_transfo->parameterId("v"), 0.0);
    m_transfo->setParameter(m_saturationId, saturation);
    m_transfo->setParameter(3, 1);//sets the type to
    m_transfo->setParameter(4, false);//sets the colorize to none.
    m_transfo->transform(bristleColor.data(), bristleColor.data() , 1);
}

void HairyBrush::opacityDepletion(Bristle* bristle, KoColor& bristleColor, qreal pressure, qreal inkDeplation)
{
    qreal opacity = OPACITY_OPAQUE_F;
    if (m_properties->useWeights) {
        opacity = pressure * m_properties->pressureWeight +
                  bristle->length() * m_properties->bristleLengthWeight +
                  bristle->inkAmount() * m_properties->bristleInkAmountWeight +
                  (1.0 - inkDeplation) * m_properties->inkDepletionWeight;
    }
    else {
        opacity =
            bristle->length() *
            bristle->inkAmount();
    }

    opacity = qBound(0.0, opacity, 1.0);
    bristleColor.setOpacity(opacity);
}

inline void HairyBrush::addBristleInk(Bristle *bristle,const QPointF &pos, const KoColor &color)
{
    Q_UNUSED(bristle);
    if (m_properties->antialias) {
        if (m_properties->useCompositing) {
            paintParticle(pos, color);
        } else {
            paintParticle(pos, color, 1.0);
        }
    }
    else {
        int ix = qRound(pos.x());
        int iy = qRound(pos.y());
        if (m_properties->useCompositing) {
            plotPixel(ix, iy, color);
        }
        else {
            darkenPixel(ix, iy, color);
        }
    }
}

void HairyBrush::paintParticle(QPointF pos, const KoColor& color, qreal weight)
{
    // opacity top left, right, bottom left, right
    quint8 opacity = color.opacityU8();
    opacity *= weight;

    int ipx = int (pos.x());
    int ipy = int (pos.y());
    qreal fx = qAbs(pos.x() - ipx);
    qreal fy = qAbs(pos.y() - ipy);

    quint8 btl = qRound((1.0 - fx) * (1.0 - fy) * opacity);
    quint8 btr = qRound((fx)  * (1.0 - fy) * opacity);
    quint8 bbl = qRound((1.0 - fx) * (fy)  * opacity);
    quint8 bbr = qRound((fx)  * (fy)  * opacity);

    const KoColorSpace * cs = m_dab->colorSpace();

    m_dabAccessor->moveTo(ipx  , ipy);
    btl = quint8(qBound<quint16>(OPACITY_TRANSPARENT_U8, btl + cs->opacityU8(m_dabAccessor->rawData()), OPACITY_OPAQUE_U8));
    memcpy(m_dabAccessor->rawData(), color.data(), cs->pixelSize());
    cs->setOpacity(m_dabAccessor->rawData(), btl, 1);

    m_dabAccessor->moveTo(ipx + 1, ipy);
    btr =  quint8(qBound<quint16>(OPACITY_TRANSPARENT_U8, btr + cs->opacityU8(m_dabAccessor->rawData()), OPACITY_OPAQUE_U8));
    memcpy(m_dabAccessor->rawData(), color.data(), cs->pixelSize());
    cs->setOpacity(m_dabAccessor->rawData(), btr, 1);

    m_dabAccessor->moveTo(ipx, ipy + 1);
    bbl = quint8(qBound<quint16>(OPACITY_TRANSPARENT_U8, bbl + cs->opacityU8(m_dabAccessor->rawData()), OPACITY_OPAQUE_U8));
    memcpy(m_dabAccessor->rawData(), color.data(), cs->pixelSize());
    cs->setOpacity(m_dabAccessor->rawData(), bbl, 1);

    m_dabAccessor->moveTo(ipx + 1, ipy + 1);
    bbr = quint8(qBound<quint16>(OPACITY_TRANSPARENT_U8, bbr + cs->opacityU8(m_dabAccessor->rawData()), OPACITY_OPAQUE_U8));
    memcpy(m_dabAccessor->rawData(), color.data(), cs->pixelSize());
    cs->setOpacity(m_dabAccessor->rawData(), bbr, 1);
}

void HairyBrush::paintParticle(QPointF pos, const KoColor& color)
{
    // opacity top left, right, bottom left, right
    memcpy(m_color.data(), color.data(), m_pixelSize);
    quint8 opacity = color.opacityU8();

    int ipx = int (pos.x());
    int ipy = int (pos.y());
    qreal fx = qAbs(pos.x() - ipx);
    qreal fy = qAbs(pos.y() - ipy);

    quint8 btl = qRound((1.0 - fx) * (1.0 - fy) * opacity);
    quint8 btr = qRound((fx)  * (1.0 - fy) * opacity);
    quint8 bbl = qRound((1.0 - fx) * (fy)  * opacity);
    quint8 bbr = qRound((fx)  * (fy)  * opacity);

    m_color.setOpacity(btl);
    plotPixel(ipx  , ipy, m_color);

    m_color.setOpacity(btr);
    plotPixel(ipx + 1  , ipy, m_color);

    m_color.setOpacity(bbl);
    plotPixel(ipx  , ipy + 1, m_color);

    m_color.setOpacity(bbr);
    plotPixel(ipx + 1 , ipy + 1, m_color);
}


inline void HairyBrush::plotPixel(int wx, int wy, const KoColor &color)
{
    m_dabAccessor->moveTo(wx, wy);
    m_compositeOp->composite(m_dabAccessor->rawData(), m_pixelSize, color.data() , m_pixelSize, 0, 0, 1, 1, OPACITY_OPAQUE_U8);
}

inline void HairyBrush::darkenPixel(int wx, int wy, const KoColor &color)
{
    m_dabAccessor->moveTo(wx, wy);
    if (m_dab->colorSpace()->opacityU8(m_dabAccessor->rawData()) < color.opacityU8()) {
        memcpy(m_dabAccessor->rawData(), color.data(), m_pixelSize);
    }
}

double HairyBrush::computeMousePressure(double distance)
{
    static const double scale = 20.0;
    static const double minPressure = 0.02;

    double oldPressure = m_oldPressure;

    double factor = 1.0 - distance / scale;
    if (factor < 0.0) factor = 0.0;

    double result = ((4.0 * oldPressure) + minPressure + factor) / 5.0;

    m_oldPressure = result;
    return result;
}


void HairyBrush::colorifyBristles(KisPaintDeviceSP source, QPointF point)
{
    KoColor bristleColor(m_dab->colorSpace());
    KisCrossDeviceColorPickerInt colorPicker(source, bristleColor);

    Bristle *b = 0;
    int size = m_bristles.size();
    for (int i = 0; i < size; i++) {
        b = m_bristles[i];
        int x = qRound(b->x() + point.x());
        int y = qRound(b->y() + point.y());

        colorPicker.pickOldColor(x, y, bristleColor.data());
        b->setColor(bristleColor);
    }

}


