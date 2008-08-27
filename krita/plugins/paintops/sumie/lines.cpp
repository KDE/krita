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
#include "lines.h"
#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_random_accessor.h>

void Lines::drawLine(KisPaintDeviceSP dev, int x0,int y0,int x1,int y1,const KoColor &color)
{
    KisPainter gc( dev );
    gc.setPaintColor( color );
// call to KisPainter
    gc.drawThickLine( QPointF(x0, y0), QPointF(x1, y1), 1, 1);
// working implementation with RandomAccessor
//	drawThick(dev,color, QPointF(x0, y0), QPointF(x1, y1), 1, 1);
}


void Lines::drawDDALine(KisPaintDeviceSP image, int x0, int y0, int x1, int y1,const KoColor &color)
{
    KisPainter gc( image );
    gc.setPaintColor( color );
    gc.drawDDALine( QPointF(x0, y0), QPointF(x1, y1) );
}

void Lines::drawDDAALine(KisPaintDeviceSP image, int x0, int y0, int x1, int y1,const KoColor &color)
{
    KisPainter gc( image );
    gc.setPaintColor( color );
    gc.drawWobblyLine( QPointF(x0, y0), QPointF(x1, y1) );


}
void Lines::drawWuLine ( KisPaintDeviceSP dev, float x0, float y0, float x1, float y1, const KoColor &color )
{
    KisPainter gc( dev );
    gc.setPaintColor( color );
    gc.drawWuLine( QPointF(x0, y0), QPointF(x1, y1) );
}

// cool wu lines with thickness support
void Lines::drawThickLine ( KisPaintDeviceSP dev, int x0, int y0, int x1, int y1,
                            const KoColor color1, const KoColor color2, int w1, int w2 )
{
    Q_UNUSED(color2);
    KisPainter gc( dev );
    gc.setPaintColor( color1 );
    gc.drawThickLine( QPointF(x0, y0), QPointF(x1, y1), w1, w2);
}

