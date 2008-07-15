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

#include "brush_shape.h"
#include <QVector>
#include <cmath>
#include "kis_debug.h"

#include <iostream>
using namespace std;

const float PI = 3.141592f;

BrushShape::BrushShape(){

}

BrushShape::~BrushShape(){

}


void BrushShape::fromGaussian(int radius, float sigma){
	    m_width = m_height = radius * 2 + 1;
        int gaussLength = (int)(m_width*m_width);
		//int center = (edgeSize - 1) / 2;
		
		float sigmaSquare = - 2.0 * sigma * sigma;
		float sigmaConst = 1.0 / (2.0 * PI * sigma * sigma);
	
		float total = 0;
		float length = 0;
		int p = 0;

				

		for (int y=-radius;y<=radius;y++){
			for (int x=-radius;x<=radius;x++){
				length = (exp( (x*x + y*y) / sigmaSquare ) * sigmaConst);
				total += length;
				Bristle b(x,y,length);
				b.setInkAmount(1.0f);
				m_bristles.append(b);
				p++;
			}
		}

	// dbgKrita << "total: " << total <<  " " << p << endl << flush;
	float minLen = m_bristles[0].length();
	float maxLen = m_bristles[gaussLength/2].length();
	float dist = maxLen - minLen;
	
	// normalise lengths 
	float result;
	int i = 0;

	for (int x=0;x<m_width;x++)
	{
		for (int y=0;y<m_height;y++,i++)
		{
		result = (m_bristles[i].length() - minLen ) / dist;
		m_bristles[i].setLength(result);
		}
	}

}

QVector<Bristle> BrushShape::getBristles(){
	return m_bristles;
}

int BrushShape::width(){
	return m_width;
}

int BrushShape::height(){
	return m_height;
}
