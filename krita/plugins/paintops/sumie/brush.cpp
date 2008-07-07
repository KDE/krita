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
#include "kis_random_accessor.h"
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


void Brush::paint(KisPaintDeviceSP dev, float x, float y,const KoColor &color){

	m_counter++;
	float decr = ((m_counter*m_counter*m_counter)+(m_counter*m_counter)+5)/100000.0f;

	Bristle *bristle;
	float max = 0.0f;
	int pos = 0;

	qint32 pixelSize = dev->colorSpace()->pixelSize();
	KisRandomAccessor accessor = dev->createRandomAccessor((int)x, (int)y);
	KoColor mycolor(color);

	double u;
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

		mycolor.setOpacity(len);
		accessor.moveTo(dx,dy);
		memcpy(accessor.rawData(), mycolor.data(), pixelSize);
		//dev->setPixel(x+bristle.x(), y+bristle.y(), mycolor);

		
		bristle->setInkAmount(1.0f-decr);
	}

	//dbgPlugins << "Maximum at: " << pos << " value: "  << max << endl;
	dbgPlugins << "decr: " << decr << " counter: " << m_counter << endl;
}

Brush::~Brush(){

}