void Lines::drawThick(KisPaintDeviceSP dev,KoColor color1,const QPointF & start, const QPointF & end, int startWidth, int endWidth){
    KisRandomAccessor accessor = dev->createRandomAccessor( (int)start.x(), (int)start.y() );
    int pixelSize = dev->colorSpace()->pixelSize();
    KoColorSpace * cs = dev->colorSpace();

    quint8 alpha;

    int x0 = (int)start.x();
    int y0 = (int)start.y();
    int x1 = (int)end.x();
    int y1 = (int)end.y();

    KoColor c1 ( color1 );
    KoColor c2 ( color1 );
    KoColor c3 ( color1 );
    KoColor col1 ( c1 );
    KoColor col2 ( c1 );

    float grada, gradb, dxa, dxb, dya, dyb, adya, adyb, fraca, fracb,
	xfa, yfa, xfb, yfb, b1a, b2a, b1b, b2b, dx, dy;
    int x, y, ix1, ix2, iy1, iy2;
    KoColor pix;
    int x0a, y0a, x1a, y1a, x0b, y0b, x1b, y1b;
    int tp0, tn0, tp1, tn1;
    int horizontal = 0;
    float opacity = 0.0;

    tp0 = startWidth/2;
    tn0 = startWidth/2;
    if ( startWidth%2==0 ) // even width startWidth
        tn0--;

    tp1 = endWidth/2;
    tn1 = endWidth/2;
    if ( endWidth%2==0 ) // even width endWidth
        tn1--;

    dx = x1 - x0; // run of general line
    dy = y1 - y0; // rise of general line

    if ( dy < 0 ) dy = -dy;
    if ( dx < 0 ) dx = -dx;

    if ( dx > dy ) // horizontalish
    {
        horizontal=1;
        x0a = x0;   y0a = y0 - tn0;
        x0b = x0;   y0b = y0 + tp0;
        x1a = x1;   y1a = y1 - tn1;
        x1b = x1;   y1b = y1 + tp1;
    }
    else
    {
        x0a = x0 - tn0;   y0a = y0;
        x0b = x0 + tp0;   y0b = y0;
        x1a = x1 - tn1;   y1a = y1;
        x1b = x1 + tp1;   y1b = y1;
    }

    if ( horizontal ) // draw endpoints
    {
        for ( int i=y0a; i<=y0b; i++ )
        {
            //dev->setPixel ( x0,i,c1 );
            accessor.moveTo(x0, i);
            memcpy( accessor.rawData(), c1.data(), pixelSize);
        }
        for ( int i=y1a; i<=y1b; i++ )
        {
            //	dev->setPixel ( x1,i,c1 );
            accessor.moveTo(x1, i);
            memcpy( accessor.rawData(), c1.data(), pixelSize);
        }
    }
    else
    {
        for ( int i=x0a; i<=x0b; i++ )
        {
            //	dev->setPixel ( i,y0,c1 );
            accessor.moveTo(i, y0);
            memcpy( accessor.rawData(), c1.data(), pixelSize);
        }

        for ( int i=x1a; i<=x1b; i++ )
        {
            //dev->setPixel ( i,y1,c1 );
            accessor.moveTo(i, y1);
            memcpy( accessor.rawData(), c1.data(), pixelSize);
        }

    }

    //antialias endpoints
    if ( x1!=x0 && y1!=y0 )
    {
        if ( horizontal )
        {

            //dev->pixel ( x0a,y0a-1,&pix );
            accessor.moveTo( x0a, y0a-1 );

            alpha = cs->alpha(accessor.rawData());
            opacity = .25*c1.opacity() + ( 1-.25 ) * alpha;
            col1.setOpacity ( static_cast<int> ( opacity ) );
            //dev->setPixel ( x0a, y0a-1, col1 );
            memcpy( accessor.rawData(), col1.data(), pixelSize);



            //dev->pixel ( x1b,y1b+1,&pix );
            accessor.moveTo( x1b, y1b+1 );

            alpha = cs->alpha(accessor.rawData());
            opacity = .25*c2.opacity() + ( 1-.25 ) * alpha;
            col1.setOpacity ( static_cast<int> ( opacity ) );
            //dev->setPixel ( x1b, y1b+1, col1 );
            memcpy( accessor.rawData(), col1.data(), pixelSize);

        }
        else
        {
            //dev->pixel ( x0a-1,y0a,&pix );
            accessor.moveTo( x0a-1,y0a );

            alpha = cs->alpha(accessor.rawData());
            opacity = .25*c1.opacity() + ( 1-.25 ) * alpha;
            col1.setOpacity ( static_cast<int> ( opacity ) );
            //dev->setPixel ( x0a-1, y0a, col1 );
            memcpy( accessor.rawData(), col1.data(), pixelSize);


            //dev->pixel ( x1b+1,y1b,&pix );
            accessor.moveTo( x1b+1,y1b );

            alpha = cs->alpha(accessor.rawData());
            opacity = .25*c2.opacity() + ( 1-.25 ) * alpha;
            col1.setOpacity ( static_cast<int> ( opacity ) );
            //dev->setPixel ( x1b+1, y1b, col1 );
            memcpy( accessor.rawData(), col1.data(), pixelSize);

        }
    }

    dxa = x1a - x0a; // run of a
    dya = y1a - y0a; // rise of a
    dxb = x1b - x0b; // run of b
    dyb = y1b - y0b; // rise of b

    if ( dya < 0 ) adya = -dya;
    else adya = dya;
    if ( dyb < 0 ) adyb = -dyb;
    else adyb = dyb;


    if ( horizontal ) // horizontal-ish lines
    {
        if ( x1 < x0 )
        {
            int xt, yt, wt;
            KoColor tmp;
            xt = x1a;     x1a = x0a;    x0a = xt;
            yt = y1a;     y1a = y0a;    y0a = yt;
            xt = x1b;     x1b = x0b;    x0b = xt;
            yt = y1b;     y1b = y0b;    y0b = yt;
            xt = x1;      x1 = x0;      x0 = xt;
            yt = y1;      y1 = y0;      y0 = yt;

            tmp = c1; c1= c2; c2 = tmp;
            wt = startWidth;      startWidth = endWidth;      endWidth = wt;
        }

        grada = dya/dxa;
        gradb = dyb/dxb;

        ix1 = x0;   iy1 = y0;
        ix2 = x1;   iy2 = y1;

        yfa = y0a + grada;
        yfb = y0b + gradb;

        for ( x = ix1+1; x <= ix2-1; x++ )
        {
            fraca = yfa - int ( yfa );
            b1a = 1 - fraca;
            b2a = fraca;

            fracb = yfb - int ( yfb );
            b1b = 1 - fracb;
            b2b = fracb;

            // color first pixel of bottom line
            opacity = ( ( x-ix1 ) /dx ) *c2.opacity() + ( 1 - ( x-ix1 ) /dx ) *c1.opacity();
            c3.setOpacity ( static_cast<int> ( opacity ) );

            //dev->pixel ( x, (int)yfa, &pix );
            accessor.moveTo( x, (int)yfa );

            alpha = cs->alpha(accessor.rawData());
            opacity = b1a*c3.opacity() + ( 1-b1a ) * alpha;
            col1.setOpacity ( static_cast<int> ( opacity ) );
            //dev->setPixel ( x, (int)yfa, col1 );
            memcpy( accessor.rawData(), col1.data(), pixelSize);



            // color first pixel of top line
            if ( ! ( startWidth==1 && endWidth==1 ) )
            {
                //dev->pixel ( x, (int)yfb, &pix );
                accessor.moveTo( x, (int)yfb );

                alpha = cs->alpha(accessor.rawData());
                opacity = b1b*c3.opacity() + ( 1-b1b ) * alpha;
                col1.setOpacity ( static_cast<int> ( opacity ) );
                //dev->setPixel ( x, (int)yfb, col1 );
                memcpy( accessor.rawData(), col1.data(), pixelSize);


            }

            // color second pixel of bottom line
            if ( grada != 0 && grada != 1 ) // if not flat or exact diagonal
            {
                //dev->pixel ( x, int ( yfa ) +1, &pix );
                accessor.moveTo( x, int(yfa) + 1 );

                alpha = cs->alpha(accessor.rawData());
                opacity = b2a*c3.opacity() + ( 1-b2a ) * alpha;
                col2.setOpacity ( static_cast<int> ( opacity ) );
                //dev->setPixel ( x, int ( yfa ) +1, col2 );
                memcpy( accessor.rawData(), col2.data(), pixelSize);

            }

            // color second pixel of top line
            if ( gradb != 0 && gradb != 1 && ! ( startWidth==1 && endWidth==1 ) )
            {
                //dev->pixel ( x, int ( yfb ) +1, &pix );
                accessor.moveTo( x, int( yfb ) + 1 );

                alpha = cs->alpha(accessor.rawData());
                opacity = b2b*c3.opacity() + ( 1-b2b ) * alpha;
                col2.setOpacity ( static_cast<int> ( opacity ) );
                //dev->setPixel ( x, int ( yfb ) +1, col2 );
                memcpy( accessor.rawData(), col2.data(), pixelSize);

            }

            // fill remaining pixels
            if ( ! ( startWidth==1 && endWidth==1 ) )
            {
                if ( yfa<yfb )
                    for ( int i=yfa+1; i<=yfb; i++ )
                    {
                        //dev->setPixel ( x,i, c3 );
                        accessor.moveTo( x, i );
                        memcpy( accessor.rawData(), c3.data(), pixelSize);
                    }
                else
                    for ( int i=yfa+1; i>=yfb; i-- )
                    {
                        //dev->setPixel ( x,i, c3 );
                        accessor.moveTo( x, i );
                        memcpy( accessor.rawData(), c3.data(), pixelSize);
                    }
            }

            yfa += grada;
            yfb += gradb;
        }
    }
    else // vertical-ish lines
    {
        if ( y1 < y0 )
        {
            int xt, yt, wt;
            xt = x1a;     x1a = x0a;    x0a = xt;
            yt = y1a;     y1a = y0a;    y0a = yt;
            xt = x1b;     x1b = x0b;    x0b = xt;
            yt = y1b;     y1b = y0b;    y0b = yt;
            xt = x1;      x1 = x0;      x0 = xt;
            yt = y1;      y1 = y0;      y0 = yt;

            KoColor tmp;
            tmp = c1; c1 = c2; c2 = tmp;
            wt = startWidth;      startWidth = endWidth;      endWidth = wt;
        }

        grada = dxa/dya;
        gradb = dxb/dyb;

        ix1 = x0;   iy1 = y0;
        ix2 = x1;   iy2 = y1;

        xfa = x0a + grada;
        xfb = x0b + gradb;

        for ( y = iy1+1; y <= iy2-1; y++ )
        {
            fraca = xfa - int ( xfa );
            b1a = 1 - fraca;
            b2a = fraca;

            fracb = xfb - int ( xfb );
            b1b = 1 - fracb;
            b2b = fracb;

            // color first pixel of left line
            opacity = ( ( y-iy1 ) /dy ) *c2.opacity() + ( 1 - ( y-iy1 ) /dy ) *c1.opacity();
            c3.setOpacity ( static_cast<int> ( opacity ) );


            //dev->pixel ( int ( xfa ), y, &pix );
            accessor.moveTo( int ( xfa ), y );

            alpha = cs->alpha(accessor.rawData());
            opacity = b1a*c3.opacity() + ( 1-b1a ) * alpha;
            col1.setOpacity ( static_cast<int> ( opacity ) );
            //dev->setPixel ( int ( xfa ), y, col1 );
            memcpy( accessor.rawData(), col1.data(), pixelSize);




            // color first pixel of right line
            if ( ! ( startWidth==1 && endWidth==1 ) )
            {
                //dev->pixel ( int ( xfb ), y, &pix );
                accessor.moveTo( int ( xfb ), y );
                alpha = cs->alpha(accessor.rawData());
                opacity = b1b*c3.opacity() + ( 1-b1b ) * alpha;
                col1.setOpacity ( static_cast<int> ( opacity ) );
                //dev->setPixel ( int ( xfb ), y, col1 );
                memcpy( accessor.rawData(), col1.data(), pixelSize);
            }

            // color second pixel of left line
            if ( grada != 0 && grada != 1 ) // if not flat or exact diagonal
            {
                //dev->pixel ( int ( xfa ) +1, y, &pix );
                accessor.moveTo( int ( xfa ) +1, y );

                alpha = cs->alpha(accessor.rawData());
                opacity = b2a*c3.opacity() + ( 1-b2a ) * alpha;
                col2.setOpacity ( static_cast<int> ( opacity ) );
//					dev->setPixel ( int ( xfa ) +1, y, col2 );
                memcpy( accessor.rawData(), col2.data(), pixelSize);


                // color second pixel of right line
                if ( gradb != 0 && gradb != 1 && ! ( startWidth==1 && endWidth==1 ) )
                {
// 				dev->pixel ( int ( xfb ) +1, y, &pix );
                    accessor.moveTo( int ( xfb ) +1, y );

                    alpha = cs->alpha(accessor.rawData());
                    opacity = b2b*c3.opacity() + ( 1-b2b ) * alpha;
                    col2.setOpacity ( static_cast<int> ( opacity ) );
// 					dev->setPixel ( int ( xfb ) +1, y, col2 );
                    memcpy( accessor.rawData(), col2.data(), pixelSize);

                }

                // fill remaining pixels between current xfa,xfb
                if ( ! ( startWidth==1 && endWidth==1 ) )
                {
                    if ( xfa<xfb )
                        for ( int i= ( int ) xfa+1; i<= ( int ) xfb; i++ )
                        {
// 						dev->setPixel ( i,y,c3 );
                            accessor.moveTo( i,y );

                            memcpy( accessor.rawData(), c3.data(), pixelSize);

                        }
                    else
                        for ( int i= ( int ) xfb; i<= ( int ) xfa+1; i++ )
                        {
// 						dev->setPixel ( i,y,c3 );
                            accessor.moveTo( i,y );

                            memcpy( accessor.rawData(), c3.data(), pixelSize);

                        }

                }
                xfa += grada;
                xfb += gradb;
            }
	}
    }//else

}//end of routine





