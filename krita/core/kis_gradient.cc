/*
 *  kis_gradient.cc - part of Krayon
 *
 *  Copyright (c) 2001 John Califf <jcaliff@compuzone.net>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <math.h>
#include <qpixmap.h>
#include <kimageeffect.h>
#include <kdebug.h>
#include "kis_selection.h"
#include "kis_gradient.h"
#include "kis_color.h"

//#define DEBUG_COLORS

KisGradient::KisGradient()
{
    mEffect = KImageEffect::VerticalGradient;
    setNull();
}


KisGradient::~KisGradient()
{
}


void KisGradient::setNull()
{
    mGradientWidth  = 0;
    mGradientHeight = 0;
    gradArray.resize(0);
}

/*
    mapKdeGradient - preferred method of mapping a predefined
    kde gradient to a QImage.  This works well for 2 color
    gradients - I see no need for more colors although different
    blending methods and periodicity needs to be considred later
    for our native gradients and gimp imports

    Below are the predefined types of kde gradients. Nice!

    enum GradientType
    { VerticalGradient, HorizontalGradient,
      DiagonalGradient, CrossDiagonalGradient,
      PyramidGradient, RectangleGradient,
      PipeCrossGradient, EllipticGradient };
*/

void KisGradient::mapKdeGradient(QRect gradR,
            KisColor startColor, KisColor endColor )
{
    mGradientWidth  = gradR.width();
    mGradientHeight = gradR.height();

    gradArray.resize(mGradientWidth * mGradientHeight);
    gradArray.fill(0);

    QSize size(mGradientWidth, mGradientHeight);
    QColor ca(startColor.R(), startColor.G(), startColor.B());
    QColor cb(endColor.R(), endColor.G(), endColor.B());

    // use gradient effect selected with gradient dialog
    QImage tmpImage = KImageEffect::gradient(size, ca, cb, mEffect, 0);

    QPixmap tmpPix(mGradientWidth, mGradientHeight, 8);
    tmpPix.convertFromImage(tmpImage, QPixmap::ColorOnly);

    gradImage = tmpPix.convertToImage();
    gradImage.convertDepth(32);
}


void KisGradient::mapVertGradient( QRect gradR,
            KisColor startColor, KisColor endColor )
{
    mGradientWidth  = gradR.width();
    mGradientHeight = gradR.height();

    gradArray.resize(mGradientWidth * mGradientHeight);
    gradArray.fill(0);

    uint color = 0;

    // draw gradient within rectanguar area defined above
    int length = gradR.height();

    int rDiff = ( endColor.R() - startColor.R() );
    int gDiff = ( endColor.G() - startColor.G() );
    int bDiff = ( endColor.B() - startColor.B() );

    int rl = startColor.R();
    int gl = startColor.G();
    int bl = startColor.B();

    float rlFloat = (float)rl;
    float glFloat = (float)gl;
    float blFloat = (float)bl;

    int y1 = 0;
    int y2 = gradR.height();
    int x1 = 0;
    int x2 = gradR.width();

    // gradient defined vertically
    for( int y = y1 ; y < y2 ; y++ )
    {
        // calc color
        float rlFinFloat
            = rlFloat + ((float) (y - y1) * (float)rDiff) / (float)length;
        float glFinFloat
            = glFloat + ((float) (y - y1) * (float)gDiff) / (float)length;
        float blFinFloat
            = blFloat + ((float) (y - y1) * (float)bDiff) / (float)length;

        uint red   = (uint)rlFinFloat;
        uint green = (uint)glFinFloat;
        uint blue  = (uint)blFinFloat;

        color = (0xff000000) | (red << 16) | (green << 8) | (blue);

        // draw uniform horizontal line of color -
        for( int x = x1 ; x < x2 ; x++ )
        {
            gradArray[y * (x2 - x1) + (x - x1)] = color;
        }
    }
}


void KisGradient::mapHorGradient( QRect gradR,
                                  KisColor /*startColor*/,
                                  KisColor /*endColor*/ )
{
    mGradientWidth  = gradR.width();
    mGradientHeight = gradR.height();

    gradArray.resize(mGradientWidth * mGradientHeight);
    gradArray.fill(0);

    // draw gradient within rectanguar area defined above
    //int length = gradR.width();
}


/* copied from kimageeffect.cc - needs dithering, though */

