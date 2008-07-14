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


Brush::Brush(const BrushShape &initialShape, KoColor inkColor){
	m_initialShape = initialShape;
	m_inkColor = inkColor;	
	m_counter = 0;
}

Brush::Brush(){
	m_radius = 5;
	m_counter = 0;

	BrushShape bs;
	bs.fromGaussian( m_radius, 1.0f, 0.9f );
	m_bristles = bs.getBristles();
	
	srand48(1045);
}

void Brush::setInkDepletion(QList<float> *curveData){
	int count = curveData->size();

	for (int i = 0; i< count ;i++)
	{
		m_inkDepletion.append( curveData->at(i) );
	}

	dbgKrita << "size: " << curveData->size() ;
	dbgKrita << "size depletion: " << m_inkDepletion.size() << endl;
	delete curveData;
	 // thank you
}

void Brush::paint(KisPaintDeviceSP dev, float x, float y,const KoColor &color){
	/*if (m_counter == 1){
		m_inkColor = color;
	}*/

	m_inkColor = color;

	m_counter++;
	
	float decr = (m_counter*m_counter*m_counter)/1000000.0f;

	Bristle *bristle;
	float max = 0.0f;
	int pos = 0;

	qint32 pixelSize = dev->colorSpace()->pixelSize();
	KisRandomAccessor accessor = dev->createRandomAccessor((int)x, (int)y);
	

	double u;

	QHash<QString, QVariant> params;

	double result;
	if ( m_counter >= m_inkDepletion.size()-1 ){
		result = m_inkDepletion[m_inkDepletion.size() - 1];
	}else{
		result = m_inkDepletion[m_counter];
	}


	dbgPlugins << "result:" << result << endl;
	dbgPlugins << "m_counter:" << m_counter << endl;
		
		params["h"] = 0.0;
		params["s"] = -result;
		params["v"] = 0.0;
		

		KoColorTransformation* transfo = dev->colorSpace()->createColorTransformation( "hsv_adjustment", params);
		transfo->transform( m_inkColor.data(), m_inkColor.data(), 1);
		
		/*int opacity = (1.0f+result)*255;
		dbgPlugins << "opacity: "<< opacity << endl;
		m_inkColor.setOpacity(opacity);*/
	

	for (int i=0;i<m_bristles.size();i++){
		bristle = &m_bristles[i];

		u = drand48();
		if (bristle->distanceCenter() > m_radius || u <0.5){
			continue;
		}

		//int len = (int)(bristle->length() * 1295 * bristle->amount() );
		int len = (int)(bristle->amount() * 255 );

		if (bristle->length() > max){
			max = bristle->length();
			pos = i;
		}
		if (len > 255){
			len = 255;
		}else 
		if (len < 0){
			len = 0;
		}
		
		int dx, dy;
		dx = (int)(x+bristle->x());
		dy = (int)(y+bristle->y());


		accessor.moveTo(dx,dy);
		memcpy(accessor.rawData(), m_inkColor.data(), pixelSize);
		//dev->setPixel(x+bristle.x(), y+bristle.y(), m_inkColor);

		
		bristle->setInkAmount(1.0f-decr);
	}

	//dbgPlugins << "Maximum at: " << pos << " value: "  << max << endl;
	//dbgPlugins << "decr: " << decr << " counter: " << m_counter << endl;
	
}

Brush::~Brush(){
}
