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
#include <qapplication.h>
#include <qwmatrix.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_rotate_visitor.h"
#include "kis_progress_display_interface.h"
#include "kis_iterators_pixel.h"

void KisRotateVisitor::rotate(double angle, KisProgressDisplayInterface *m_progress) 
{

        const double pi=3.1415926535897932385;
        kdDebug() << "Rotating Code called! Going to rotate image by (angle): " << angle << "\n";
        
        if(angle>=315 && angle <360){
                angle=angle-360;
        } else if(angle > -360 && angle < -45){
                angle=angle+360;
        }
        
        //progress info
        m_cancelRequested = false;
        m_progress -> setSubject(this, true, true);
        Q_INT32 progressCurrent=0;
        QRect r = m_dev -> extent();
        emit notifyProgressStage(this,i18n("Rotating Image..."),0); 
        if(angle>=-45 && angle <45){
                Q_INT32 origHeight = r.height();
                Q_INT32 origWidth = r.width();
                double theta=angle*pi/180;
                double shearX=tan(theta/2);
                double shearY=sin(theta);
                Q_INT32 newWidth=origWidth + QABS(origHeight * shearX);
                Q_INT32 newHeight=origHeight + QABS(newWidth * shearY);
                Q_INT32 progressTotal=origHeight + newWidth + newHeight;
                //first perform a shear along the x-axis by tan(theta/2)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                //next perform a shear along the y-axis by sin(theta)
                progressCurrent=yShearImage(shearY, m_progress, progressTotal, progressCurrent);
                //again perform a shear along the x-axis by tan(theta/2)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                double deltaX=(origWidth+origHeight*QABS(shearX))*QABS(shearX*shearY);
        
                if (deltaX != 0)
                        xCropImage(deltaX);
                double deltaY=origHeight*QABS(shearX*shearY);
                if (deltaY != 0)
                        yCropImage(deltaY);
        } else if(angle>=45 && angle < 135 && angle != 90){
                rotateRight90();
                Q_INT32 origHeight = r.height();
                Q_INT32 origWidth = r.width();
                double theta=(angle-90)*pi/180;
                double shearX=tan(theta/2);
                double shearY=sin(theta);
                Q_INT32 newWidth=origWidth + QABS(origHeight * shearX);
                Q_INT32 newHeight=origHeight + QABS(newWidth * shearY);
                Q_INT32 progressTotal=origHeight + newWidth + newHeight;
                //first perform a shear along the x-axis by tan(theta/2)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                //next perform a shear along the y-axis by sin(theta)
                progressCurrent=yShearImage(shearY, m_progress, progressTotal, progressCurrent);
                //again perform a shear along the x-axis by tan(theta/2)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                double deltaX=(origWidth+origHeight*QABS(shearX))*QABS(shearX*shearY);
                if (deltaX != 0)
                        xCropImage(deltaX);
                double deltaY=origHeight*QABS(shearX*shearY);
                if (deltaY != 0)
                        yCropImage(deltaY);
        } else if(angle>=135 && angle < 225 && angle != 180){
                rotate180();
                Q_INT32 origHeight = r.height();
                Q_INT32 origWidth = r.width();
                double theta=(angle-180)*pi/180;
                double shearX=tan(theta/2);
                double shearY=sin(theta);
                Q_INT32 newWidth=origWidth + QABS(origHeight * shearX);
                Q_INT32 newHeight=origHeight + QABS(newWidth * shearY);
                Q_INT32 progressTotal=origHeight + newWidth + newHeight;
                //first perform a shear along the x-axis by tan(theta/2)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                //next perform a shear along the y-axis by sin(theta)
                progressCurrent=yShearImage(shearY, m_progress, progressTotal, progressCurrent);
                //again perform a shear along the x-axis by tan(theta/2)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                double deltaX=(origWidth+origHeight*QABS(shearX))*QABS(shearX*shearY);
                if (deltaX != 0)
                        xCropImage(deltaX);
                double deltaY=origHeight*QABS(shearX*shearY);
                if (deltaY != 0)
                        yCropImage(deltaY);
        } else if(angle>=225 && angle < 315 && angle != 270){
                rotateLeft90();
                Q_INT32 origHeight = r.height();
                Q_INT32 origWidth = r.width();
                double theta=(angle-270)*pi/180;
                double shearX=tan(theta/2);
                double shearY=sin(theta);
                Q_INT32 newWidth=origWidth + QABS(origHeight * shearX);
                Q_INT32 newHeight=origHeight + QABS(newWidth * shearY);
                Q_INT32 progressTotal=origHeight + newWidth + newHeight;
                //first perform a shear along the x-axis by tan(theta/2)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                //next perform a shear along the y-axis by sin(theta)
                progressCurrent=yShearImage(shearY, m_progress, progressTotal, progressCurrent);
                //again perform a shear along the x-axis by tan(theta/2)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                double deltaX=(origWidth+origHeight*QABS(shearX))*QABS(shearX*shearY);
                if (deltaX != 0)
                        xCropImage(deltaX);
                double deltaY=origHeight*QABS(shearX*shearY);
                if (deltaY != 0)
                        yCropImage(deltaY);

        } else if(angle==90){      
                rotateRight90();
        } else if (angle==180){
                rotate180();
        } else if (angle==270){
                rotateLeft90();
        }

        emit notifyProgressDone(this);


}

