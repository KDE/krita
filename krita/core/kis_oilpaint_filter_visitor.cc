/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  ported from digikam, copyright 2004 by Gilles Caulier
 *
 * Original RainDrop algorithm copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt includes

#include <qdatetime.h> 
#include <qtimer.h>
#include <qwmatrix.h>


#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_oilpaint_filter_visitor.h"
#include "kis_progress_display_interface.h"

void KisOilPaintFilterVisitor::oilPaintFilter(Q_UINT32 brushSize, Q_UINT32 smooth, KisProgressDisplayInterface *m_progress)
{
        kdDebug() << "brushSize:" << brushSize << " smooth:" << smooth << "\n";

        Q_INT32 width = m_dev->width();
        Q_INT32 height = m_dev->height();
        //calculate widht of the croped image
        Q_INT32 targetW = width;
        Q_INT32 targetH = height;
        kdDebug() << "targetW: " << targetW << " targetH: " << targetH << "\n";
        KisTileMgrSP tm = new KisTileMgr(m_dev -> colorStrategy() -> depth(), targetW, targetH);
        QUANTUM * newData = new QUANTUM[targetW * targetH * m_dev -> depth() * sizeof(QUANTUM)];
        QUANTUM *tempRow = new QUANTUM[width * m_dev -> depth() * sizeof(QUANTUM)];
        Q_INT32 currentPos;
        //read the image to an QUANTUM array
        for(Q_INT32 y=0; y < height; y++){
                m_dev -> tiles() -> readPixelData(0, y, width-1, y, tempRow, m_dev -> depth());
                for(Q_INT32 x=0; x < width; x++){
                        currentPos = (y*targetW+x) * m_dev -> depth();
                        for(int channel = 0; channel < m_dev -> depth(); channel++){
                                newData[currentPos + channel]=tempRow[x*m_dev -> depth()+channel];
                        }    
                }
        }
        OilPaint(newData, width, height, brushSize, smooth, m_progress);
        kdDebug() << "write newData to the image!" << "\n";
        tm -> writePixelData(0, 0, targetW - 1, targetH - 1, newData, targetW * m_dev -> depth());
        m_dev -> setTiles(tm); // Also sets width and height correctly
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to apply the OilPaint effect.                
 *                                                                                    
 * data             => The image data in RGBA mode.                            
 * w                => Width of image.                          
 * h                => Height of image.                          
 * BrushSize        => Brush size.
 * Smoothness       => Smooth value.                                                
 *                                                                                  
 * Theory           => Using MostFrequentColor function we take the main color in  
 *                     a matrix and simply write at the original position.            
 */                                                                                 
    
void KisOilPaintFilterVisitor::OilPaint(QUANTUM* data, int w, int h, int BrushSize, int Smoothness, KisProgressDisplayInterface *m_progress)
{
        bool m_cancel=false; //to make it compile
    
        //Progress info
        m_cancelRequested = false;
        m_progress -> setSubject(this, true, true);
        emit notifyProgressStage(this,i18n("Applying oilpaint filter..."),0);
    
        int LineWidth = w * 4;
        if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
      
        uchar* newBits = (uchar*)data;
        int i = 0;
        uint color;
    
        for (int h2 = 0; !m_cancel && (h2 < h); ++h2)
        {
                for (int w2 = 0; !m_cancel && (w2 < w); ++w2)
                {
                        i = h2 * LineWidth + 4*w2;
                        color = MostFrequentColor ((uchar*)data, w, h, w2, h2, BrushSize, Smoothness);
                
                        newBits[i+3] = qAlpha(color);
                        newBits[i+2] = qBlue(color);
                        newBits[i+1] = qGreen(color);
                        newBits[ i ] = qRed(color);
                }
       
       // Update de progress bar in dialog.
       emit notifyProgress(this, (int) (((double)h2 * 100.0) / h));
       //m_progressBar->setValue((int) (((double)h2 * 100.0) / h));
       //kapp->processEvents();          
       }
emit notifyProgressDone(this);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to determine the most frequent color in a matrix                        
 *                                                                                
 * Bits             => Bits array                                                    
 * Width            => Image width                                                   
 * Height           => Image height                                                 
 * X                => Position horizontal                                           
 * Y                => Position vertical                                            
 * Radius           => Is the radius of the matrix to be analized                  
 * Intensity        => Intensity to calcule                                         
 *                                                                                  
 * Theory           => This function creates a matrix with the analized pixel in   
 *                     the center of this matrix and find the most frequenty color   
 */
 
uint KisOilPaintFilterVisitor::MostFrequentColor (uchar* Bits, int Width, int Height, int X, 
                                              int Y, int Radius, int Intensity)
{
    int i, w, h, I;
    uint color;
    
    double Scale = Intensity / 255.0;
    int LineWidth = 4 * Width;
    
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);   // Don't take off this step
        
    // Alloc some arrays to be used
    uchar *IntensityCount = new uchar[(Intensity + 1) * sizeof (uchar)];
    uint  *AverageColorR  = new uint[(Intensity + 1)  * sizeof (uint)];
    uint  *AverageColorG  = new uint[(Intensity + 1)  * sizeof (uint)];
    uint  *AverageColorB  = new uint[(Intensity + 1)  * sizeof (uint)];

    // Erase the array
    memset(IntensityCount, 0, (Intensity + 1) * sizeof (uchar));

    /*for (i = 0; i <= Intensity; ++i)
        IntensityCount[i] = 0;*/

    for (w = X - Radius; w <= X + Radius; ++w)
        {
        for (h = Y - Radius; h <= Y + Radius; ++h)
            {
            // This condition helps to identify when a point doesn't exist
            
            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
                {
                // You'll see a lot of times this formula
                i = h * LineWidth + 4 * w;
                I = (uint)(GetIntensity (Bits[i], Bits[i+1], Bits[i+2]) * Scale);
                IntensityCount[I]++;

                if (IntensityCount[I] == 1)
                    {
                    AverageColorR[I] = Bits[ i ];
                    AverageColorG[I] = Bits[i+1];
                    AverageColorB[I] = Bits[i+2];
                    }
                else
                    {
                    AverageColorR[I] += Bits[ i ];
                    AverageColorG[I] += Bits[i+1];
                    AverageColorB[I] += Bits[i+2];
                    }
                }
            }
        }

    I = 0;
    int MaxInstance = 0;

    for (i = 0 ; i <= Intensity ; ++i)
       {
       if (IntensityCount[i] > MaxInstance)
          {
          I = i;
          MaxInstance = IntensityCount[i];
          }
       }

    int R, G, B;
    R = AverageColorR[I] / MaxInstance;
    G = AverageColorG[I] / MaxInstance;
    B = AverageColorB[I] / MaxInstance;
    color = qRgb (R, G, B);

    delete [] IntensityCount;        // free all the arrays
    delete [] AverageColorR;
    delete [] AverageColorG;
    delete [] AverageColorB;

    return (color);                    // return the most frequenty color
}
