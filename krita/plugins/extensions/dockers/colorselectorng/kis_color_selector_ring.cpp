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

#include <Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN
#include <cmath>

#include "KoColor.h"

#include <KDebug>
        
KisColorSelectorRing::KisColorSelectorRing(KisColorSelectorBase *parent) :
    KisColorSelectorComponent(parent),
    m_cachedColorSpace(0),
    m_cachedSize(0)
{
}

int KisColorSelectorRing::innerRadius() const
{
    return (qMin(width(), height())/2)*0.7;
}

void KisColorSelectorRing::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if(colorSpace()!=m_cachedColorSpace) {
        m_cachedColorSpace = colorSpace();
        colorCache();
        paintCache();
        m_cachedSize=qMin(width(), height());
    }
    
    int size = qMin(width(), height());
    if(m_cachedSize!=size) {
        m_cachedSize=size;
        paintCache();
    }
    
    p.drawImage(0,0, m_pixelCache);
}

void KisColorSelectorRing::paintCache()
{
    QImage cache(width(), height(), QImage::Format_ARGB32_Premultiplied);
    cache.fill(qRgba(0,0,0,0));
    
    Vector2i center(cache.width()/2., cache.height()/2.);
    
    int outerRadiusSquared = qMin(cache.width(), cache.height())/2;
    int innerRadiusSquared = innerRadius();
    outerRadiusSquared*=outerRadiusSquared;
    innerRadiusSquared*=innerRadiusSquared;
    
    for(int x=0; x<cache.width(); x++) {
        for(int y=0; y<cache.height(); y++) {
            Vector2i currentPoint((float)x, (float)y);
            Vector2i relativeVector = currentPoint-center;
            
            if(relativeVector.squaredNorm() < outerRadiusSquared
               && relativeVector.squaredNorm() > innerRadiusSquared) {
                
                float angle = std::atan2(relativeVector.y(), relativeVector.x())+((float)M_PI);
                angle/=2*((float)M_PI);
                angle*=359.f;
                cache.setPixel(x, y, m_cachedColors.at(angle));
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
