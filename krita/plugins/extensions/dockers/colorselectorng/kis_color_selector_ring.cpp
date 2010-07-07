#include "kis_color_selector_ring.h"

#include <QPainter>
#include <QVector2D>

#include <Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN
#include <cmath>

#include "KoColor.h"

#include <KDebug>
        
KisColorSelectorRing::KisColorSelectorRing(KisColorSelectorBase *parent) :
    KisColorSelectorComponent(parent),
    m_cachedColorSpace(0)
{
}

void KisColorSelectorRing::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if(colorSpace()!=m_cachedColorSpace || true) {
//        kDebug()<<"##################painting cache start";
        m_cachedColorSpace = colorSpace();
        paintCache();
//        kDebug()<<"##################painting cache end";
    }
    
//    kDebug()<<"###################painting ring start";
//    int size = qMin(width(), height());
    p.drawPixmap(0,0, m_cache);
//    kDebug()<<"###################painting ring end";
    
}

void KisColorSelectorRing::paintCache()
{
    QImage cache(width(), height(), QImage::Format_ARGB32_Premultiplied);
    cache.fill(qRgba(0,0,0,0));
    
    Vector2i center(cache.width()/2., cache.height()/2.);
    const KoColorSpace* colorSpace = this->colorSpace();
    
    int outerRadiusSquared = qMin(cache.width(), cache.height())/2;
    int innerRadiusSquared = outerRadiusSquared*0.7;
    outerRadiusSquared*=outerRadiusSquared;
    innerRadiusSquared*=innerRadiusSquared;
    
    for(int x=0; x<cache.width(); x++) {
        for(int y=0; y<cache.height(); y++) {
            Vector2i currentPoint((float)x, (float)y);
            Vector2i relativeVector = currentPoint-center;
            
            if(relativeVector.squaredNorm() < outerRadiusSquared
               && relativeVector.squaredNorm() > innerRadiusSquared) {
                
                float angle = std::atan2(relativeVector.y(), relativeVector.x())+((float)M_PI);
                float hue = angle/(2*((float)M_PI));
                kDebug()<<"======================hue:"<<hue;
//                hue *= 255;
                KoColor c(QColor::fromHsvF(hue, 1, 1), colorSpace);
                cache.setPixel(x, y, c.toQColor().rgb());
//                cache.setPixel(x, y, QColor::fromHsvF(hue, 1, 1).rgb());
//                cache.setPixel(x, y, QColor::fromHsv(hue*359, 255, 255).rgb());
            }
        }
    }
    m_cache = QPixmap::fromImage(cache);
}