void KisRotateVisitor::shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress) 
{
        kdDebug() << "Shear Code called! Going to shear image by xAngle " << angleX << " and yAngle " << angleY << "\n";
        const double pi=3.1415926535897932385;
        
        //progress info
        m_cancelRequested = false;
        m_progress -> setSubject(this, true, true);
        Q_INT32 progressCurrent=0;
        emit notifyProgressStage(this,i18n("Rotating Image..."),0);
        
        if (angleX != 0 || angleY != 0){
                Q_INT32 origHeight = m_dev -> image() -> height();
                Q_INT32 origWidth = m_dev -> image() -> width();
                double thetaX=angleX*pi/180;
                double shearX=tan(thetaX);
                double thetaY=angleY*pi/180;
                double shearY=tan(thetaY);
                Q_INT32 newWidth=origWidth + QABS(origHeight * shearX);
                Q_INT32 progressTotal=origHeight + newWidth;
                //first perform a shear along the x-axis by tan(thetaX)
                progressCurrent=xShearImage(shearX, m_progress, progressTotal, progressCurrent);
                //next perform a shear along the y-axis by tan(thetaY)
                progressCurrent=yShearImage(shearY, m_progress, progressTotal, progressCurrent);
                double deltaY=origHeight*QABS(shearX*shearY);
                if (deltaY != 0 && thetaX > 0 && thetaY > 0)
                        yCropImage(deltaY);
                else if (deltaY != 0 && thetaX < 0 && thetaY < 0)
                        yCropImage(deltaY);
        }
}

Q_INT32 KisRotateVisitor::xShearImage(double shearX, KisProgressDisplayInterface *m_progress, Q_INT32 progressTotal, Q_INT32 progressCurrent)
{
        kdDebug() << "xShearImage called, shear parameter " << shearX << "\n";        
        Q_INT32 width = m_dev -> image() -> width();
        Q_INT32 height = m_dev -> image() -> height();
        double progressStart=(double)progressCurrent / progressTotal * 100; 
         
        //calculate widht of the sheared image
        Q_INT32 targetW = (Q_INT32)(width + QABS(height*shearX));
        Q_INT32 targetH = height;
        QUANTUM * newData = new QUANTUM[targetW * targetH * m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *tempRow = new QUANTUM[width * m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *pixel = new QUANTUM[m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *left = new QUANTUM[m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *oleft = new QUANTUM[m_dev -> pixelSize() * sizeof(QUANTUM)];
        double displacement;
        Q_INT32 displacementInt;
        double weight;
        Q_INT32 currentPos;
        
       
        if(shearX>=0){
                for (Q_INT32 y=0; y < height; y++){
                        emit notifyProgress(this, y * 100  / progressTotal + progressStart + 1);
                        progressCurrent++;
                        if (m_cancelRequested) {
                                break;
                        }
                        //calculate displacement
                        displacement = (height-y)*shearX;
                        displacementInt = (Q_INT32)(floor(displacement));
                        weight=displacement-displacementInt;
                        //read a row from the image
                        tempRow = m_dev -> readBytes( 0, y, width, 1);
                        //initialize oleft
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++)
                                oleft[channel]=left[channel]=0;
                        //copy the pixels to the newData array
                        for(Q_INT32 x=0; x < width; x++){
                                currentPos = (y*targetW+x+displacementInt) * m_dev -> pixelSize(); // try to be at least a little efficient
                                for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                        pixel[channel]=tempRow[x*m_dev -> pixelSize()+channel];
                                        left[channel]= (Q_INT32)(weight*pixel[channel]);
                                        pixel[channel]=pixel[channel]-left[channel]+oleft[channel];
                                        newData[currentPos  + channel]=pixel[channel];
                                        oleft[channel]=left[channel];
                                }
                        }
                }        
        } else {
                for (Q_INT32 y=0; y < height; y++){
                        emit notifyProgress(this, y * 100  / progressTotal + progressStart + 1);
                        progressCurrent++;
                        if (m_cancelRequested) {
                                break;
                        }
                        //calculate displacement
                        displacement = y*QABS(shearX);
                        displacementInt = (Q_INT32)(floor(displacement));
                        weight=displacement-displacementInt;
                        //read a row from the image
                        tempRow = m_dev -> readBytes( 0, y, width, 1);
                        //initialize oleft
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++)
                                oleft[channel]=left[channel]=0;
                        //copy the pixels to the newData array
                        for(Q_INT32 x=0; x < width; x++){
                                currentPos = (y*targetW+x+displacementInt) * m_dev -> pixelSize(); // try to be at least a little efficient
                                for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                        pixel[channel]=tempRow[x*m_dev -> pixelSize()+channel];
                                        left[channel]= (Q_INT32)(weight*pixel[channel]);
                                        pixel[channel]=pixel[channel]-left[channel]+oleft[channel];
                                        newData[currentPos  + channel]=pixel[channel];
                                        oleft[channel]=left[channel];
                                }
                        }
                }          
        }        
        //now write newData to the image
        kdDebug() << "write newData to the image!" << "\n";
        m_dev -> writeBytes( newData, 0, 0, targetW, targetH);
        return progressCurrent;
}

