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
#include "kis_raindrops_filter_visitor.h"
#include "kis_progress_display_interface.h"

void KisRainDropsFilterVisitor::rainDropsFilter(Q_UINT32 dropSize, Q_UINT32 number, Q_UINT32 fishEyes, KisProgressDisplayInterface *m_progress)
{
        kdDebug() << "dropSize:" << dropSize << " number:" << number << " fishEyes:" << fishEyes << "\n";

        /*Q_UINT32 width = m_dev -> width();
        Q_UINT32 height = m_dev -> height();
        Q_UINT32 depth = m_dev -> depth();
        
        KisTileMgrSP tm = new KisTileMgr(m_dev -> colorStrategy() -> depth(), width, height);
        QUANTUM * newData = new QUANTUM[width * height * depth * sizeof(QUANTUM)];

        //copy the image to newData
        m_dev -> tiles() -> readPixelData(0, 0, width-1, height-1, newData, m_dev -> depth());
        
        //rainDrops(newData, width, height, dropSize, number, fishEyes);

        tm -> writePixelData(0, 0, width - 1, height - 1, newData, width * m_dev -> depth());
        m_dev -> setTiles(tm); // Also sets width and height correctly
        */
        
        kdDebug() << "rotate180 called!" << "\n";
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
        rainDrops(newData, width, height, dropSize, number, fishEyes, m_progress);
        kdDebug() << "write newData to the image!" << "\n";
        tm -> writePixelData(0, 0, targetW - 1, targetH - 1, newData, targetW * m_dev -> depth());
        m_dev -> setTiles(tm); // Also sets width and height correctly
}

