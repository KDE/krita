/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
 */

#include "kis_color_selector_ring.h"

#include <QPainter>
#include <QMouseEvent>

#include <Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN
#include <cmath>

#include "KoColor.h"

KisColorSelectorRing::KisColorSelectorRing(KisColorSelector *parent) :
    KisColorSelectorComponent(parent),
    m_cachedColorSpace(0),
    m_cachedSize(0),
    m_lastHue(0)
{
}

int KisColorSelectorRing::innerRadius() const
{
    return (qMin(width(), height())/2)*0.85;
}

bool KisColorSelectorRing::containsPointInComponentCoords(int x, int y) const
{
    int outerRadiusSquared = qMin(width(), height())/2;
    int innerRadiusSquared = innerRadius();
    outerRadiusSquared*=outerRadiusSquared;
    innerRadiusSquared*=innerRadiusSquared;
    
    
    Vector2i relativeVector(x-width()/2, y-height()/2);
    
    if(relativeVector.squaredNorm() < outerRadiusSquared
       && relativeVector.squaredNorm() > innerRadiusSquared) {
        return true;
    }
    return false;
}

void KisColorSelectorRing::paint(QPainter* painter)
{
    if(colorSpace()!=m_cachedColorSpace) {
        m_cachedColorSpace = colorSpace();
        m_cachedSize=qMin(width(), height());
        colorCache();
        paintCache();
    }
    
    int size = qMin(width(), height());
    if(m_cachedSize!=size) {
        m_cachedSize=size;
        paintCache();
    }
    
    painter->drawImage(width()/2-m_pixelCache.width()/2,
                height()/2-m_pixelCache.height()/2,
                m_pixelCache);


    // paint blip
    if(m_parent->displayBlip()) {
        qreal angle;
        int y_start, y_end, x_start, x_end;
        angle=m_lastHue*2.*M_PI+(M_PI);
        y_start=innerRadius()*sin(angle)+height()/2;
        y_end=outerRadius()*sin(angle)+height()/2;
        x_start=innerRadius()*cos(angle)+width()/2;
        x_end=outerRadius()*cos(angle)+width()/2;

        painter->setPen(QColor(0,0,0));
        painter->drawLine(x_start, y_start, x_end, y_end);

        angle+=M_PI/180.;
        y_start=innerRadius()*sin(angle)+height()/2;
        y_end=outerRadius()*sin(angle)+height()/2;
        x_start=innerRadius()*cos(angle)+width()/2;
        x_end=outerRadius()*cos(angle)+width()/2;

        painter->setPen(QColor(255,255,255));
        painter->drawLine(x_start, y_start, x_end, y_end);
    }
}

QColor KisColorSelectorRing::selectColor(int x, int y) {
    QPoint ringMiddle(width()/2, height()/2);
    QPoint ringCoord = QPoint(x, y)-ringMiddle;
    qreal hue = std::atan2(qreal(ringCoord.y()), qreal(ringCoord.x()))+(M_PI);
    hue/=2.*M_PI;
    emit paramChanged(hue, -1, -1, -1, -1);
    m_lastHue=hue;
    emit update();

    return QColor();
}

void KisColorSelectorRing::setColor(const QColor &color)
{
    // selector keeps the position on the ring if hue is undefined (when saturation is 0)
    if (!qFuzzyCompare(color.saturationF(), 0.0)) {
        emit paramChanged(color.hueF(), -1, -1, -1, -1);
        m_lastHue=color.hueF();
    } else {
        emit paramChanged(m_lastHue, -1, -1, -1, -1);
    }
    emit update();
}

void KisColorSelectorRing::paintCache()
{
    QImage cache(m_cachedSize, m_cachedSize, QImage::Format_ARGB32_Premultiplied);
    
    Vector2i center(cache.width()/2., cache.height()/2.);
    
    for(int x=0; x<cache.width(); x++) {
        for(int y=0; y<cache.height(); y++) {
            Vector2i currentPoint((float)x, (float)y);
            Vector2i relativeVector = currentPoint-center;

            qreal currentRadius = relativeVector.squaredNorm();
            currentRadius=sqrt(currentRadius);
            
            if(currentRadius < outerRadius()+1
               && currentRadius > innerRadius()-1)
            {

                float angle = std::atan2((float)relativeVector.y(), (float)relativeVector.x())+((float)M_PI);
                angle/=2*((float)M_PI);
                angle*=359.f;
                if(currentRadius < outerRadius()
                   && currentRadius > innerRadius()) {
                    cache.setPixel(x, y, m_cachedColors.at(angle));
                }
                else {
                    // draw antialiased border
                    qreal coef=1.;
                    if(currentRadius > outerRadius()) {
                        // outer border
                        coef-=currentRadius;
                        coef+=outerRadius();
                    }
                    else {
                        // inner border
                        coef+=currentRadius;
                        coef-=innerRadius();
                    }
                    coef=qBound(qreal(0.), coef, qreal(1.));
                    int red=qRed(m_cachedColors.at(angle));
                    int green=qGreen(m_cachedColors.at(angle));
                    int blue=qBlue(m_cachedColors.at(angle));

                    // the format is premultiplied, so we have to take care of that
                    QRgb color = qRgba(red*coef, green*coef, blue*coef, 255*coef);
                    cache.setPixel(x, y, color);
                }
            }
            else {
                cache.setPixel(x, y, qRgba(0,0,0,0));
            }
        }
    }
    m_pixelCache = cache;
}

void KisColorSelectorRing::colorCache()
{
    Q_ASSERT(m_cachedColorSpace);
    m_cachedColors.clear();
    KoColor koColor(m_cachedColorSpace);
    QColor qColor;
    for(int i=0; i<360; i++) {
        qColor.setHsv(i, 255, 255);
        koColor.fromQColor(qColor);
        m_cachedColors.append(koColor.toQColor().rgb());
    }
}

int KisColorSelectorRing::outerRadius() const
{
    return m_cachedSize/2-1;
}
