/***************************************************************************
 *   Copyright (c) 2007 by Zoran Mesec ( zoran.mesec@gmail.com )           *
 *   Copyright (c) 2007 Cyrille Berger ( cberger@cberger.net )             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <string>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <kis_types.h>
#include <QRect>


class QRect;

using namespace std;

class APImage
 {

public:
    APImage(KisPaintDeviceSP device, const QRect& area);
    int getWidth();
    int getWidthBW();
    int getHeight();
    int getHeightBW();
    int getPixel(int x, int y);
    int getIntegralPixel(int x,int y);
    void drawCircle(int x,int y, int radius);
    void drawLine(int x1,int y1, int x2,int y2);
    void drawRectangle(int x,int y, int radius);
    void integrate();
    int getRegionSum(int x1, int y1, int x2, int y2);
    void scale(double factor);

private:
    int xToDevice(int x) const;
    int yToDevice(int y) const;
 	private:
 	/**
 	* Holds the convolution of the image.
 	*/
 	vector<vector<int> > convolution;
    /**
 	* Holds the values of the integral image.
 	*/
 	vector<vector<int> > integral;
 	int _getValue4Integral(int x, int y);
    KisPaintDeviceSP m_device;
    QRect m_area;
    KisRandomAccessor* m_randomAccessor;
 };


#endif // IMAGE_H_INCLUDED
