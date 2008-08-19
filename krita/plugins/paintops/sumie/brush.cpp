/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#include "brush.h"
#include "brush_shape.h"
#include "lines.h"
#include "gauss.h"
#include "trajectory.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>
#include <QList>

#include "kis_random_accessor.h"
#include "widgets/kcurve.h"

#include <cmath>
#include <ctime>

const float radToDeg = 57.29578;

#ifdef _WIN32
#define srand48 srand
#define drand48 rand
#endif

Brush::Brush(const BrushShape &initialShape, KoColor inkColor){
    m_initialShape = initialShape;
    m_inkColor = inkColor;    
    m_counter = 0;
}

Brush::Brush(){
    m_radius = 20;
    m_sigma = 20.f;    

    m_counter = 0;
    m_lastAngle = 0.0;
    m_oldPressure = 0.0f;

    BrushShape bs;
    //bs.fromGaussian( m_radius, m_sigma );
    bs.fromLine( m_radius, m_sigma );
    
    setBrushShape(bs);    
}

void Brush::setBrushShape(BrushShape brushShape){
    m_initialShape = brushShape;
    //dbgPlugins << "radius in setBrushShape: " << brushShape.width()/2 << endl;

    m_bristles = brushShape.getBristles();
}

void Brush::enableMousePressure(bool enable){
    m_mousePressureEnabled = enable;
}

void Brush::setInkColor(const KoColor &color){
    for (int i=0;i<m_bristles.size();i++){
        m_bristles[i].setColor(color);
    }
    m_inkColor = color;
}


void Brush::setInkDepletion(QList<float> *curveData){
    int count = curveData->size();

    for (int i = 0; i< count ;i++)
    {
        m_inkDepletion.append( curveData->at(i) );
    }
    delete curveData; // thank you, delete your self
}

void Brush::paint(KisPaintDeviceSP dev, const KisPaintInformation &info){
    Q_UNUSED(dev)
    Q_UNUSED(info)
/*    m_counter++;
    
    Bristle *bristle;
    KoColor brColor;
    int x = info.pos().x();
    int y = info.pos().y();

    qint32 pixelSize = m_dev->colorSpace()->pixelSize();
    KisRandomAccessor accessor = m_dev->createRandomAccessor( x,y );
    
    double inkDeplation;
    if ( m_counter >= m_inkDepletion.size()-1 ){
        inkDeplation = m_inkDepletion[m_inkDepletion.size() - 1];
    }else{
        inkDeplation = m_inkDepletion[m_counter];
    }

    QHash<QString, QVariant> params;
    params["h"] = 0.0;
    params["s"] = 0.0;
    params["v"] = 0.0;
    KoColorTransformation* transfo;

    double pressure = 1.0f;//computeMousePressure(info);

    BrushShape bs;
    bs.fromLine(m_initialShape.radius()+(int)(pressure+0.5) , m_initialShape.sigma() );
    setBrushShape(bs);

    dbgPlugins << "pressure: " << (int)(pressure+0.5) << endl;
    dbgPlugins << "pressure: " << pressure << endl;

//     dbgPlugins << m_counter ;
    
    int dx, dy; // used for counting the coords of bristles relative to the center of the brush
    bool rotate = true;

    float angleDistance = m_lastAngle - info.angle();
    float angleDec;

    if (angleDistance < 0){
        angleDec = +0.1;
    } else
    {
        angleDec = -0.1;
    }

    
*/
    /*
    * MAIN LOOP *HIGHLY* careful
    */
/*    while ( rotate )
    {
        for ( int i=0;i<m_bristles.size();i++ )
        {
//             if (m_bristles[i].distanceCenter() > m_radius || drand48() <0.5){
//                 continue;
//             }
            bristle = &m_bristles[i];
            brColor = bristle->color();
    
            // saturation
            params["s"] = - ( 1.0 - ( info.pressure() * bristle->length() * bristle->inkAmount() ) );
            transfo = m_dev->colorSpace()->createColorTransformation ( "hsv_adjustment", params );
            transfo->transform ( m_inkColor.data(), brColor.data() , 1 );
    
            // opacity
            brColor.setOpacity ( static_cast<int> ( 255.0* pressure*bristle->length() * bristle->inkAmount() * ( 1.0 - inkDeplation ) ) );
    
            dx = ( int ) ( x+bristle->x() );
            dy = ( int ) ( y+bristle->y() );
    
            accessor.moveTo ( dx,dy );
            memcpy ( accessor.rawData(), brColor.data(), pixelSize );
    
            bristle->setInkAmount ( 1.0 - inkDeplation - pressure );
            //bristle->setColor(brColor);
        }
        
        */
        return;
        //TODO When the angle is wide, paint every bristle
        /*if (m_lastAngle == info.angle() || safeCounter > 12)    
            rotate = false;
        else
        {
            m_lastAngle = m_lastAngle + angleDec;
            rotateBristles(m_lastAngle);
            safeCounter++;
        }*/
//     }
}


