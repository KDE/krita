/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <math.h>

#include <qwmatrix.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_rotate_visitor.h"
#include "kis_progress_display_interface.h"

void KisRotateVisitor::rotate(double angle, KisProgressDisplayInterface *m_progress) 
{
        const double pi=3.1415926535897932385;
        kdDebug() << "Rotating Code called! Going to rotate image by (angle): " << angle << "\n";
        double theta=angle*pi/180;
        //first perform a shear along the x-axis by tan(theta/2)
        double shearX=tan(theta/2);
        xShearImage(shearX, m_progress);
        //next perform a shear along the y-axis by sin(theta)
        double shearY=sin(theta);
        yShearImage(shearY, m_progress);
        //again perform a shear along the x-axis by tan(theta/2)
        xShearImage(shearX, m_progress);
}

void KisRotateVisitor::shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress) 
{
        kdDebug() << "Shear Code called! Going to shear image by xAngle " << angleX << " and yAngle " << angleY << "\n";
        const double pi=3.1415926535897932385;
        
        if(angleX != 0 && angleY == 0){
                double theta=angleX*pi/180;
                double shearX=tan(theta);
                xShearImage(shearX, m_progress);
        }
        if(angleX == 0 && angleY != 0){
                double theta=angleY*pi/180;
                double shearY=tan(theta);
                yShearImage(shearY, m_progress);
        }
}

void KisRotateVisitor::xShearImage(double shearX, KisProgressDisplayInterface *m_progress)
{
        kdDebug() << "xShearImage called, shear parameter " << shearX << "\n";        
        Q_INT32 width = m_dev->width();
        Q_INT32 height = m_dev->height();
        
        //calculate widht of the sheared image
        Q_INT32 targetW = width + QABS(height*shearX);
        Q_INT32 targetH = height;
        KisTileMgrSP tm = new KisTileMgr(m_dev -> colorStrategy() -> depth(), targetW, targetH);
        QUANTUM * newData = new QUANTUM[targetW * targetH * m_dev -> depth() * sizeof(QUANTUM)];
        QUANTUM *tempRow = new QUANTUM[width * m_dev -> depth() * sizeof(QUANTUM)];
        QUANTUM *pixel = new QUANTUM[m_dev -> depth() * sizeof(QUANTUM)];
        QUANTUM *left = new QUANTUM[m_dev -> depth() * sizeof(QUANTUM)];
        QUANTUM *oleft = new QUANTUM[m_dev -> depth() * sizeof(QUANTUM)];
        double displacement;
        Q_INT32 displacementInt;
        double weight;
        Q_INT32 currentPos;
        
        if(shearX>=0){
                for (Q_INT32 y=0; y < height; y++){
                        //calculate displacement
                        displacement = (height-y)*shearX;
                        displacementInt = floor(displacement);
                        weight=displacement-displacementInt;
                        //read a row from the image
                        m_dev -> tiles() -> readPixelData(0, y, width-1, y, tempRow, m_dev -> depth());
                        //initialize oleft
                        for(int channel = 0; channel < m_dev -> depth(); channel++)
                                oleft[channel]=left[channel]=0;
                        //copy the pixels to the newData array
                        for(Q_INT32 x=0; x < width; x++){
                                currentPos = (y*targetW+x+displacementInt) * m_dev -> depth(); // try to be at least a little efficient
                                for(int channel = 0; channel < m_dev -> depth(); channel++){
                                        pixel[channel]=tempRow[x*m_dev -> depth()+channel];
                                        left[channel]=weight*pixel[channel];
                                        pixel[channel]=pixel[channel]-left[channel]+oleft[channel];
                                        newData[currentPos  + channel]=pixel[channel];
                                        oleft[channel]=left[channel];
                                }
                        }
                }        
        } else {
                for (Q_INT32 y=0; y < height; y++){
                        //calculate displacement
                        displacement = y*QABS(shearX);
                        displacementInt = floor(displacement);
                        weight=displacement-displacementInt;
                        //read a row from the image
                        m_dev -> tiles() -> readPixelData(0, y, width-1, y, tempRow, m_dev -> depth());
                        //initialize oleft
                        for(int channel = 0; channel < m_dev -> depth(); channel++)
                                oleft[channel]=left[channel]=0;
                        //copy the pixels to the newData array
                        for(Q_INT32 x=0; x < width; x++){
                                currentPos = (y*targetW+x+displacementInt) * m_dev -> depth(); // try to be at least a little efficient
                                for(int channel = 0; channel < m_dev -> depth(); channel++){
                                        pixel[channel]=tempRow[x*m_dev -> depth()+channel];
                                        left[channel]=weight*pixel[channel];
                                        pixel[channel]=pixel[channel]-left[channel]+oleft[channel];
                                        newData[currentPos  + channel]=pixel[channel];
                                        oleft[channel]=left[channel];
                                }
                        }
                }          
        }        
        //now write newData to the image
        kdDebug() << "write newData to the image!" << "\n";
        tm -> writePixelData(0, 0, targetW - 1, targetH - 1, newData, targetW * m_dev -> depth());
        m_dev -> setTiles(tm); // Also sets width and height correctly
}