QImage KisGradient::gradient(const QSize &size, const QColor &ca,
	const QColor &cb, KImageEffect::GradientType eff, int ncols)
{
    int rDiff, gDiff, bDiff, maxDiff;
    int rca, gca, bca, rcb, gcb, bcb;
    int deltaY = 1;

    QImage image(size, 32);

    if (size.width() == 0 || size.height() == 0)
    {
        kdWarning() << "Invalid image" << endl;
        return image;
    }

    register int x, y;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    maxDiff = QMAX(rDiff, gDiff);
    maxDiff = QMAX(bDiff, maxDiff);

    if( eff == KImageEffect::VerticalGradient
    || eff == KImageEffect::HorizontalGradient )
    {
        uint *p;
        uint *p1; //jwc
        uint rgb;
        uint oldrgb = 0; // jwc

        register int rl = rca << 16;
        register int gl = gca << 16;
        register int bl = bca << 16;

        // vertical gradient means colors change with vertical increments
        // this is somewhat confusing - each color line is laid out
        // horizontally
        if( eff == KImageEffect::VerticalGradient )
        {
            if (maxDiff < 1) maxDiff = 1;

            deltaY = size.height()/maxDiff; //jwc
            if (deltaY < 1) deltaY = 1;

            kdDebug() << "deltaY " <<  deltaY << endl;

            int rcdelta = ((1<<16) / size.height()) * rDiff;
            int gcdelta = ((1<<16) / size.height()) * gDiff;
            int bcdelta = ((1<<16) / size.height()) * bDiff;

            for ( y = 0; y < size.height(); y++ )
            {
                p = (uint *) image.scanLine(y);

                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                rgb = qRgb( (rl>>16), (gl>>16), (bl>>16) );
                if(y == 0) oldrgb = rgb;


#ifdef DEBUG_COLORS

    kdDebug() << qRed(rgb) << " " << qGreen(rgb) << " " << qBlue(rgb) << endl;

#endif
                for( x = 0; x < size.width(); x++ )
                {

                    *p = rgb;
                    p++;

                }

                // dither
                p1 = (uint *) image.scanLine(y);

                if(!(y % deltaY))
                {
                    oldrgb = rgb; //jwc
                }

                for( x = 0; x < size.width(); x++ )
                {
                   if(x%2)
                   {
                      *p1 = oldrgb;
                   }
                   p1++;
                }
            }
        }
        else
        {                  // must be HorizontalGradient

            unsigned int *o_src = (unsigned int *)image.scanLine(0);
            unsigned int *src = o_src;

            int rcdelta = ((1<<16) / size.width()) * rDiff;
            int gcdelta = ((1<<16) / size.width()) * gDiff;
            int bcdelta = ((1<<16) / size.width()) * bDiff;

            for( x = 0; x < size.width(); x++)
            {

                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                *src++ = qRgb( (rl>>16), (gl>>16), (bl>>16));
            }

            src = o_src;

            // Believe it or not, manually copying in a for loop is faster
            // than calling memcpy for each scanline (on the order of ms...).
            // I think this is due to the function call overhead (mosfet).

            for (y = 1; y < size.height(); ++y)
            {

                p = (unsigned int *)image.scanLine(y);
                src = o_src;
                for(x=0; x < size.width(); ++x)
                    *p++ = *src++;
            }
        }
    }

    else
    {

        float rfd, gfd, bfd;
        float rd = rca, gd = gca, bd = bca;

        unsigned char *xtable[3];
        unsigned char *ytable[3];

        unsigned int w = size.width(), h = size.height();
        xtable[0] = new unsigned char[w];
        xtable[1] = new unsigned char[w];
        xtable[2] = new unsigned char[w];
        ytable[0] = new unsigned char[h];
        ytable[1] = new unsigned char[h];
        ytable[2] = new unsigned char[h];
        w*=2, h*=2;

        if ( eff == KImageEffect::DiagonalGradient
        || eff == KImageEffect::CrossDiagonalGradient)
        {
            // Diagonal dgradient code inspired by BlackBox (mosfet)
            // BlackBox dgradient is (C) Brad Hughes, <bhughes@tcac.net> and
            // Mike Cole <mike@mydot.com>.

            rfd = (float)rDiff/w;
            gfd = (float)gDiff/w;
            bfd = (float)bDiff/w;

            int dir;
            for (x = 0; x < size.width(); x++, rd+=rfd, gd+=gfd, bd+=bfd)
            {
                dir = eff == KImageEffect::DiagonalGradient?
                    x : size.width() - x - 1;
                xtable[0][dir] = (unsigned char) rd;
                xtable[1][dir] = (unsigned char) gd;
                xtable[2][dir] = (unsigned char) bd;
            }
            rfd = (float)rDiff/h;
            gfd = (float)gDiff/h;
            bfd = (float)bDiff/h;
            rd = gd = bd = 0;
            for (y = 0; y < size.height(); y++, rd+=rfd, gd+=gfd, bd+=bfd)
            {
                ytable[0][y] = (unsigned char) rd;
                ytable[1][y] = (unsigned char) gd;
                ytable[2][y] = (unsigned char) bd;
            }

            for (y = 0; y < size.height(); y++)
            {
                unsigned int *scanline = (unsigned int *)image.scanLine(y);
                for (x = 0; x < size.width(); x++)
                {
                    scanline[x] = qRgb(xtable[0][x] + ytable[0][y],
                                       xtable[1][x] + ytable[1][y],
                                       xtable[2][x] + ytable[2][y]);
                }
            }
        }

        else if (eff == KImageEffect::RectangleGradient ||
                 eff == KImageEffect::PyramidGradient ||
                 eff == KImageEffect::PipeCrossGradient ||
                 eff == KImageEffect::EllipticGradient)
        {
            int rSign = rDiff>0? 1: -1;
            int gSign = gDiff>0? 1: -1;
            int bSign = bDiff>0? 1: -1;

            rfd = (float)rDiff / size.width();
            gfd = (float)gDiff / size.width();
            bfd = (float)bDiff / size.width();

            rd = (float)rDiff/2;
            gd = (float)gDiff/2;
            bd = (float)bDiff/2;

            for (x = 0; x < size.width(); x++, rd-=rfd, gd-=gfd, bd-=bfd)
            {
                xtable[0][x] = (unsigned char) abs((int)rd);
                xtable[1][x] = (unsigned char) abs((int)gd);
                xtable[2][x] = (unsigned char) abs((int)bd);
            }

            rfd = (float)rDiff/size.height();
            gfd = (float)gDiff/size.height();
            bfd = (float)bDiff/size.height();

            rd = (float)rDiff/2;
            gd = (float)gDiff/2;
            bd = (float)bDiff/2;

            for (y = 0; y < size.height(); y++, rd-=rfd, gd-=gfd, bd-=bfd)
            {
                ytable[0][y] = (unsigned char) abs((int)rd);
                ytable[1][y] = (unsigned char) abs((int)gd);
                ytable[2][y] = (unsigned char) abs((int)bd);
            }
            unsigned int rgb;
            int h = (size.height()+1)>>1;
            for (y = 0; y < h; y++)
            {
                unsigned int *sl1 = (unsigned int *)image.scanLine(y);
                unsigned int *sl2 = (unsigned int *)image.scanLine(QMAX(size.height()-y-1, y));

                int w = (size.width()+1)>>1;
                int x2 = size.width()-1;

                for (x = 0; x < w; x++, x2--)
                {
		                rgb = 0;

                    if (eff == KImageEffect::PyramidGradient)
                    {
                        rgb = qRgb(rcb-rSign*(xtable[0][x]+ytable[0][y]),
                                   gcb-gSign*(xtable[1][x]+ytable[1][y]),
                                   bcb-bSign*(xtable[2][x]+ytable[2][y]));
                    }
                    if (eff == KImageEffect::RectangleGradient)
                    {
                        rgb = qRgb(rcb - rSign *
                                   QMAX(xtable[0][x], ytable[0][y]) * 2,
                                   gcb - gSign *
                                   QMAX(xtable[1][x], ytable[1][y]) * 2,
                                   bcb - bSign *
                                   QMAX(xtable[2][x], ytable[2][y]) * 2);
                    }
                    if (eff == KImageEffect::PipeCrossGradient)
                    {
                        rgb = qRgb(rcb - rSign *
                                   QMIN(xtable[0][x], ytable[0][y]) * 2,
                                   gcb - gSign *
                                   QMIN(xtable[1][x], ytable[1][y]) * 2,
                                   bcb - bSign *
                                   QMIN(xtable[2][x], ytable[2][y]) * 2);
                    }
                    if (eff == KImageEffect::EllipticGradient)
                    {
                        rgb = qRgb(rcb - rSign *
                                   (int)sqrt((xtable[0][x]*xtable[0][x] +
                                              ytable[0][y]*ytable[0][y])*2.0),
                                   gcb - gSign *
                                   (int)sqrt((xtable[1][x]*xtable[1][x] +
                                              ytable[1][y]*ytable[1][y])*2.0),
                                   bcb - bSign *
                                   (int)sqrt((xtable[2][x]*xtable[2][x] +
                                              ytable[2][y]*ytable[2][y])*2.0));
                    }

                    sl1[x] = sl2[x] = rgb;
                    sl1[x2] = sl2[x2] = rgb;
                }
            }
        }

        delete [] xtable[0];
        delete [] xtable[1];
        delete [] xtable[2];
        delete [] ytable[0];
        delete [] ytable[1];
        delete [] ytable[2];
    }

    // dither if necessary
    if (ncols && (QPixmap::defaultDepth() < 15 ))
    {
	    if ( ncols < 2 || ncols > 256 )
	        ncols = 3;

	    QColor *dPal = new QColor[ncols];
	    for (int i=0; i<ncols; i++)
        {
	        dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
			     gca + gDiff * i / ( ncols - 1 ),
			     bca + bDiff * i / ( ncols - 1 ) );
	    }

        dither(image, dPal, ncols);
	    delete [] dPal;
    }

    return image;
}


