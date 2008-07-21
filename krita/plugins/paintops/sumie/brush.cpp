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
	srand48(1045);
	m_radius = 20;
	m_sigma = 20.f;	

	m_counter = 0;
	m_lastAngle = 0.0;

	BrushShape bs;
	//bs.fromGaussian( m_radius, m_sigma );
	bs.fromLine( m_radius, m_sigma );
	
	setBrushShape(bs);	
}

void Brush::setBrushShape(BrushShape brushShape){
	m_bristles = brushShape.getBristles();
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
	m_counter++;
	
	Bristle *bristle;
	KoColor brColor;
	int x = info.pos().x();
	int y = info.pos().y();

	qint32 pixelSize = dev->colorSpace()->pixelSize();
	KisRandomAccessor accessor = dev->createRandomAccessor( x,y );
	
	double result;
	if ( m_counter >= m_inkDepletion.size()-1 ){
		result = m_inkDepletion[m_inkDepletion.size() - 1];
	}else{
		result = m_inkDepletion[m_counter];
	}

	QHash<QString, QVariant> params;
	params["h"] = 0.0;
	params["s"] = 0.0;
	params["v"] = 0.0;
	KoColorTransformation* transfo;


	int dx, dy; // used for counting the coords of bristles relative to the center of the brush
	bool rotate = true;

	float angleDistance = m_lastAngle - info.angle();
	float angleDec;

	int safeCounter = 0;

	if (angleDistance < 0){
		angleDec = +0.1;
	} else
	{
		angleDec = -0.1;
	}

	/*
	* MAIN LOOP *HIGHLY* careful
	*/
	while ( rotate )
	{
		for ( int i=0;i<m_bristles.size();i++ )
		{
			/*if (m_bristles[i].distanceCenter() > m_radius || drand48() <0.5){
				continue;
			}*/
			bristle = &m_bristles[i];
			brColor = bristle->color();
	
			// saturation
			params["s"] = - ( 1.0 - ( bristle->length() * bristle->inkAmount() ) );
			transfo = dev->colorSpace()->createColorTransformation ( "hsv_adjustment", params );
			transfo->transform ( m_inkColor.data(), brColor.data() , 1 );
	
			// opacity
			brColor.setOpacity ( static_cast<int> ( 255.0*bristle->length() * bristle->inkAmount() * ( 1.0 - result ) ) );
	
			dx = ( int ) ( x+bristle->x() );
			dy = ( int ) ( y+bristle->y() );
	
			accessor.moveTo ( dx,dy );
			memcpy ( accessor.rawData(), brColor.data(), pixelSize );
	
			bristle->setInkAmount ( 1.0 - result );
			//bristle->setColor(brColor);
		}
		
		
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
	}
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
	
	if (angle == m_lastAngle){ 
		//dbgPlugins << "returnAngle slope & angle \t[ " << slope <<":" << angle*radToDeg << " ]"<<endl;
		return;
	}
	else
	{
		
/*	if (slope == m_lastSlope){ 
		dbgPlugins << "returnSlope slope & angle \t[ " << slope <<":" << angle*radToDeg << " ]"<<endl;
		return;
	}else{
		m_lastSlope = slope;
	}
	dbgPlugins << "slope & angle \t[ " << slope <<":" << angle*radToDeg << " ]"<<endl;
*/
	double dist = m_lastAngle - angle;
	float incr = dist/12.0;

	} // if-else
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