void Brush::paintLine(KisPaintDeviceSP dev,const KisPaintInformation &pi1, const KisPaintInformation &pi2){
/* 
    Q_CHECK_PTR(m_accessor);
        dbgPlugins << "Accessor passed...";
    Q_CHECK_PTR(m_dev);
        dbgPlugins << "PaintDevice passed...";
*/


    m_counter++;

    double dx = pi2.pos().x() - pi1.pos().x();
    double dy = pi2.pos().y() - pi1.pos().y();

    double x1 = pi1.pos().x();
    double y1 = pi1.pos().y();

    double x2 = pi2.pos().x();
    double y2 = pi2.pos().y();

    double angle = atan2(dy, dx);
    //dbgPlugins << "angle: " << angle;

/*    double slope = 0.0;
    if (dx != 0){
        slope = dy / dx;
    } */
//     dbgPlugins << "slope: " << slope;

    double distance = sqrt(dx*dx + dy*dy);

    double pressure = pi2.pressure();
    if ( m_mousePressureEnabled && pi1.pressure() == 0.5) // it is mouse
    {
        pressure = 1.0 - computeMousePressure(distance);
    } else // leave it as it is
    {
        pressure = pi1.pressure();
    }
    //dbgPlugins << "pressure: " << pressure << endl;

    Bristle *bristle = 0;
    KoColor brColor;

    KisRandomAccessor accessor = dev->createRandomAccessor( (int)x1, (int)y1 );
    m_pixelSize = dev->colorSpace()->pixelSize();
    m_accessor = &accessor;
    m_dev = dev;

    double inkDeplation;

    QHash<QString, QVariant> params;
    params["h"] = 0.0;
    params["s"] = 0.0;
    params["v"] = 0.0;

    QString saturation("s");

    KoColorTransformation* transfo;
    transfo = m_dev->colorSpace()->createColorTransformation ( "hsv_adjustment", params );

    rotateBristles(angle+1.57);
    int ix1, iy1, ix2, iy2;

    srand48(time(0));
    int size = m_bristles.size();
    Trajectory trajectory; // used for interpolation the path of bristles
    QVector<QPointF> bristlePath; // path for single bristle
    for ( int i=0;i<size;i++ )
    {
/*            if (m_bristles[i].distanceCenter() > m_radius || drand48() <0.5){
                continue;
            }*/
            bristle = &m_bristles[i];

            double fx1, fy1, fx2, fy2;

            double rndFactor = m_randomFactor;
            double scaleFactor = m_scaleFactor;
            double shearFactor = m_shearFactor;

            double randomX = drand48();
            double randomY = drand48();
            randomX -= 0.5;
            randomY -= 0.5;
            randomX *= rndFactor;
            randomY *= rndFactor;

            double scale = pressure*scaleFactor;
            double shear = pressure*shearFactor;

            m_transform.reset();
            m_transform.scale(scale,scale);
            m_transform.translate(randomX, randomY);

            m_transform.shear(shear, shear);

            // transform start dab
            m_transform.map(bristle->x(),bristle->y(),&fx1, &fy1);
            // transform end dab
            m_transform.map(bristle->x(),bristle->y(),&fx2, &fy2);

            // all coords relative to device position
            fx1 += x1;
            fy1 += y1;

            fx2 += x2;
            fy2 += y2;

/*            fx1 =  ( randomFactor + x1 + bristle->x()* scale );
            fy1 =  ( randomFactor + y1 + bristle->y()* scale );

            fx2 =  ( randomFactor + x2 + bristle->x()* scale );
            fy2 =  ( randomFactor + y2 + bristle->y()* scale );*/

            ix1 = (int)fx1;
            iy1 = (int)fy1;
            ix2 = (int)fx2;
            iy2 = (int)fy2;

            // paint between first and last dab
            //lines.drawLine(m_dev, ix1, iy1, ix2, iy2, brColor);

            bristlePath = trajectory.getLinearTrajectory( QPointF(fx1,fy1),QPointF(fx2,fy2), 1.0);

            brColor = bristle->color();
            int bristleCounter = 0;
            int brpathSize = bristlePath.size();
            int inkDepletionSize = m_inkDepletion.size();

            for (int i = 0;i < brpathSize ; i++)
            {
                bristleCounter = bristle->increment();
                if ( bristleCounter >= inkDepletionSize-1){
                    inkDeplation = m_inkDepletion[inkDepletionSize - 1];
                }else{
                    inkDeplation = m_inkDepletion[bristleCounter];
                }

                // saturation
                params[saturation] = ( 
                pressure* 
                bristle->length()* 
                bristle->inkAmount()* 
                (1.0 - inkDeplation))-1.0; 

                transfo->setParameters(params);
                transfo->transform ( m_inkColor.data(), brColor.data() , 1 );

                // opacity
                double opacity =
                255.0*    
                pressure* 
                bristle->length()* 
                bristle->inkAmount()*
                (1.0 - inkDeplation); 

                brColor.setOpacity ( static_cast<int>(opacity) );
                //dbgPlugins << "opacity: "<< brColor.opacity();

                QPointF *bristlePos = &bristlePath[i];
//                 putBristle(bristle, bristlePos->x(), bristlePos->y(), brColor);
                addBristleInk(bristle, bristlePos->x(), bristlePos->y(), brColor);
                bristle->setInkAmount ( 1.0 - inkDeplation );
            }
            //dbgPlugins << "path size" << path.size();

            // paint start bristle
            //putBristle(ix1, iy1, brColor);
            // paint end bristle
            //putBristle(ix2,iy2, brColor);
            // set ink amount for next paint
            
        }
        rotateBristles(-(angle+1.57));

//         repositionBristles(angle,slope);

    m_dev = 0;
    m_accessor = 0;
}


