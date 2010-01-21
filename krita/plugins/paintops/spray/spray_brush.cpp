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


#include <QVariant>
#include <QHash>
#include <QTransform>
#include <QImage>
#include <QMatrix>

#include <kis_random_accessor.h>
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
    m_rand = new RandomGauss(time(0));
}

SprayBrush::~SprayBrush()
{
    delete m_painter;
    delete m_rand;
}


qreal SprayBrush::rotationAngle()
{
    qreal rotation = 0.0;
    
    if ( m_properties->fixedRotation ){
        rotation = deg2rad( m_properties->fixedAngle );
    }
    
    if ( m_properties->randomRotation ){
        
        if ( m_properties->gaussian ) {
                rotation = linearInterpolation(rotation ,M_PI * 2.0 * qBound(0.0, m_rand->nextGaussian(0.0, 0.50) , 1.0), m_properties->randomRotationWeight );
        } else {
                rotation = linearInterpolation(rotation, M_PI * 2.0 * drand48(), m_properties->randomRotationWeight );
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
        m_painter->setMaskImageSize(m_properties->width, m_properties->height );
        m_pixelSize = dab->colorSpace()->pixelSize();
        
        m_brushQImage = m_properties->image;
        if (!m_brushQImage.isNull()){
            m_brushQImage = m_brushQImage.scaled(m_properties->width, m_properties->height);
        }
        m_imageDevice = new KisPaintDevice( dab->colorSpace() );
    }


    qreal x = info.pos().x();
    qreal y = info.pos().y();
    KisRandomAccessor accessor = dab->createRandomAccessor(qRound(x), qRound(y));
    KisRandomSubAccessorPixel subAcc = source->createRandomSubAccessor();
    
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
    qreal rotationZ;

    int steps = 118;
    bool shouldColor = true;
    if (m_colorProperties->fillBackground){
        m_painter->setPaintColor(bgColor);
        paintCircle(m_painter,x,y,m_radius,steps);
    }

    QMatrix m;
    m.reset();
    // constant user rotation
    m.rotate( m_properties->brushRotation );
    // rotation from the sensor
    m.rotate( rad2deg(rotation) );
    m.scale( m_properties->scale, m_properties->scale);

    for (quint32 i = 0; i < m_particlesCount; i++){
        // generate random angle
        angle = drand48() * M_PI * 2;

        // generate random length 
        if ( m_properties->gaussian ) {
            length = qBound(0.0, m_rand->nextGaussian(0.0, 0.50) , 1.0);
        } else {
            length = drand48();
        }

        // rotation
        rotationZ = rotationAngle();
        
        if (m_properties->followCursor){
            rotationZ = linearInterpolation( rotationZ,angle,m_properties->followCursorWeigth );
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
                subAcc.moveTo(nx+x, ny+y);
                subAcc.sampledRawData( m_inkColor.data() );
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

            if (m_colorProperties->useRandomHSV){
                params["h"] = (m_colorProperties->hue / 180.0) * drand48();
                params["s"] = (m_colorProperties->saturation / 100.0) * drand48();
                params["v"] = (m_colorProperties->value / 100.0) * drand48();

                KoColorTransformation* transfo;
                transfo = dab->colorSpace()->createColorTransformation("hsv_adjustment", params);
                transfo->transform(m_inkColor.data(), m_inkColor.data() , 1);
            }
                
            if (m_colorProperties->useRandomOpacity){
                quint8 alpha = qRound(drand48() * OPACITY_OPAQUE);
                m_inkColor.setOpacity( alpha );
                m_painter->setOpacity( alpha );
            }

            if ( !m_colorProperties->colorPerParticle ){
                shouldColor = false;
            }
            m_painter->setPaintColor(m_inkColor);
        }

        qreal random = drand48();
        qreal jitteredWidth;
        qreal jitteredHeight; 

        if (m_properties->randomSize){
            jitteredWidth = m_properties->width * random + 1;
            jitteredHeight = m_properties->height * random + 1;
        }else{
            jitteredWidth = m_properties->width;
            jitteredHeight = m_properties->height;
        }
        switch (m_properties->shape){
            // ellipse
            case 0:
            {
                if (m_properties->width == m_properties->height){
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
                accessor.moveTo(ix, iy);
                memcpy(accessor.rawData(), m_inkColor.data(), m_pixelSize);
                break;
            }
            case 4:
            {
                if ( !m_brushQImage.isNull() )
                {
                    
                    QMatrix m;
                    m.rotate(rad2deg(rotationZ));

                    if (m_properties->randomSize){
                        m.scale(random,random);
                    }
                    m_transformed = m_brushQImage.transformed(m, Qt::SmoothTransformation);
                    m_imageDevice->convertFromQImage(m_transformed, "");
                    KisRandomAccessor ac = m_imageDevice->createRandomAccessor(0,0);
                    QRect rc = m_transformed.rect();

                    if (m_colorProperties->useRandomHSV){
                        params["h"] = (m_colorProperties->hue / 180.0) * drand48();
                        params["s"] = (m_colorProperties->saturation / 100.0) * drand48();
                        params["v"] = (m_colorProperties->value / 100.0) * drand48();

                        KoColorTransformation* transfo;
                        transfo = dab->colorSpace()->createColorTransformation("hsv_adjustment", params);

                        for (int y = rc.y(); y< rc.y()+rc.height(); y++){
                            for (int x = rc.x(); x < rc.x()+rc.width();x++){
                                ac.moveTo(x,y);
                                transfo->transform(ac.rawData(), ac.rawData() , 1);    
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
        
    }
    // hidden code for outline detection
    //m_inkColor.setOpacity(128);
    //paintOutline(dev,m_inkColor,x, y, m_radius * 2);

    // recover from jittering of color,
    // m_inkColor.opacity is recovered with every paint
}



void SprayBrush::paintParticle(KisRandomAccessor &writeAccessor, const KoColor &color, qreal rx, qreal ry)
{
    // opacity top left, right, bottom left, right
    KoColor pcolor(color);
    int opacity = pcolor.opacity();

    int ipx = int (rx);
    int ipy = int (ry);
    qreal fx = rx - ipx;
    qreal fy = ry - ipy;

    int btl = qRound((1 - fx) * (1 - fy) * opacity);
    int btr = qRound((fx)  * (1 - fy) * opacity);
    int bbl = qRound((1 - fx) * (fy)  * opacity);
    int bbr = qRound((fx)  * (fy)  * opacity);

    // this version overwrite pixels, e.g. when it sprays two particle next
    // to each other, the pixel with lower opacity can override other pixel.
    // Maybe some kind of compositing using here would be cool

    pcolor.setOpacity(btl);
    writeAccessor.moveTo(ipx  , ipy);
    memcpy(writeAccessor.rawData(), pcolor.data(), m_pixelSize);

    pcolor.setOpacity(btr);
    writeAccessor.moveTo(ipx + 1, ipy);
    memcpy(writeAccessor.rawData(), pcolor.data(), m_pixelSize);

    pcolor.setOpacity(bbl);
    writeAccessor.moveTo(ipx, ipy + 1);
    memcpy(writeAccessor.rawData(), pcolor.data(), m_pixelSize);

    pcolor.setOpacity(bbr);
    writeAccessor.moveTo(ipx + 1, ipy + 1);
    memcpy(writeAccessor.rawData(), pcolor.data(), m_pixelSize);
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
    KisRandomAccessor accessor = dev->createRandomAccessor(qRound(posX), qRound(posY));

    for (int y = -radius + posY; y <= radius + posY; y++) {
        for (int x = -radius + posX; x <= radius + posX; x++) {
            accessor.moveTo(x, y);
            qreal alpha = dev->colorSpace()->alpha(accessor.rawData());

            if (alpha != 0) {
                // top left
                accessor.moveTo(x - 1, y - 1);
                if (dev->colorSpace()->alpha(accessor.rawData()) == 0) {
                    antiPixels.append(QPointF(x - 1, y - 1));
                    //continue;
                }

                // top
                accessor.moveTo(x, y - 1);
                if (dev->colorSpace()->alpha(accessor.rawData()) == 0) {
                    antiPixels.append(QPointF(x, y - 1));
                    //continue;
                }

                // top right
                accessor.moveTo(x + 1, y - 1);
                if (dev->colorSpace()->alpha(accessor.rawData()) == 0) {
                    antiPixels.append(QPointF(x + 1, y - 1));
                    //continue;
                }

                //left
                accessor.moveTo(x - 1, y);
                if (dev->colorSpace()->alpha(accessor.rawData()) == 0) {
                    antiPixels.append(QPointF(x - 1, y));
                    //continue;
                }

                //right
                accessor.moveTo(x + 1, y);
                if (dev->colorSpace()->alpha(accessor.rawData()) == 0) {
                    antiPixels.append(QPointF(x + 1, y));
                    //continue;
                }

                // bottom left
                accessor.moveTo(x - 1, y + 1);
                if (dev->colorSpace()->alpha(accessor.rawData()) == 0) {
                    antiPixels.append(QPointF(x - 1, y + 1));
                    //continue;
                }

                // bottom
                accessor.moveTo(x, y + 1);
                if (dev->colorSpace()->alpha(accessor.rawData()) == 0) {
                    antiPixels.append(QPointF(x, y + 1));
                    //continue;
                }

                // bottom right
                accessor.moveTo(x + 1, y + 1);
                if (dev->colorSpace()->alpha(accessor.rawData()) == 0) {
                    antiPixels.append(QPointF(x + 1, y + 1));
                    //continue;
                }
            }

        }
    }

    // anti-alias it
    int size = antiPixels.size();
    for (int i = 0; i < size; i++) {
        accessor.moveTo(antiPixels[i].x(), antiPixels[i].y());
        memcpy(accessor.rawData(), outlineColor.data(), dev->colorSpace()->pixelSize());
    }
}