Q_INT32 KisRotateVisitor::yShearImage(double shearY, KisProgressDisplayInterface *m_progress, Q_INT32 progressTotal, Q_INT32 progressCurrent)
{
        kdDebug() << "YShearImage called, shear paramter " << shearY << "\n";
        
        Q_INT32 width = m_dev -> image() -> width();
        Q_INT32 height = m_dev -> image() -> height();
        double progressStart=(double)progressCurrent / progressTotal * 100; 
        
        //calculate widht of the sheared image
        Q_INT32 targetW = width;
        Q_INT32 targetH = (Q_INT32)(height + QABS(width*shearY));
        QUANTUM * newData = new QUANTUM[targetW * targetH * m_dev -> pixelSize() * sizeof(QUANTUM)];
        
        //shear the image
        QUANTUM *tempCol = new QUANTUM[height * m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *pixel = new QUANTUM[m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *left = new QUANTUM[m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *oleft = new QUANTUM[m_dev -> pixelSize() * sizeof(QUANTUM)];
        double displacement;
        Q_INT32 displacementInt;
        double weight;
        Q_INT32 currentPos;
        
        if(shearY>=0){
                for (Q_INT32 x=0; x < width; x++){
                        emit notifyProgress(this, x * 100  / progressTotal + progressStart + 1);
                        progressCurrent++;
                        if (m_cancelRequested) {
                                break;
                        }
                        //calculate displacement
                        displacement = x*shearY;
                        displacementInt = (Q_INT32)(floor(displacement));
                        weight=displacement-displacementInt;
                        //read a column from the image
                        tempCol = m_dev -> readBytes( x, 0, 1, height);
                        //initialize oleft
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++)
                                oleft[channel]=left[channel]=0;
                        //copy the pixels to the newData array
                        for(Q_INT32 y=0; y < height; y++){
                                currentPos = ((y+displacementInt)*targetW+x) * m_dev -> pixelSize(); // try to be at least a little efficient
                                for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                        pixel[channel]=tempCol[y*m_dev -> pixelSize()+channel];
                                        left[channel] = (Q_INT32)(weight*pixel[channel]);
                                        pixel[channel]=pixel[channel]-left[channel]+oleft[channel];
                                        newData[currentPos  + channel]=pixel[channel];
                                        oleft[channel]=left[channel];
                                }
                        }
                }        
        } else {
                for (Q_INT32 x=0; x < width; x++){
                        emit notifyProgress(this, x * 100  / progressTotal + progressStart + 1);
                        progressCurrent++;
                        if (m_cancelRequested) {
                                break;
                        }
                        //calculate displacement
                        displacement = (width-x)*QABS(shearY);
                        displacementInt = (Q_INT32)(floor(displacement));
                        weight=displacement-displacementInt;
                        //read a column from the image
                        tempCol = m_dev -> readBytes( x, 0, 1, height);
                        //initialize oleft
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++)
                                oleft[channel]=left[channel]=0;
                        //copy the pixels to the newData array
                        for(Q_INT32 y=0; y < height; y++){
                                currentPos = ((y+displacementInt)*targetW+x) * m_dev -> pixelSize(); // try to be at least a little efficient
                                for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                        pixel[channel]=tempCol[y*m_dev -> pixelSize()+channel];
                                        left[channel] = (Q_INT32)(weight*pixel[channel]);
                                        pixel[channel]=pixel[channel]-left[channel]+oleft[channel];
                                        newData[currentPos  + channel]=pixel[channel];
                                        oleft[channel]=left[channel];
                                }
                        }
                }        
        }
        //now write newData to the image
        kdDebug() << "write newData to the image!" << "\n";
        m_dev -> writeBytes( newData, 0, 0, targetW, targetH);
        return progressCurrent;
}

