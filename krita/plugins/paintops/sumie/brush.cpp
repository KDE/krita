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

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>
#include <QList>

#include "kis_random_accessor.h"
#include "widgets/kcurve.h"

#include <cmath>


const float radToDeg = 57.29578;

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
	m_mousePressure = enable;
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
/*	m_counter++;
	
	Bristle *bristle;
	KoColor brColor;
	int x = info.pos().x();
	int y = info.pos().y();

	qint32 pixelSize = dev->colorSpace()->pixelSize();
	KisRandomAccessor accessor = dev->createRandomAccessor( x,y );
	
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

// 	dbgPlugins << m_counter ;
	
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
/*	while ( rotate )
	{
		for ( int i=0;i<m_bristles.size();i++ )
		{
// 			if (m_bristles[i].distanceCenter() > m_radius || drand48() <0.5){
// 				continue;
// 			}
			bristle = &m_bristles[i];
			brColor = bristle->color();
	
			// saturation
			params["s"] = - ( 1.0 - ( info.pressure() * bristle->length() * bristle->inkAmount() ) );
			transfo = dev->colorSpace()->createColorTransformation ( "hsv_adjustment", params );
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
// 	}
}


void Brush::paintLine(KisPaintDeviceSP dev,const KisPaintInformation &pi1, const KisPaintInformation &pi2){
	m_counter++;

	double dx = pi2.pos().x() - pi1.pos().x();
	double dy = pi2.pos().y() - pi1.pos().y();

	double x1 = pi1.pos().x();
	double y1 = pi1.pos().y();

	double x2 = pi2.pos().x();
	double y2 = pi2.pos().y();

	double angle = atan2(dy, dx);
	//dbgPlugins << "angle: " << angle;

	double slope = 0.0;
	if (dx != 0){
		slope = dy / dx;
	} 
// 	dbgPlugins << "slope: " << slope;

	double distance = sqrt(dx*dx + dy*dy);

	double pressure = pi2.pressure();
	if ( m_mousePressure && pi1.pressure() == 0.5) // it is mouse
	{
		pressure = 1.0 - computeMousePressure(distance);
	} else // leave it as it is
	{
		pressure = pi1.pressure();
	}
	dbgPlugins << "pressure: " << pressure << endl;

	Bristle *bristle = 0;
	KoColor brColor;

	KisRandomAccessor accessor = dev->createRandomAccessor( (int)x1, (int)y1 );
	qint32 pixelSize = dev->colorSpace()->pixelSize();

	double inkDeplation;
	if ( m_counter >= m_inkDepletion.size()-1 ){
		inkDeplation = m_inkDepletion[m_inkDepletion.size() - 1];
	}else{
		inkDeplation = m_inkDepletion[m_counter];
	}
// 	dbgPlugins << "inkDeplation: " << inkDeplation;

	QHash<QString, QVariant> params;
	params["h"] = 0.0;
	params["s"] = 0.0;
	params["v"] = 0.0;
	KoColorTransformation* transfo;


	/*BrushShape bs;
	bs.fromLine(m_initialShape.radius()+(int)(pressure+0.5) , m_initialShape.sigma() );
	setBrushShape(bs);*/
		rotateBristles(angle+1.57);
		int ix1, iy1, ix2, iy2;
		Lines lines;
		for ( int i=0;i<m_bristles.size();i++ )
		{
			/*if (m_bristles[i].distanceCenter() > m_radius || drand48() <0.5){
				continue;
			}*/
			bristle = &m_bristles[i];
			brColor = bristle->color();
	
			// saturation
			params["s"] = ( 
			pressure* 
			bristle->length()* 
			bristle->inkAmount()* 
			(1.0 - inkDeplation))-1.0; 
// 			dbgPlugins << "saturation: " << params["s"];

			transfo = dev->colorSpace()->createColorTransformation ( "hsv_adjustment", params );
			transfo->transform ( m_inkColor.data(), brColor.data() , 1 );
	
			// opacity
			double opacity =
			255.0*	
 			pressure* 
			bristle->length()* 
			bristle->inkAmount()*
			(1.0 - inkDeplation); 

// 			dbgPlugins << "opacity: " << opacity;
			brColor.setOpacity ( static_cast<int>(opacity) );

			// paint start bristle			
			float fx1, fy1, fx2, fy2;
			
			srand48((int)(angle*57));
			double randomFactor = drand48();
			double scaleFactor = 2.0;
			double scale = pressure*scaleFactor;

			fx1 = ix1 = ( int ) ( randomFactor + x1 + bristle->x()* scale );
			fy1 = iy1 = ( int ) ( randomFactor + y1 + bristle->y()* scale );

			fx2 = ix2 = ( int ) ( randomFactor + x2 + bristle->x()* scale );
			fy2 = iy2 = ( int ) ( randomFactor + y2 + bristle->y()* scale );

			lines.drawDDALine(dev, ix1, iy1, ix2, iy2, brColor);

			accessor.moveTo ( ix1,iy1 );
			memcpy ( accessor.rawData(), brColor.data(), pixelSize );
			// paint end bristle
			accessor.moveTo ( ix2,iy2 );
			memcpy ( accessor.rawData(), brColor.data(), pixelSize );

			bristle->setInkAmount ( 1.0 - inkDeplation );
		}
		rotateBristles(-(angle+1.57));

// 		repositionBristles(angle,slope);
}

inline double modulo(double x, double r)
{
            return x-floor(x/r)*r;
}

double Brush::getAngleDelta(const KisPaintInformation& info)
{
    double angle = info.angle();
    double v = modulo(m_angle - angle + M_PI, 2.0 * M_PI) - M_PI;
    if(v < 0)
    {
        m_angle += 0.01;
    } else if( v > 0)
    {
        m_angle -= 0.01;
    }
    m_angle = modulo(m_angle, 2.0 * M_PI);
    return m_angle;
}


void Brush::rotateBristles(double angle){
	float rx, ry, x, y;
	for ( int i=0;i<m_bristles.size();i++ )
	{
		x = m_bristles[i].x();
		y = m_bristles[i].y();

		rx = cos(angle)*x - sin(angle)*y;
		ry = sin(angle)*x + cos(angle)*y;

		m_bristles[i].setX(rx);
		m_bristles[i].setY(ry);
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