void KisRotateVisitor::yShearImage(double shearY, KisProgressDisplayInterface *m_progress)
{
        kdDebug() << "YShearImage called, shear paramter " << shearY << "\n";

        const double pi=3.1415926535897932385;
        
        Q_INT32 width = m_dev->width();
        Q_INT32 height = m_dev->height();
        
        //calculate widht of the sheared image
        Q_INT32 targetW = width;
        Q_INT32 targetH = height + QABS(width*shearY); ;
        KisTileMgrSP tm = new KisTileMgr(m_dev -> colorStrategy() -> depth(), targetW, targetH);
        QUANTUM * newData = new QUANTUM[targetW * targetH * m_dev -> depth() * sizeof(QUANTUM)];
        
        //shear the image
        QUANTUM *tempCol = new QUANTUM[height * m_dev -> depth() * sizeof(QUANTUM)];
        QUANTUM *pixel = new QUANTUM[m_dev -> depth() * sizeof(QUANTUM)];
        QUANTUM *left = new QUANTUM[m_dev -> depth() * sizeof(QUANTUM)];
        QUANTUM *oleft = new QUANTUM[m_dev -> depth() * sizeof(QUANTUM)];
        double displacement;
        Q_INT32 displacementInt;
        double weight;
        Q_INT32 currentPos;
        
        if(shearY>=0){
                for (Q_INT32 x=0; x < width; x++){
                        //calculate displacement
                        displacement = x*shearY;
                        displacementInt = floor(displacement);
                        weight=displacement-displacementInt;
                        //read a column from the image
                        m_dev -> tiles() -> readPixelData(x, 0, x, height - 1, tempCol, m_dev -> depth());
                        //initialize oleft
                        for(int channel = 0; channel < m_dev -> depth(); channel++)
                                oleft[channel]=left[channel]=0;
                        //copy the pixels to the newData array
                        for(Q_INT32 y=0; y < height; y++){
                                currentPos = ((y+displacementInt)*targetW+x) * m_dev -> depth(); // try to be at least a little efficient
                                for(int channel = 0; channel < m_dev -> depth(); channel++){
                                        pixel[channel]=tempCol[y*m_dev -> depth()+channel];
                                        left[channel]=weight*pixel[channel];
                                        pixel[channel]=pixel[channel]-left[channel]+oleft[channel];
                                        newData[currentPos  + channel]=pixel[channel];
                                        oleft[channel]=left[channel];
                                }
                        }
                }        
        } else {
                for (Q_INT32 x=0; x < width; x++){
                        //calculate displacement
                        displacement = (width-x)*QABS(shearY);
                        kdDebug() << "displacement " << displacement << "\n";
                        displacementInt = floor(displacement);
                        weight=displacement-displacementInt;
                        //read a column from the image
                        m_dev -> tiles() -> readPixelData(x, 0, x, height - 1, tempCol, m_dev -> depth());
                        //initialize oleft
                        for(int channel = 0; channel < m_dev -> depth(); channel++)
                                oleft[channel]=left[channel]=0;
                        //copy the pixels to the newData array
                        for(Q_INT32 y=0; y < height; y++){
                                currentPos = ((y+displacementInt)*targetW+x) * m_dev -> depth(); // try to be at least a little efficient
                                for(int channel = 0; channel < m_dev -> depth(); channel++){
                                        pixel[channel]=tempCol[y*m_dev -> depth()+channel];
                                        left[channel]=weight*pixel[channel];
                                        pixel[channel]=pixel[channel]-left[channel]+oleft[channel];
                                        newData[currentPos  + channel]=pixel[channel];
                                        oleft[channel]=left[channel];
                                }
                        }
                }        
        }
        //now write newData to the image
        kdDebug() << "write newData to the image!" << "\n";
        tm -> writePixelData(0, 0, targetW - 1, targetH - 1, newData, targetW * m_dev -> depth());
        m_dev -> setTiles(tm); // Also sets width and height correctly
}