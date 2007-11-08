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
#include <string>
#include <math.h>
#include "APImage.h"

#include <KoColorSpaceRegistry.h>

#include <kis_paint_device.h>
#include <kis_random_accessor.h>

APImage::APImage(KisPaintDeviceSP _device, const QRect& _area) : m_area(_area)
{
    KoColorSpace* graycs = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
    m_device = new KisPaintDevice(*_device);
    m_device->convertTo(graycs);
    m_randomAccessor = new KisRandomAccessor( m_device->createRandomAccessor(0,0) );
}

int APImage::xToDevice(int x) const
{
    Q_ASSERT(x >= 0 and x < m_area.width());
    return x + m_area.x();
}

int APImage::yToDevice(int y) const
{
    Q_ASSERT(y >= 0 and y < m_area.height());
    return y + m_area.y();
}

int APImage::getPixel(int y,int x) {
    m_randomAccessor->moveTo( xToDevice(x), yToDevice(y) );
    return *(m_randomAccessor->rawData());
}
int APImage::getWidth() {
	return m_area.width();
}
int APImage::getWidthBW() {
	return m_area.width();
}
int APImage::getHeight() {
	return m_area.height();
}
int APImage::getHeightBW() {
	return m_area.height();
}
void APImage::scale(double factor) {
}

void APImage::drawCircle(int x,int y, int radius) {
}
void APImage::drawRectangle(int x,int y, int radius) {
}

void APImage::drawLine(int x1,int y1, int x2,int y2) {

}

/**
 * Calculates the integral image
 */
void APImage::integrate() {

    cout << "Height:"<< this->getHeightBW() <<"\n";
    cout << "Width:"<< this->getWidthBW() <<"\n";

    this->integral.clear();
    this->integral.resize(this->getHeightBW());
    for(int i=0;i<this->getHeightBW();i++) {
        this->integral[i].resize(this->getWidthBW());
        for(int j=0; j<this->getWidthBW();j++) {
            //cout << i << ","<< j<<"\n";
            this->integral[i][j]=this->_getValue4Integral(i,j-1)+this->_getValue4Integral(i-1,j)+this->getPixel(i,j)-this->_getValue4Integral(i-1,j-1);
        }
        //cout << "\n";
    }
}
int APImage::_getValue4Integral(int x, int y) {
if(x==-1 || y==-1) return 0;
else return this->integral[x][y];
}
int APImage::getIntegralPixel(int x,int y) {
 return this->integral[x][y];
}
int APImage::getRegionSum(int x1, int y1, int x2, int y2) {
    if(x1<=0) x1=1;
    if(y1<=0) y1=1;
    if(x2<=0) x2=1;
    if(y2<=0) y2=1;
    if(x1>=this->getHeightBW()) x1=this->getHeightBW()-1;
    if(x2>=this->getHeightBW()) x2=this->getHeightBW()-1;
    if(y1>=this->getWidthBW()) y1=this->getWidthBW()-1;
    if(y2>=this->getWidthBW()) y2=this->getWidthBW()-1;
 return this->integral[x2][y2]+this->integral[x1-1][y1-1]-this->integral[x1-1][y2]-this->integral[x2][y1-1];
}