void Brush::rotateBristles(double angle){
    qreal tx, ty, x, y;

    m_transform.reset();
    m_transform.rotateRadians ( angle );
    
    for ( int i=0;i<m_bristles.size();i++ )
    {
        x = m_bristles[i].x();
        y = m_bristles[i].y();
        m_transform.map ( x,y,&tx, &ty );
    //      tx = cos(angle)*x - sin(angle)*y;
    //      ty = sin(angle)*x + cos(angle)*y;
        m_bristles[i].setX ( tx );
        m_bristles[i].setY ( ty );
    }
    m_lastAngle = angle;
}

void Brush::repositionBristles(double angle, double slope){
    // setX
    srand48((int)slope);
    for ( int i=0;i<m_bristles.size();i++ )
    {
        float x = m_bristles[i].x();
        m_bristles[i].setX(x + drand48() );
    }

    // setY
    srand48((int)angle);
    for ( int i=0;i<m_bristles.size();i++ )
    {
        float y = m_bristles[i].y();
        m_bristles[i].setY(y + drand48() );
    }
}

Brush::~Brush(){
/*    if (!m_accessor){ 
        delete m_accessor;
    }*/
}


void Brush::addBristleInk(Bristle *bristle, float wx, float wy, const KoColor &color){
    KoMixColorsOp * mixOp = m_dev->colorSpace()->mixColorsOp();
    m_accessor->moveTo((int)wx,   (int)wy);
    const quint8 *colors[2];
    colors[0] = color.data();
    colors[1] = m_accessor->rawData();

    qint16 colorWeights[2];

    colorWeights[0] = static_cast<quint8>(color.opacity());
    colorWeights[1] = static_cast<quint8>(255-color.opacity());
    mixOp->mixColors(colors, colorWeights, 2, m_accessor->rawData() );

    //memcpy ( m_accessor->rawData(), c.data(), m_pixelSize );
    // bristle delivered some ink
    bristle->upIncrement();

}

