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

#include "spray_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include "kis_paintop.h"

#include <QVariant>
#include <QHash>
#include <QTransform>
#include <QImage>
#include <QTransform>

#include <kis_random_accessor_ng.h>
#include <kis_random_sub_accessor.h>

#include <kis_paint_device.h>

#include <kis_painter.h>
#include <kis_paint_information.h>

#include "kis_spray_paintop_settings.h"

#include <cmath>
#include <ctime>

#include "random_gauss.h"

SprayBrush::SprayBrush()
{
    srand48(time(0));
    m_painter = 0;
    m_transfo = 0;
    m_rand = new RandomGauss(time(0));
}

SprayBrush::~SprayBrush()
{
    delete m_painter;
    delete m_transfo;
    delete m_rand;
}


qreal SprayBrush::rotationAngle()
{
    qreal rotation = 0.0;

    if ( m_shapeDynamicsProperties->fixedRotation ){
        rotation = deg2rad( m_shapeDynamicsProperties->fixedAngle );
    }

    if ( m_shapeDynamicsProperties->randomRotation ){

        if ( m_properties->gaussian ) {
                rotation = linearInterpolation(rotation ,M_PI * 2.0 * qBound<qreal>(0.0, m_rand->nextGaussian(0.0, 0.50) , 1.0), m_shapeDynamicsProperties->randomRotationWeight );
        } else {
                rotation = linearInterpolation(rotation, M_PI * 2.0 * drand48(), m_shapeDynamicsProperties->randomRotationWeight );
        }
    }

    return rotation;
}