/*
    adapted from kFSDither (C) 1997 Martin Jones (mjones@kde.org)

    Floyd-Steinberg dithering
    Ref: Bitmapped Graphics Programming in C++
        Marv Luse, Addison-Wesley Publishing, 1993.
*/

QImage& KisGradient::dither(QImage &img, const QColor *palette, int size)
{
    if (img.width() == 0 || img.height() == 0 ||
        palette == 0 || img.depth() <= 8)
      return img;

    QImage dImage( img.width(), img.height(), 8, size );
    int i;

    dImage.setNumColors( size );
    for ( i = 0; i < size; i++ )
        dImage.setColor( i, palette[ i ].rgb() );

    int *rerr1 = new int [ img.width() * 2 ];
    int *gerr1 = new int [ img.width() * 2 ];
    int *berr1 = new int [ img.width() * 2 ];

    memset( rerr1, 0, sizeof( int ) * img.width() * 2 );
    memset( gerr1, 0, sizeof( int ) * img.width() * 2 );
    memset( berr1, 0, sizeof( int ) * img.width() * 2 );

    int *rerr2 = rerr1 + img.width();
    int *gerr2 = gerr1 + img.width();
    int *berr2 = berr1 + img.width();

    for ( int j = 0; j < img.height(); j++ )
    {
        uint *ip = (uint * )img.scanLine( j );
        uchar *dp = dImage.scanLine( j );

        for ( i = 0; i < img.width(); i++ )
        {
            rerr1[i] = rerr2[i] + qRed( *ip );
            rerr2[i] = 0;
            gerr1[i] = gerr2[i] + qGreen( *ip );
            gerr2[i] = 0;
            berr1[i] = berr2[i] + qBlue( *ip );
            berr2[i] = 0;
            ip++;
        }

        *dp++ = nearestColor( rerr1[0], gerr1[0], berr1[0], palette, size );

        for ( i = 1; i < img.width()-1; i++ )
        {
            int indx = nearestColor( rerr1[i], gerr1[i], berr1[i],
                palette, size );
            *dp = indx;

            int rerr = rerr1[i];
            rerr -= palette[indx].red();
            int gerr = gerr1[i];
            gerr -= palette[indx].green();
            int berr = berr1[i];
            berr -= palette[indx].blue();

            // diffuse red error
            rerr1[ i+1 ] += ( rerr * 7 ) >> 4;
            rerr2[ i-1 ] += ( rerr * 3 ) >> 4;
            rerr2[  i  ] += ( rerr * 5 ) >> 4;
            rerr2[ i+1 ] += ( rerr ) >> 4;

            // diffuse green error
            gerr1[ i+1 ] += ( gerr * 7 ) >> 4;
            gerr2[ i-1 ] += ( gerr * 3 ) >> 4;
            gerr2[  i  ] += ( gerr * 5 ) >> 4;
            gerr2[ i+1 ] += ( gerr ) >> 4;

            // diffuse red error
            berr1[ i+1 ] += ( berr * 7 ) >> 4;
            berr2[ i-1 ] += ( berr * 3 ) >> 4;
            berr2[  i  ] += ( berr * 5 ) >> 4;
            berr2[ i+1 ] += ( berr ) >> 4;

            dp++;
        }

        *dp = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
    }

    delete [] rerr1;
    delete [] gerr1;
    delete [] berr1;

    img = dImage;
    return img;
}


int KisGradient::nearestColor( int r, int g, int b,
    const QColor *palette, int size )
{
    if (palette == 0)
      return 0;

    int dr = palette[0].red() - r;
    int dg = palette[0].green() - g;
    int db = palette[0].blue() - b;

    int minDist =  dr*dr + dg*dg + db*db;
    int nearest = 0;

    for (int i = 1; i < size; i++ )
    {
        dr = palette[i].red() - r;
        dg = palette[i].green() - g;
        db = palette[i].blue() - b;

        int dist = dr*dr + dg*dg + db*db;

        if ( dist < minDist )
        {
            minDist = dist;
            nearest = i;
        }
    }

    return nearest;
}