void Brush::putBristle(Bristle *bristle, float wx, float wy, const KoColor &color)
{
    m_accessor->moveTo((int)wx,   (int)wy);
    memcpy ( m_accessor->rawData(), color.data(), m_pixelSize );
    // bristle delivered some ink
    bristle->upIncrement();

// Wu particles..useless, the result is not better        
//     int x = int(wx);
//     int y = int(wy);
//     float fx = wx - x;
//     float fy = wy - y;
// 
//     float MAX_OPACITY = mycolor.opacity();
// 
//     int btl = (1-fx) * (1-fy) * MAX_OPACITY;
//     int btr =  (fx)  * (1-fy) * MAX_OPACITY;
//     int bbl = (1-fx) *  (fy)  * MAX_OPACITY;
//     int bbr =  (fx)  *  (fy)  * MAX_OPACITY;
// 
//     const int MIN_OPACITY = 10;
// 
//     if (btl>MIN_OPACITY)
//     {
//         m_accessor->moveTo(x,   y);
//         mycolor.setOpacity(btl);
//         memcpy ( m_accessor->rawData(), mycolor.data(), m_pixelSize );
//     }
// 
//     if (btr>MIN_OPACITY){
//         m_accessor->moveTo(x+1, y);
//         mycolor.setOpacity(btr);
//         memcpy ( m_accessor->rawData(), mycolor.data(), m_pixelSize );
//     }
// 
//     if (bbl>MIN_OPACITY){
//         m_accessor->moveTo(x, y+1);
//         mycolor.setOpacity(bbl);
//         memcpy ( m_accessor->rawData(), mycolor.data(), m_pixelSize );
//     }
// 
//     if (bbr>MIN_OPACITY){
//         m_accessor->moveTo(x+1, y+1);
//         mycolor.setOpacity(bbr);
//         memcpy ( m_accessor->rawData(), mycolor.data(), m_pixelSize );
//     }
}

void Brush::addStrokeSample(StrokeSample sample){
    Q_UNUSED(sample);
}
void Brush::addStrokeSample(float x,float y,float pressure,float tiltX, float tiltY,float rotation){
    StrokeSample sample(x,y,pressure,tiltX, tiltY, rotation);
    m_stroke.append(sample);
}

void Brush::setRadius(int radius){
    m_radius = radius;
}

void Brush::setSigma(double sigma){
    m_sigma = sigma;
}

double Brush::computeMousePressure(double distance)
{
    double scale = 20.0;
    double minPressure = 0.02;
    double oldPressure = m_oldPressure;
    
    double factor = 1.0 - distance/scale;
    if (factor < 0.0) factor = 0.0;

    double result = ((4.0*oldPressure) + minPressure + factor)/5.0;
    m_oldPressure = result;
    return result;
}