void SprayBrush::paint(KisPaintDeviceSP dab, KisPaintDeviceSP source,
                       const KisPaintInformation& info,qreal rotation, qreal scale,
                       const KoColor &color, const KoColor &bgColor)
{
    // initializing painter

    if (!m_painter) {
        m_painter = new KisPainter(dab);
        m_painter->setFillStyle(KisPainter::FillStyleForegroundColor);
        m_painter->setMaskImageSize(m_shapeProperties->width, m_shapeProperties->height );
        m_pixelSize = dab->colorSpace()->pixelSize();
        if (m_colorProperties->useRandomHSV){
            m_transfo = dab->colorSpace()->createColorTransformation("hsv_adjustment", QHash<QString, QVariant>());
        }
        
        m_brushQImage = m_shapeProperties->image;
        if (!m_brushQImage.isNull()){
            m_brushQImage = m_brushQImage.scaled(m_shapeProperties->width, m_shapeProperties->height);
        }
        m_imageDevice = new KisPaintDevice( dab->colorSpace() );
    }


    qreal x = info.pos().x();
    qreal y = info.pos().y();
    KisRandomAccessorSP accessor = dab->createRandomAccessorNG(qRound(x), qRound(y));
    KisRandomSubAccessorSP subAcc = source->createRandomSubAccessor();

    m_inkColor = color;

    // apply size sensor
    m_radius = m_properties->radius * scale;

    // jitter movement
    if ( m_properties->jitterMovement ) {
        x = x + ((2 * m_radius * drand48()) - m_radius) * m_properties->amount;
        y = y + ((2 * m_radius * drand48()) - m_radius) * m_properties->amount;
    }

    // this is wrong for every shape except pixel and anti-aliased pixel


    if (m_properties->useDensity) {
        m_particlesCount = (m_properties->coverage * (M_PI * m_radius * m_radius));
    }else{
        m_particlesCount = m_properties->particleCount;
    }

    QHash<QString, QVariant> params;
    qreal nx, ny;
    int ix, iy;

    qreal angle;
    qreal length;
    qreal rotationZ = 0.0;
    qreal particleScale = 1.0;

    int steps = 118;
    bool shouldColor = true;
    if (m_colorProperties->fillBackground){
        m_painter->setPaintColor(bgColor);
        paintCircle(m_painter,x,y,m_radius,steps);
    }

    QTransform m;
    m.reset();
    m.rotateRadians(-rotation + deg2rad(m_properties->brushRotation) );
    m.scale( m_properties->scale, m_properties->scale);
    
    for (quint32 i = 0; i < m_particlesCount; i++){
        // generate random angle
        angle = drand48() * M_PI * 2;

        // generate random length
        if ( m_properties->gaussian ) {
            length = qBound<qreal>(0.0, m_rand->nextGaussian(0.0, 0.50) , 1.0);
        } else {
            length = drand48();
        }

        if (m_shapeDynamicsProperties->enabled){
            // rotation
            rotationZ = rotationAngle();

            if (m_shapeDynamicsProperties->followCursor){
                
                rotationZ = linearInterpolation( rotationZ,angle,m_shapeDynamicsProperties->followCursorWeigth );
            }

            
            if (m_shapeDynamicsProperties->followDrawingAngle){
                
                rotationZ = linearInterpolation( rotationZ,info.angle(),m_shapeDynamicsProperties->followDrawingAngleWeight );
            }

            // random size - scale
            if (m_shapeDynamicsProperties->randomSize){
                particleScale = drand48();
            }
        }
        // generate polar coordinate
        nx = (m_radius * cos(angle)  * length);
        ny = (m_radius * sin(angle)  * length);

        // compute the height of the ellipse
        ny *= m_properties->aspect;

        // transform
        m.map(nx,ny, &nx,&ny);

        // color transformation
        
        if (shouldColor){
            if (m_colorProperties->sampleInputColor){
                subAcc->moveTo(nx+x, ny+y);
                subAcc->sampledOldRawData( m_inkColor.data() );
            }else{
                 //revert the color
                 memcpy(m_inkColor.data(),color.data(), m_pixelSize);
            }

            // mix the color with background color
            if (m_colorProperties->mixBgColor)
            {
                KoMixColorsOp * mixOp = source->colorSpace()->mixColorsOp();

                const quint8 *colors[2];
                colors[0] = m_inkColor.data();
                colors[1] = bgColor.data();

                qint16 colorWeights[2];
                int MAX_16BIT = 255;
                qreal blend = info.pressure();

                colorWeights[0] = static_cast<quint16>( blend * MAX_16BIT);
                colorWeights[1] = static_cast<quint16>( (1.0 - blend) * MAX_16BIT);
                mixOp->mixColors(colors, colorWeights, 2, m_inkColor.data() );
            }

            if (m_colorProperties->useRandomHSV && m_transfo){
                params["h"] = (m_colorProperties->hue / 180.0) * drand48();
                params["s"] = (m_colorProperties->saturation / 100.0) * drand48();
                params["v"] = (m_colorProperties->value / 100.0) * drand48();
                m_transfo->setParameters(params);
                m_transfo->transform(m_inkColor.data(), m_inkColor.data() , 1);
            }

            if (m_colorProperties->useRandomOpacity){
                quint8 alpha = qRound(drand48() * OPACITY_OPAQUE_U8);
                m_inkColor.setOpacity( alpha );
                m_painter->setOpacity( alpha );
            }

            if ( !m_colorProperties->colorPerParticle ){
                shouldColor = false;
            }
            m_painter->setPaintColor(m_inkColor);
        }

        qreal jitteredWidth = qMax(qreal(1.0),m_shapeProperties->width * particleScale);
        qreal jitteredHeight = qMax(qreal(1.0),m_shapeProperties->height * particleScale);

        if (m_shapeProperties->enabled){
        switch (m_shapeProperties->shape){
            // ellipse
            case 0:
            {
                if (m_shapeProperties->width == m_shapeProperties->height){
                    paintCircle(m_painter, nx + x, ny + y, qRound(jitteredWidth * 0.5) , steps);
                }else
                {
                    paintEllipse(m_painter, nx + x, ny + y, qRound(jitteredWidth * 0.5) , qRound(jitteredHeight * 0.5), rotationZ , steps);
                }
                break;
            }
            // rectangle
            case 1:
            {
                paintRectangle(m_painter, nx + x, ny + y, qRound(jitteredWidth) , qRound(jitteredHeight), rotationZ , steps);
                break;
            }
            // wu-particle
            case 2:
            {
                paintParticle(accessor, m_inkColor, nx + x, ny + y);
                break;
            }
            // pixel
            case 3:
            {
                ix = qRound(nx + x);
                iy = qRound(ny + y);
                accessor->moveTo(ix, iy);
                memcpy(accessor->rawData(), m_inkColor.data(), m_pixelSize);
                break;
            }
            case 4:
            {
                if ( !m_brushQImage.isNull() )
                {

                    QTransform m;
                    m.rotate(rad2deg(rotationZ));

                    if (m_shapeDynamicsProperties->randomSize){
                        m.scale(particleScale,particleScale);
                    }
                    m_transformed = m_brushQImage.transformed(m, Qt::SmoothTransformation);
                    m_imageDevice->convertFromQImage(m_transformed, 0);
                    KisRandomAccessorSP ac = m_imageDevice->createRandomAccessorNG(0,0);
                    QRect rc = m_transformed.rect();

                    if (m_colorProperties->useRandomHSV && m_transfo){

                        for (int y = rc.y(); y< rc.y()+rc.height(); y++){
                            for (int x = rc.x(); x < rc.x()+rc.width();x++){
                                ac->moveTo(x,y);
                                m_transfo->transform(ac->rawData(), ac->rawData() , 1);
                            }
                        }
                    }

                    ix = qRound(nx + x - rc.width() * 0.5);
                    iy = qRound(ny + y - rc.height() * 0.5);
                    m_painter->bitBlt(QPoint(ix,iy), m_imageDevice, rc);
                    m_imageDevice->clear();
                    break;
                }
            }
        }
        // Auto-brush
        }else{
            QPointF hotSpot = m_brush->hotSpot(particleScale, particleScale, -rotationZ);
            QPointF pos(nx + x, ny + y);
            QPointF pt = pos - hotSpot;

            qint32 ix;
            qreal xFraction;
            qint32 iy;
            qreal yFraction;

            KisPaintOp::splitCoordinate(pt.x(), &ix, &xFraction);
            KisPaintOp::splitCoordinate(pt.y(), &iy, &yFraction);

            //KisFixedPaintDeviceSP dab;
            if (m_brush->brushType() == IMAGE || 
                m_brush->brushType() == PIPE_IMAGE) 
            {
                m_fixedDab = m_brush->paintDevice(m_fixedDab->colorSpace(), particleScale, -rotationZ, info, xFraction, yFraction);

                if (m_colorProperties->useRandomHSV && m_transfo){
                    quint8 * dabPointer = m_fixedDab->data();
                    int pixelCount = m_fixedDab->bounds().width() * m_fixedDab->bounds().height();
                    m_transfo->transform(dabPointer, dabPointer, pixelCount);
                }
                
            } else {
                m_brush->mask(m_fixedDab, m_inkColor, particleScale, particleScale, -rotationZ, info, xFraction, yFraction);
            }
            m_painter->bltFixed(QPoint(ix, iy), m_fixedDab, m_fixedDab->bounds());
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
    memcpy(writeAccessor->rawData(), pcolor.data(), m_pixelSize);

    pcolor.setOpacity(btr);
    writeAccessor->moveTo(ipx + 1, ipy);
    memcpy(writeAccessor->rawData(), pcolor.data(), m_pixelSize);

    pcolor.setOpacity(bbl);
    writeAccessor->moveTo(ipx, ipy + 1);
    memcpy(writeAccessor->rawData(), pcolor.data(), m_pixelSize);

    pcolor.setOpacity(bbr);
    writeAccessor->moveTo(ipx + 1, ipy + 1);
    memcpy(writeAccessor->rawData(), pcolor.data(), m_pixelSize);
}

void SprayBrush::paintCircle(KisPainter * painter, qreal x, qreal y, int radius, int steps)
{
    QPainterPath path;
    // circle x, circle y
    qreal cx, cy;

    qreal length = 2.0 * M_PI;
    qreal step = 1.0 / steps;
    path.moveTo(radius + x, y);
    for (int i = 1; i < steps; i++) {
        cx = cos(i * step * length);
        cy = sin(i * step * length);

        cx *= radius;
        cy *= radius;

        cx += x;
        cy += y;

        path.lineTo(cx, cy);
    }
    path.closeSubpath();
    painter->fillPainterPath(path);
}



void SprayBrush::paintEllipse(KisPainter* painter, qreal x, qreal y, int a, int b, qreal angle, int steps)
{
    QPainterPath path;
    qreal beta = angle;
    qreal sinbeta = sin(beta);
    qreal cosbeta = cos(beta);

    path.moveTo(x + a * cosbeta, y + a * sinbeta);
    qreal step = 360.0 / steps;
    for (int i = step; i < 360; i += step) {
        qreal alpha = i * (M_PI / 180) ;
        qreal sinalpha = sin(alpha);
        qreal cosalpha = cos(alpha);

        qreal X = x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta);
        qreal Y = y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta);

        path.lineTo(X, Y);
    }
    path.closeSubpath();
    painter->fillPainterPath(path);
}