void KisRainDropsFilterVisitor::rainDrops(QUANTUM *data, int Width, int Height, int DropSize, int Amount, int Coeff, KisProgressDisplayInterface *m_progress)
{
        //Progress info
        m_cancelRequested = false;
        m_progress -> setSubject(this, true, true);
        emit notifyProgressStage(this,i18n("Applying oilpaint filter..."),0);
    
        int BitCount = 0;

        if (Coeff <= 0) Coeff = 1;
    
        if (Coeff > 100) Coeff = 100;

        int LineWidth = Width * 4;                     
        if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);

        BitCount = LineWidth * Height;
        uchar*    Bits = (uchar*)data;
        uchar* NewBits = new uchar[BitCount];
        bool** BoolMatrix = CreateBoolArray (Width, Height);

        int       i, j, k, l, m, n;                 // loop variables
        int       p, q;                             // positions
        int       Bright;                           // Bright value for shadows and highlights
        int       x, y;                             // center coordinates
        int       Counter = 0;                      // Counter (duh !)
        int       NewSize;                          // Size of current raindrop
        int       halfSize;                         // Half of the current raindrop
        int       Radius;                           // Maximum radius for raindrop
        int       BlurRadius;                       // Blur Radius
        int       BlurPixels;
    
        double    r, a;                             // polar coordinates
        double    OldRadius;                        // Radius before processing
        double    NewCoeff = (double)Coeff * 0.01;  // FishEye Coefficients
        double    s;
        double    R, G, B;

        bool      FindAnother = false;              // To search for good coordinates
    
        QDateTime dt = QDateTime::currentDateTime();
        QDateTime Y2000( QDate(2000, 1, 1), QTime(0, 0, 0) );
    
        srand ((uint) dt.secsTo(Y2000));
    
        bool m_cancel=false; //only to make it compile
    
        // Init booleen Matrix.
    
        for (i = 0 ; !m_cancel && (i < Width) ; ++i)
        {
                for (j = 0 ; !m_cancel && (j < Height) ; ++j)
                {
                        p = j * LineWidth + 4 * i;         
                        NewBits[p+3] = Bits[p+3];
                        NewBits[p+2] = Bits[p+2];
                        NewBits[p+1] = Bits[p+1];
                        NewBits[ p ] = Bits[ p ];
                        BoolMatrix[i][j] = false;
                }
       }

        for (int NumBlurs = 0 ; !m_cancel && (NumBlurs <= Amount) ; ++NumBlurs)
        {
                NewSize = (int)(rand() * ((double)(DropSize - 5) / RAND_MAX) + 5);
                halfSize = NewSize / 2;
                Radius = halfSize;
                s = Radius / log (NewCoeff * Radius + 1);

                Counter = 0;
        
                do
                {
                        FindAnother = false;
                        y = (int)(rand() * ((double)( Width - 1) / RAND_MAX));
                        x = (int)(rand() * ((double)(Height - 1) / RAND_MAX));

                        if (BoolMatrix[y][x])
                                FindAnother = true;
                        else
                                for (i = x - halfSize ; !m_cancel && (i <= x + halfSize) ; i++)
                                        for (j = y - halfSize ; !m_cancel && (j <= y + halfSize) ; j++)
                                                if ((i >= 0) && (i < Height) && (j >= 0) && (j < Width))
                                                        if (BoolMatrix[j][i])
                                                                FindAnother = true;

                        Counter++;
                } 
        while (!m_cancel && (FindAnother && (Counter < 10000)) );

        if (Counter >= 10000)
        {
                NumBlurs = Amount;
                break;
        }

        for (i = -1 * halfSize ; !m_cancel && (i < NewSize - halfSize) ; i++)
        {
                for (j = -1 * halfSize ; !m_cancel && (j < NewSize - halfSize) ; j++)
                {
                        r = sqrt (i * i + j * j);
                        a = atan2 (i, j);

                        if (r <= Radius)
                        {
                                OldRadius = r;
                                r = (exp (r / s) - 1) / NewCoeff;

                                k = x + (int)(r * sin (a));
                                l = y + (int)(r * cos (a));

                                m = x + i;
                                n = y + j;

                                if ((k >= 0) && (k < Height) && (l >= 0) && (l < Width))
                                {
                                        if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width))
                                        {
                                                p = k * LineWidth + 4 * l;        
                                                q = m * LineWidth + 4 * n;        
                                                NewBits[q+2] = Bits[p+2];
                                                NewBits[q+1] = Bits[p+1];
                                                NewBits[ q ] = Bits[ p ];
                                                BoolMatrix[n][m] = true;
                                                Bright = 0;
                                
                                                if (OldRadius >= 0.9 * Radius)
                                                {
                                                        if ((a <= 0) && (a > -2.25))
                                                                Bright = -80;
                                                        else if ((a <= -2.25) && (a > -2.5))
                                                                Bright = -40;
                                                        else if ((a <= 0.25) && (a > 0))
                                                                Bright = -40;
                                                }

                                                else if (OldRadius >= 0.8 * Radius)
                                                {
                                                        if ((a <= -0.75) && (a > -1.50))
                                                                Bright = -40;
                                                        else if ((a <= 0.10) && (a > -0.75))
                                                                Bright = -30;
                                                        else if ((a <= -1.50) && (a > -2.35))
                                                                Bright = -30;
                                                }

                                                else if (OldRadius >= 0.7 * Radius)
                                                {
                                                        if ((a <= -0.10) && (a > -2.0))
                                                                Bright = -20;
                                                        else if ((a <= 2.50) && (a > 1.90))
                                                                Bright = 60;
                                                }
                                
                                                else if (OldRadius >= 0.6 * Radius)
                                                {
                                                        if ((a <= -0.50) && (a > -1.75))
                                                                Bright = -20;
                                                        else if ((a <= 0) && (a > -0.25))
                                                                Bright = 20;
                                                        else if ((a <= -2.0) && (a > -2.25))
                                                                Bright = 20;
                                                }
        
                                                else if (OldRadius >= 0.5 * Radius)
                                                {
                                                        if ((a <= -0.25) && (a > -0.50))
                                                                Bright = 30;
                                                        else if ((a <= -1.75 ) && (a > -2.0))
                                                                Bright = 30;
                                                }
        
                                                else if (OldRadius >= 0.4 * Radius)
                                                {
                                                        if ((a <= -0.5) && (a > -1.75))
                                                                Bright = 40;
                                                }
        
                                                else if (OldRadius >= 0.3 * Radius)
                                                {
                                                        if ((a <= 0) && (a > -2.25))
                                                                Bright = 30;
                                                }
        
                                                else if (OldRadius >= 0.2 * Radius)
                                                {
                                                        if ((a <= -0.5) && (a > -1.75))
                                                                Bright = 20;
                                                }
        
                                                NewBits[q+2] = LimitValues (NewBits[q+2] + Bright);
                                                NewBits[q+1] = LimitValues (NewBits[q+1] + Bright);
                                                NewBits[ q ] = LimitValues (NewBits[ q ] + Bright);
                                        }
                                }
                        }
                }
        }

        BlurRadius = NewSize / 25 + 1;
            
        for (i = -1 * halfSize - BlurRadius ; !m_cancel && (i < NewSize - halfSize + BlurRadius) ; i++)
        {
                for (j = -1 * halfSize - BlurRadius ; !m_cancel && (j < NewSize - halfSize + BlurRadius) ; j++)
                {
                        r = sqrt (i * i + j * j);
                
                        if (r <= Radius * 1.1)
                        {
                                R = G = B = 0;
                                BlurPixels = 0;
                        
                                for (k = -1 * BlurRadius; k < BlurRadius + 1; k++)
                                        for (l = -1 * BlurRadius; l < BlurRadius + 1; l++)
                                        {
                                                m = x + i + k;
                                                n = y + j + l;
                            
                                                if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width))
                                                {
                                                        p = m * LineWidth + 4 * n;           
                                                        R += NewBits[p+2];
                                                        G += NewBits[p+1];
                                                        B += NewBits[ p ];
                                                        BlurPixels++;
                                                }
                                        }

                                        m = x + i;
                                        n = y + j;
                        
                                        if ((m >= 0) && (m < Height) && (n >= 0) && (n < Width))
                                        {
                                                p = m * LineWidth + 4 * n;                  
                                                NewBits[p+2] = (uchar)(R / BlurPixels);
                                                NewBits[p+1] = (uchar)(G / BlurPixels);
                                                NewBits[ p ] = (uchar)(B / BlurPixels);
                                        }
                                }
                        }
                }
        emit notifyProgress(this, (int) (((double)NumBlurs * 100.0) / Amount));
        // Update the progress bar in dialog.
        // m_progressBar->setValue((int) (((double)NumBlurs * 100.0) / Amount));
        // kapp->processEvents(); 
        }
    
        if (!m_cancel) 
        memcpy (data, NewBits, BitCount);        
        
        delete [] NewBits;

        FreeBoolArray (BoolMatrix, Width);
        emit notifyProgressDone(this);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.

