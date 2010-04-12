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

#include <KoCompositeOps.h>

#include "hairy_brush.h"
#include "brush_shape.h"
#include "trajectory.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>
#include <QList>

#include "kis_random_accessor.h"

#include <cmath>
#include <ctime>

const float radToDeg = 57.29578f;

const QString HUE = "h";
const QString SATURATION = "s";
const QString VALUE = "v";

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif


HairyBrush::HairyBrush()
{
    srand48(time(0));
    m_counter = 0;
    m_lastAngle = 0.0;
    m_oldPressure = 0.0f;

    m_params[HUE] = 0.0;
    m_params[SATURATION] = 0.0;
    m_params[VALUE] = 0.0;
}

HairyBrush::~HairyBrush()
{

}

void HairyBrush::setBrushShape(BrushShape brushShape)
{
    m_initialShape = brushShape;
    m_bristles = brushShape.getBristles();
}


void HairyBrush::setInkColor(const KoColor &color)
{
    for (int i = 0; i < m_bristles.size(); i++) {
        m_bristles[i]->setColor(color);
    }
    m_color = color;
}


void HairyBrush::paintLine(KisPaintDeviceSP dab, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2, qreal scale)
{
    m_counter++;
    
    qreal x1 = pi1.pos().x();
    qreal y1 = pi1.pos().y();

    qreal x2 = pi2.pos().x();
    qreal y2 = pi2.pos().y();

    qreal dx = x2 - x1;
    qreal dy = y2 - y1;
   
    qreal angle = atan2(dy, dx);

    qreal mousePressure = 1.0;
    if (m_properties->useMousePressure) { // want pressure from mouse movement
        qreal distance = sqrt(dx * dx + dy * dy);
        mousePressure = (1.0 - computeMousePressure(distance));
        scale *= mousePressure;
    }
    // this pressure controls shear and ink depletion
    qreal pressure = mousePressure * (pi2.pressure() * 2);

    Bristle *bristle = 0;
    KoColor bristleColor;

    KisRandomAccessor accessor = dab->createRandomAccessor((int)x1, (int)y1);
    m_pixelSize = dab->colorSpace()->pixelSize();
    m_dabAccessor = &accessor;
    m_dab = dab;

    if (firstStroke()){
        m_compositeOp = m_dab->colorSpace()->compositeOp(COMPOSITE_OVER);
    }
    
    m_params[SATURATION] = 0.0;
    KoColorTransformation * transfo = 0;
    int saturationId = 0;
    if (m_properties->useSaturation){
        transfo = m_dab->colorSpace()->createColorTransformation("hsv_adjustment", m_params);
        saturationId = transfo->parameterId(SATURATION);
    }
    
    
    rotateBristles(angle);
    // if this is first time the brush touches the canvas and we use soak the ink from canvas
    if (firstStroke() && m_properties->useSoakInk){
        KisRandomConstAccessor laccessor = layer->createRandomConstAccessor((int)x1, (int)y1);
        colorifyBristles(laccessor,layer->colorSpace() ,pi1.pos());
    }

    qreal fx1, fy1, fx2, fy2;
    qreal randomX, randomY;
    qreal shear; 
    
    QVector<QPointF> bristlePath; // path for single bristle
    
    QVariant saturationVariant;
    float inkDeplation;
    int inkDepletionSize = m_properties->inkDepletionCurve.size();
    int bristleCount = m_bristles.size();
    int bristlePathSize;
    qreal treshold = 1.0 - pi2.pressure();
    for (int i = 0; i < bristleCount; i++) {

        if (!m_bristles.at(i)->enabled()) continue;
        bristle = m_bristles[i];

        randomX = (drand48() * 2 - 1.0) * m_properties->randomFactor;
        randomY = (drand48() * 2 - 1.0) * m_properties->randomFactor;

        shear = pressure * m_properties->shearFactor;

        m_transform.reset();
        m_transform.scale(scale, scale);
        m_transform.translate(randomX, randomY);
        m_transform.shear(shear, shear);

        if (firstStroke() || (!m_properties->connectedPath)){
            // transform start dab
            m_transform.map(bristle->x(), bristle->y(), &fx1, &fy1);
            // transform end dab
            m_transform.map(bristle->x(), bristle->y(), &fx2, &fy2);
        }else{
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

        if ( m_properties->threshold && (bristle->length() < treshold) ) continue;
        // paint between first and last dab
        bristlePath = m_trajectory.getLinearTrajectory(QPointF(fx1, fy1), QPointF(fx2, fy2), 1.0);
        bristlePathSize = m_trajectory.size();

        bristleColor = bristle->color();
        for (int i = 0; i < bristlePathSize ; i++) {
            if (bristle->counter() >= inkDepletionSize - 1) {
                inkDeplation = m_properties->inkDepletionCurve[inkDepletionSize - 1];
            } else {
                inkDeplation = m_properties->inkDepletionCurve[bristle->counter()];
            }

            // saturation transformation of the bristle ink color
            // add check for hsv transformation
            if (m_properties->useSaturation && transfo != 0) {
                if (m_properties->useWeights) {
                    // new weighted way (experiment)
                    saturationVariant = (
                                            (pressure * m_properties->pressureWeight) +
                                            (bristle->length() * m_properties->bristleLengthWeight) +
                                            (bristle->inkAmount() * m_properties->bristleInkAmountWeight) +
                                            ((1.0 - inkDeplation) * m_properties->inkDepletionWeight)) - 1.0;
                } else {
                    // old way of computing saturation
                    saturationVariant = (
                                            pressure *
                                            bristle->length() *
                                            bristle->inkAmount() *
                                            (1.0 - inkDeplation)) - 1.0;

                }
                transfo->setParameter(saturationId, saturationVariant);
                transfo->transform(bristleColor.data(), bristleColor.data() , 1);
            }

            // opacity transformation of the bristle color
            if (m_properties->useOpacity) {
                qreal opacity = OPACITY_OPAQUE_F;
                if (m_properties->useWeights) {
                    opacity = qBound(0.0,
                                  (pressure * m_properties->pressureWeight) +
                                  (bristle->length() * m_properties->bristleLengthWeight) +
                                  (bristle->inkAmount() * m_properties->bristleInkAmountWeight) +
                                  ((1.0 - inkDeplation) * m_properties->inkDepletionWeight),1.0);

                } else {
                    opacity =
                        bristle->length() *
                        bristle->inkAmount();
                }
                bristleColor.setOpacity(opacity);
            }

            addBristleInk(bristle, bristlePath.at(i), bristleColor);
            bristle->setInkAmount(1.0 - inkDeplation);
            bristle->upIncrement();
        }

    }
    rotateBristles(-angle);
    //repositionBristles(angle,slope);
    m_dab = 0;
    m_dabAccessor = 0;
}


void HairyBrush::rotateBristles(double angle)
{
    qreal tx, ty, x, y;

    m_transform.reset();
    m_transform.rotateRadians(angle);

    for (int i = 0; i < m_bristles.size(); i++) {
        if (m_bristles.at(i)->enabled()){
            x = m_bristles.at(i)->x();
            y = m_bristles.at(i)->y();
            m_transform.map(x, y, &tx, &ty);
            m_bristles[i]->setX(tx);
            m_bristles[i]->setY(ty);
        }
    }
    m_lastAngle = angle;
}

void HairyBrush::repositionBristles(double angle, double slope)
{
    // setX
    srand48((int)slope);
    for (int i = 0; i < m_bristles.size(); i++) {
        float x = m_bristles[i]->x();
        m_bristles[i]->setX(x + drand48());
    }

    // setY
    srand48((int)angle);
    for (int i = 0; i < m_bristles.size(); i++) {
        float y = m_bristles[i]->y();
        m_bristles[i]->setY(y + drand48());
    }
}

inline void HairyBrush::addBristleInk(Bristle *bristle, QPointF pos, const KoColor &color)
{
    if (m_properties->antialias)
    {
        if (m_properties->useCompositing){
            paintParticle(pos, color);
        }else{
            paintParticle(pos,color,1.0);
        }
    }
    else
    {
        int ix = qRound(pos.x());
        int iy = qRound(pos.y());
        if (m_properties->useCompositing){
            plotPixel(ix,iy,color);
        }else{
            darkenPixel(ix,iy,color);
        }
    }
}

void HairyBrush::paintParticle(QPointF pos, const KoColor& color,qreal weight)
{
    // opacity top left, right, bottom left, right
    quint8 opacity = color.opacityU8();
    opacity *= weight;
    
    int ipx = int (pos.x());
    int ipy = int (pos.y());
    qreal fx = pos.x() - ipx;
    qreal fy = pos.y() - ipy;

    quint8 btl = qRound((1.0 - fx) * (1.0 - fy) * opacity);
    quint8 btr = qRound((fx)  * (1.0 - fy) * opacity);
    quint8 bbl = qRound((1.0 - fx) * (fy)  * opacity);
    quint8 bbr = qRound((fx)  * (fy)  * opacity);

    const KoColorSpace * cs = m_dab->colorSpace();
    
    m_dabAccessor->moveTo(ipx  , ipy);
    btl = quint8(qBound<quint16>(OPACITY_TRANSPARENT_U8,btl + cs->opacityU8(m_dabAccessor->rawData()),OPACITY_OPAQUE_U8));
    memcpy(m_dabAccessor->rawData(), color.data(), cs->pixelSize());
    cs->setOpacity(m_dabAccessor->rawData(),btl,1);

    m_dabAccessor->moveTo(ipx + 1, ipy);
    btr =  quint8(qBound<quint16>(OPACITY_TRANSPARENT_U8,btr + cs->opacityU8(m_dabAccessor->rawData()),OPACITY_OPAQUE_U8));
    memcpy(m_dabAccessor->rawData(), color.data(), cs->pixelSize());
    cs->setOpacity(m_dabAccessor->rawData(),btr,1);
    
    m_dabAccessor->moveTo(ipx, ipy + 1);
    bbl = quint8(qBound<quint16>(OPACITY_TRANSPARENT_U8,bbl + cs->opacityU8(m_dabAccessor->rawData()),OPACITY_OPAQUE_U8));
    memcpy(m_dabAccessor->rawData(), color.data(), cs->pixelSize());
    cs->setOpacity(m_dabAccessor->rawData(),bbl,1);

    m_dabAccessor->moveTo(ipx + 1, ipy + 1);
    bbr = quint8(qBound<quint16>(OPACITY_TRANSPARENT_U8,bbr + cs->opacityU8(m_dabAccessor->rawData()),OPACITY_OPAQUE_U8));
    memcpy(m_dabAccessor->rawData(), color.data(), cs->pixelSize());
    cs->setOpacity(m_dabAccessor->rawData(),bbr,1);
}

void HairyBrush::paintParticle(QPointF pos, const KoColor& color)
{
    // opacity top left, right, bottom left, right
    memcpy(m_color.data(), color.data(), m_pixelSize);
    quint8 opacity = color.opacityU8();
    
    int ipx = int (pos.x());
    int ipy = int (pos.y());
    qreal fx = pos.x() - ipx;
    qreal fy = pos.y() - ipy;

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
    m_compositeOp->composite(m_dabAccessor->rawData(), m_pixelSize, color.data() , m_pixelSize, 0,0,1,1,OPACITY_OPAQUE_U8);
}

inline void HairyBrush::darkenPixel(int wx, int wy, const KoColor &color){
    m_dabAccessor->moveTo(wx, wy);
    if (m_dab->colorSpace()->opacityU8(m_dabAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_dabAccessor->rawData(), color.data(), m_pixelSize);
    }
}

double HairyBrush::computeMousePressure(double distance)
{
    double scale = 20.0;
    double minPressure = 0.02;
    double oldPressure = m_oldPressure;

    double factor = 1.0 - distance / scale;
    if (factor < 0.0) factor = 0.0;

    double result = ((4.0 * oldPressure) + minPressure + factor) / 5.0;
    m_oldPressure = result;
    return result;
}


void HairyBrush::colorifyBristles(KisRandomConstAccessor& acc, KoColorSpace * cs, QPointF point)
{
    QPoint p = point.toPoint();
    KoColor color(cs);
    int pixelSize = cs->pixelSize();

    Bristle *b = 0;
    int size = m_bristles.size();
    for (int i = 0; i < size; i++){
        b = m_bristles[i];
        int x = qRound(b->x() + point.x());
        int y = qRound(b->y() + point.y());
        acc.moveTo(x,y);
        memcpy(color.data(),acc.rawData(),pixelSize);
        b->setColor(color);
    }
    
}