void SprayBrush::paintRectangle(KisPainter* painter, qreal x, qreal y, int width, int height, qreal angle, int steps)
{
    Q_UNUSED(steps);
    QPainterPath path;
    QTransform transform;

    qreal halfWidth = width * 0.5;
    qreal halfHeight = height * 0.5;
    qreal tx, ty;


    transform.reset();
    transform.rotateRadians(angle);
    // top left
    transform.map(- halfWidth,  - halfHeight, &tx, &ty);
    path.moveTo(QPointF(tx + x, ty + y));
    // top right
    transform.map(+ halfWidth,  - halfHeight, &tx, &ty);
    path.lineTo(QPointF(tx + x, ty + y));
    // bottom right
    transform.map(+ halfWidth,  + halfHeight, &tx, &ty);
    path.lineTo(QPointF(tx + x, ty + y));
    // botom left
    transform.map(- halfWidth,  + halfHeight, &tx, &ty);
    path.lineTo(QPointF(tx + x, ty + y));
    path.closeSubpath();
    painter->fillPainterPath(path);
}


void SprayBrush::paintOutline(KisPaintDeviceSP dev , const KoColor &outlineColor, qreal posX, qreal posY, qreal radius)
{
    QList<QPointF> antiPixels;
    KisRandomAccessorSP accessor = dev->createRandomAccessorNG(qRound(posX), qRound(posY));

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