/* Function to free a dinamic boolean array                                            
 *                                                               
 * lpbArray          => Dynamic boolean array                                      
 * Columns           => The array bidimension value                                 
 *                                                                                  
 * Theory            => An easy to undestand 'for' statement                          
 */
void KisRainDropsFilterVisitor::FreeBoolArray (bool** lpbArray, uint Columns)
{
        for (uint i = 0; i < Columns; ++i)
                free (lpbArray[i]);
        
        free (lpbArray);
}

/* Function to create a bidimentional dinamic boolean array                  
 *                                                                                
 * Columns           => Number of columns                                          
 * Rows              => Number of rows                                              
 *                                                                                  
 * Theory            => Using 'for' statement, we can alloc multiple dinamic arrays   
 *                      To create more dimentions, just add some 'for's, ok?           
 */
bool** KisRainDropsFilterVisitor::CreateBoolArray (uint Columns, uint Rows)
{
        bool** lpbArray = NULL;
        lpbArray = (bool**) malloc (Columns * sizeof (bool*));

        if (lpbArray == NULL)
                return (NULL);

        for (uint i = 0; i < Columns; ++i)
        {
                lpbArray[i] = (bool*) malloc (Rows * sizeof (bool));
                if (lpbArray[i] == NULL)
                {
                        FreeBoolArray (lpbArray, Columns);
                        return (NULL);
                }
        }

        return (lpbArray);
}

// This method have been ported from Pieter Z. Voloshyn algorithm code.  
 
/* This function limits the RGB values                        
 *                                                                         
 * ColorValue        => Here, is an RGB value to be analized                   
 *                                                                             
 * Theory            => A color is represented in RGB value (e.g. 0xFFFFFF is     
 *                      white color). But R, G and B values has 256 values to be used   
 *                      so, this function analize the value and limits to this range   
 */   

uchar KisRainDropsFilterVisitor::LimitValues (int ColorValue)
{
        if (ColorValue > 255)        // MAX = 255
                ColorValue = 255;        
        if (ColorValue < 0)          // MIN = 0
                ColorValue = 0;
        return ((uchar) ColorValue);
}
                   