void KisRotateVisitor::xCropImage(double deltaX)
{
        Q_INT32 width = m_dev -> image() -> width();
        Q_INT32 height = m_dev -> image() -> height();
        //calculate widht of the croped image
        Q_INT32 targetW = (Q_INT32)(width -2 * deltaX + 2);
        Q_INT32 targetH = height;
        QUANTUM * newData = new QUANTUM[targetW * targetH * m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *tempRow = new QUANTUM[width * m_dev -> pixelSize() * sizeof(QUANTUM)];
        Q_INT32 currentPos;
        for(Q_INT32 y=0; y < height; y++){
                tempRow = m_dev -> readBytes( 0, y, width, 1);
                for(Q_INT32 x = (Q_INT32)deltaX; x < (Q_INT32)((width - deltaX) + 1); x++){
                        currentPos = (y*targetW+x) * m_dev -> pixelSize();
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                newData[currentPos - (int)deltaX*m_dev -> pixelSize() + channel]=tempRow[x*m_dev -> pixelSize()+channel];
                        }    
                }
        }
        kdDebug() << "write newData to the image!" << "\n";
        m_dev -> writeBytes( newData, 0, 0, targetW, targetH);
}

void KisRotateVisitor::yCropImage(double deltaY)
{
        Q_INT32 width = m_dev -> image() -> width();
        Q_INT32 height = m_dev -> image() -> height();
        //calculate widht of the croped image
        Q_INT32 targetW = width;
        Q_INT32 targetH = (Q_INT32)(height - 2 * deltaY + 2);
        QUANTUM * newData = new QUANTUM[targetW * targetH * m_dev -> pixelSize() * sizeof(QUANTUM)];
        QUANTUM *tempRow = new QUANTUM[width * m_dev -> pixelSize() * sizeof(QUANTUM)];
        Q_INT32 currentPos;
        for(Q_INT32 y = (Q_INT32)deltaY; y < (Q_INT32)(height - deltaY); y++){
                tempRow = m_dev -> readBytes( 0, y, width, 1);
                for(Q_INT32 x=0; x < width; x++){
                        currentPos = (y*targetW+x) * m_dev -> pixelSize();
                        for(int channel = 0; channel < m_dev -> pixelSize(); channel++){
                                newData[currentPos - (int)deltaY*targetW*m_dev -> pixelSize() + channel]=tempRow[x*m_dev -> pixelSize()+channel];
                        }    
                }
        }
        kdDebug() << "write newData to the image!" << "\n";
        m_dev -> writeBytes( newData, 0, 0, targetW, targetH);
}

void KisRotateVisitor::rotateRight90()
{
	Q_INT32 x, y, rx, ry, rw, rh;
	m_dev -> extent(rx, ry, rw, rh);
	Q_INT32 pixelSize = m_dev -> pixelSize();
	
	x = 0;
	for (y = ry; y < rh; ++y) {
		KisHLineIterator hit = m_dev -> createHLineIterator(rx, y, rw, true);
		KisVLineIterator vit = m_dev -> createVLineIterator(x, ry, rh, false);
		while (!hit.isDone() && !vit.isDone()) {
			memcpy(hit.rawData(), vit.oldRawData(), pixelSize);
			++hit;
			++vit;
		}
		x = x + 1;
		qApp -> processEvents();
	}
}

void KisRotateVisitor::rotateLeft90()
{
	Q_INT32 x, y, rx, ry, rw, rh;
	m_dev -> extent(rx, ry, rw, rh);
	Q_INT32 pixelSize = m_dev -> pixelSize();
	
	x = rw;
	for (y = ry; y < rh; ++y) {
		KisHLineIterator hit = m_dev -> createHLineIterator(rx, y, rw, true);
		KisVLineIterator vit = m_dev -> createVLineIterator(x, ry, rh, false);
		while (!hit.isDone() && !vit.isDone()) {
			memcpy(hit.rawData(), vit.oldRawData(), pixelSize);
			++hit;
			++vit;
		}
		x = x - 1;
		qApp -> processEvents();
	}
}

void KisRotateVisitor::rotate180()
{
	rotateLeft90();
	rotateLeft90();
}
