/* 
 Copyright (C) 2004, 2005, 2007
 Daniel M. Duley <daniel.duley@verizon.net>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/*
 Portions of this software are were originally based on ImageMagick's
 algorithms. ImageMagick is copyrighted under the following conditions:

Copyright (C) 2003 ImageMagick Studio, a non-profit organization dedicated to
making software imaging solutions freely available.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files ("ImageMagick"), to deal
in ImageMagick without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense,  and/or sell
copies of ImageMagick, and to permit persons to whom the ImageMagick is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of ImageMagick.

The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement.  In no event shall
ImageMagick Studio be liable for any claim, damages or other liability,
whether in an action of contract, tort or otherwise, arising from, out of or
in connection with ImageMagick or the use or other dealings in ImageMagick.

Except as contained in this notice, the name of the ImageMagick Studio shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in ImageMagick without prior written authorization from the
ImageMagick Studio.
*/

#include <QImage>
#include <cmath>
#include "scalefilter.h"
/**
 * This is a port of the ImageMagick scaling functions from resize.c.
 *
 * The most signficant change is ImageMagick uses a function pointer for the
 * filter type. This is called usually a couple times in a loop for each
 * horizontal and vertical coordinate. I changed this into a switch statement
 * that does each type with inline functions. More code but faster.
 */

#define MagickEpsilon  1.0e-6
#define MagickPI  3.14159265358979323846264338327950288419716939937510

namespace BlitzScaleFilter{
    typedef struct{
        float weight;
        unsigned int pixel;
    } ContributionInfo;

    bool horizontalFilter(QImage *srcImg, QImage *destImg,
                          float x_factor, float blur,
                          ContributionInfo *contribution,
                          Blitz::ScaleFilterType filter);
    bool verticalFilter(QImage *srcImg, QImage *destImg,
                        float y_factor, float blur,
                        ContributionInfo *contribution,
                        Blitz::ScaleFilterType filter);

    // These arrays were moved from their respective functions because they
    // are inline
    static const float
        J1Pone[] = {
            0.581199354001606143928050809e+21f,
            -0.6672106568924916298020941484e+20f,
            0.2316433580634002297931815435e+19f,
            -0.3588817569910106050743641413e+17f,
            0.2908795263834775409737601689e+15f,
            -0.1322983480332126453125473247e+13f,
            0.3413234182301700539091292655e+10f,
            -0.4695753530642995859767162166e+7f,
            0.270112271089232341485679099e+4f
        },
        J1Qone[] = {
            0.11623987080032122878585294e+22f,
            0.1185770712190320999837113348e+20f,
            0.6092061398917521746105196863e+17f,
            0.2081661221307607351240184229e+15f,
            0.5243710262167649715406728642e+12f,
            0.1013863514358673989967045588e+10f,
            0.1501793594998585505921097578e+7f,
            0.1606931573481487801970916749e+4f,
            0.1e+1
        };

    static const float
        P1Pone[] = {
            0.352246649133679798341724373e+5f,
            0.62758845247161281269005675e+5f,
            0.313539631109159574238669888e+5f,
            0.49854832060594338434500455e+4f,
            0.2111529182853962382105718e+3f,
            0.12571716929145341558495e+1f
        },
        P1Qone[] = {
            0.352246649133679798068390431e+5f,
            0.626943469593560511888833731e+5f,
            0.312404063819041039923015703e+5f,
            0.4930396490181088979386097e+4f,
            0.2030775189134759322293574e+3f,
            0.1e+1f
        };

    static const float
        Q1Pone[] = {
            0.3511751914303552822533318e+3f,
            0.7210391804904475039280863e+3f,
            0.4259873011654442389886993e+3f,
            0.831898957673850827325226e+2f,
            0.45681716295512267064405e+1f,
            0.3532840052740123642735e-1f
        },
        Q1Qone[] = {
            0.74917374171809127714519505e+4f,
            0.154141773392650970499848051e+5f,
            0.91522317015169922705904727e+4f,
            0.18111867005523513506724158e+4f,
            0.1038187585462133728776636e+3f,
            0.1e+1
        };

    static const float filterSupport[Blitz::SincFilter+1] = {
        /*Undefined*/ 0.0f,
        /*Point*/ 0.0f,
        /*Box*/ 0.5f,
        /*Triangle*/ 1.0f,
        /*Hermite*/ 1.0f,
        /*Hanning*/ 1.0f,
        /*Hamming*/ 1.0f,
        /*Blackman*/ 1.0f,
        /*Gaussian*/ 1.25f,
        /*Quadratic*/ 1.5f,
        /*Cubic*/ 2.0f,
        /*Catrom*/ 2.0f,
        /*Mitchell*/ 2.0f,
        /*Lanczos*/ 3.0f,
        /*BlackmanBessel*/ 3.2383f,
        /*BlackmanSinc*/ 4.0f
    };

    inline float J1(float x){
        float p, q;
        p=J1Pone[8]; q=J1Qone[8];
        for(int i=7; i >= 0; i--){
            p=p*x*x+J1Pone[i];
            q=q*x*x+J1Qone[i];
        }
        return(p/q);
    }

    inline float P1(float x){
        float p, q;
        p=P1Pone[5]; q=P1Qone[5];
        for(int i=4; i >= 0; i--){
            p=p*(8.0/x)*(8.0/x)+P1Pone[i];
            q=q*(8.0/x)*(8.0/x)+P1Qone[i];
        }
        return(p/q);
    }

    inline float Q1(float x){
        float p, q;
        p=Q1Pone[5]; q=Q1Qone[5];
        for(int i=4; i >= 0; i--){
            p=p*(8.0/x)*(8.0/x)+Q1Pone[i];
            q=q*(8.0/x)*(8.0/x)+Q1Qone[i];
        }
        return(p/q);
    }

    inline float BesselOrderOne(float x){
        float p, q;
        if(x == 0.0)
            return(0.0);
        p = x;
        if(x < 0.0)
            x = (-x);
        if(x < 8.0)
            return(p*J1(x));
        q = std::sqrt((float) (2.0/(MagickPI*x)))*(P1(x)*(1.0/std::sqrt(2.0)*(std::sin(x)-
            std::cos(x)))-8.0/x*Q1(x)*(-1.0/std::sqrt(2.0)*(std::sin(x)+
                                                            std::cos(x))));
        if (p < 0.0)
            q=(-q);
        return(q);
    }

    inline float Bessel(const float x, const float /*support*/){
        if(x == 0.0)
            return((float)(MagickPI/4.0));
        return(BesselOrderOne(MagickPI*x)/(2.0*x));
    }

    inline float Sinc(const float x, const float /*support*/){
        if(x == 0.0)
            return(1.0);
        return(std::sin(MagickPI*x)/(MagickPI*x));
    }

    inline float Blackman(const float x, const float /*support*/){
        return(0.42+0.5*std::cos(MagickPI*x)+0.08*std::cos(2*MagickPI*x));
    }

    inline float BlackmanBessel(const float x,const float support){
        return(Blackman(x/support,support)*Bessel(x,support));
    }

    inline float BlackmanSinc(const float x, const float support){
        return(Blackman(x/support,support)*Sinc(x,support));
    }

    inline float Box(const float x, const float /*support*/){
        if(x < -0.5)
            return(0.0);
        if(x < 0.5)
            return(1.0);
        return(0.0);
    }

    inline float Catrom(const float x, const float /*support*/){
        if(x < -2.0)
            return(0.0);
        if(x < -1.0)
            return(0.5*(4.0+x*(8.0+x*(5.0+x))));
        if(x < 0.0)
            return(0.5*(2.0+x*x*(-5.0-3.0*x)));
        if(x < 1.0)
            return(0.5*(2.0+x*x*(-5.0+3.0*x)));
        if(x < 2.0)
            return(0.5*(4.0+x*(-8.0+x*(5.0-x))));
        return(0.0);
    }

    inline float Cubic(const float x, const float /*support*/){
        if(x < -2.0)
            return(0.0);
        if(x < -1.0)
            return((2.0+x)*(2.0+x)*(2.0+x)/6.0);
        if(x < 0.0)
            return((4.0+x*x*(-6.0-3.0*x))/6.0);
        if(x < 1.0)
            return((4.0+x*x*(-6.0+3.0*x))/6.0);
        if(x < 2.0)
            return((2.0-x)*(2.0-x)*(2.0-x)/6.0);
        return(0.0);
    }

    inline float Gaussian(const float x, const float /*support*/){
        return(std::exp((float)(-2.0*x*x))*std::sqrt(2.0/MagickPI));
    }

    inline float Hanning(const float x, const float /*support*/){
        return(0.5+0.5*std::cos(MagickPI*(double) x));
    }

    inline float Hamming(const float x, const float /*support*/){
        return(0.54+0.46*std::cos(MagickPI*(double) x));
    }

    inline float Hermite(const float x, const float /*support*/){
        if(x < -1.0)
            return(0.0);
        if(x < 0.0)
            return((2.0*(-x)-3.0)*(-x)*(-x)+1.0);
        if(x < 1.0)
            return((2.0*x-3.0)*x*x+1.0);
        return(0.0);
    }

    inline float Lanczos(const float x, const float support){
        if(x < -3.0)
            return(0.0);
        if(x < 0.0)
            return(Sinc(-x,support)*Sinc(-x/3.0,support));
        if(x < 3.0)
            return(Sinc(x,support)*Sinc(x/3.0,support));
        return(0.0);
    }

    inline float Mitchell(const float x, const float /*support*/){
#define B   (1.0/3.0)
#define C   (1.0/3.0)
#define P0  ((  6.0- 2.0*B       )/6.0)
#define P2  ((-18.0+12.0*B+ 6.0*C)/6.0)
#define P3  (( 12.0- 9.0*B- 6.0*C)/6.0)
#define Q0  ((       8.0*B+24.0*C)/6.0)
#define Q1  ((     -12.0*B-48.0*C)/6.0)
#define Q2  ((       6.0*B+30.0*C)/6.0)
#define Q3  ((     - 1.0*B- 6.0*C)/6.0)
        if(x < -2.0)
            return(0.0);
        if(x < -1.0)
            return(Q0-x*(Q1-x*(Q2-x*Q3)));
        if(x < 0.0)
            return(P0+x*x*(P2-x*P3));
        if(x < 1.0)
            return(P0+x*x*(P2+x*P3));
        if(x < 2.0)
            return(Q0+x*(Q1+x*(Q2+x*Q3)));
        return(0.0);
    }

    inline float Quadratic(const float x, const float /*support*/){
        if(x < -1.5)
            return(0.0);
        if(x < -0.5)
            return(0.5*(x+1.5)*(x+1.5));
        if(x < 0.5)
            return(0.75-x*x);
        if(x < 1.5)
            return(0.5*(x-1.5)*(x-1.5));
        return(0.0);
    }

    inline float Triangle(const float x, const float /*support*/){
        if(x < -1.0)
            return(0.0);
        if(x < 0.0)
            return(1.0+x);
        if(x < 1.0)
            return(1.0-x);
        return(0.0);
    }
}

using namespace BlitzScaleFilter;


//
// Horizontal and vertical filters
//

bool BlitzScaleFilter::horizontalFilter(QImage *srcImg,
                                        QImage *destImg,
                                        float x_factor, float blur,
                                        ContributionInfo *contribution,
                                        Blitz::ScaleFilterType filter)
{
    int n, start, stop, i, x, y;
    float center, density, scale, support;
    float r, g, b, a;
    QRgb *srcData = (QRgb *)srcImg->bits();
    QRgb *destData = (QRgb *)destImg->bits();
    int sw = srcImg->width();
    int dw = destImg->width();
    QRgb pixel;

    scale = blur*qMax(1.0/x_factor, 1.0);
    support = scale*filterSupport[filter];
    if(support <= 0.5){
        support = float(0.5+MagickEpsilon);
        scale = 1.0;
    }
    scale = 1.0/scale;

    for(x=0; x < destImg->width(); ++x){
        center = (float) (x+0.5)/x_factor;
        start = (int)qMax((double)center-support+0.5, (double)0.0);
        stop = (int)qMin((double)center+support+0.5, (double)srcImg->width());
        density=0.0;

        for(n=0; n < (stop-start); ++n){
            contribution[n].pixel = start+n;
            switch(filter){
            case Blitz::UndefinedFilter:
            default:
                contribution[n].weight =
                    Box(scale*((float)(start+n)-center+0.5),
                        filterSupport[filter]);
                break;
            case Blitz::PointFilter:
                contribution[n].weight =
                    Box(scale*((float)(start+n)-center+0.5),
                        filterSupport[filter]);
                break;
            case Blitz::BoxFilter:
                contribution[n].weight =
                    Box(scale*((float)(start+n)-center+0.5),
                        filterSupport[filter]);
                break;
            case Blitz::TriangleFilter:
                contribution[n].weight =
                    Triangle(scale*((float)(start+n)-center+0.5),
                             filterSupport[filter]);
                break;
            case Blitz::HermiteFilter:
                contribution[n].weight =
                    Hermite(scale*((float)(start+n)-center+0.5),
                            filterSupport[filter]);
                break;
            case Blitz::HanningFilter:
                contribution[n].weight =
                    Hanning(scale*((float)(start+n)-center+0.5),
                            filterSupport[filter]);
                break;
            case Blitz::HammingFilter:
                contribution[n].weight =
                    Hamming(scale*((float)(start+n)-center+0.5),
                            filterSupport[filter]);
                break;
            case Blitz::BlackmanFilter:
                contribution[n].weight =
                    Blackman(scale*((float)(start+n)-center+0.5),
                             filterSupport[filter]);
                break;
            case Blitz::GaussianFilter:
                contribution[n].weight =
                    Gaussian(scale*((float)(start+n)-center+0.5),
                             filterSupport[filter]);
                break;
            case Blitz::QuadraticFilter:
                contribution[n].weight =
                    Quadratic(scale*((float)(start+n)-center+0.5),
                              filterSupport[filter]);
                break;
            case Blitz::CubicFilter:
                contribution[n].weight =
                    Cubic(scale*((float)(start+n)-center+0.5),
                          filterSupport[filter]);
                break;
            case Blitz::CatromFilter:
                contribution[n].weight =
                    Catrom(scale*((float)(start+n)-center+0.5),
                           filterSupport[filter]);
                break;
            case Blitz::MitchellFilter:
                contribution[n].weight =
                    Mitchell(scale*((float)(start+n)-center+0.5),
                             filterSupport[filter]);
                break;
            case Blitz::LanczosFilter:
                contribution[n].weight =
                    Lanczos(scale*((float)(start+n)-center+0.5),
                        filterSupport[filter]);
                break;
            case Blitz::BesselFilter:
                contribution[n].weight =
                    BlackmanBessel(scale*((float)(start+n)-center+0.5),
                                   filterSupport[filter]);
                break;
            case Blitz::SincFilter:
                contribution[n].weight =
                    BlackmanSinc(scale*((float)(start+n)-center+0.5),
                                 filterSupport[filter]);
                break;
            }
            density += contribution[n].weight;
        }

        if((density != 0.0) && (density != 1.0)){
            // Normalize
            density = 1.0/density;
            for(i=0; i < n; ++i)
                contribution[i].weight *= density;
        }

        for(y=0; y < destImg->height(); ++y){
            r = g = b = a = 0;
            for(i=0; i < n; ++i){
                pixel = *(srcData+(y*sw)+contribution[i].pixel);
                r += qRed(pixel)*contribution[i].weight;
                g += qGreen(pixel)*contribution[i].weight;
                b += qBlue(pixel)*contribution[i].weight;
                a += qAlpha(pixel)*contribution[i].weight;
            }
            r = r < 0 ? 0 : r > 255 ? 255 : r + 0.5;
            g = g < 0 ? 0 : g > 255 ? 255 : g + 0.5;
            b = b < 0 ? 0 : b > 255 ? 255 : b + 0.5;
            a = a < 0 ? 0 : a > 255 ? 255 : a + 0.5;
            *(destData+(y*dw)+x) = qRgba((unsigned char)r,
                                         (unsigned char)g,
                                         (unsigned char)b,
                                         (unsigned char)a);
        }
    }
    return(true);
}

bool BlitzScaleFilter::verticalFilter(QImage *srcImg,
                                      QImage *destImg,
                                      float y_factor, float blur,
                                      ContributionInfo *contribution,
                                      Blitz::ScaleFilterType filter)
{
    int n, start, stop, i, x, y;
    float center, density, scale, support;
    float r, g, b, a;
    QRgb *srcData = (QRgb *)srcImg->bits();
    QRgb *destData = (QRgb *)destImg->bits();
    int sw = srcImg->width();
    int dw = destImg->width();
    QRgb pixel;

    scale = blur*qMax(1.0/y_factor, 1.0);
    support = scale*filterSupport[filter];
    if(support <= 0.5){
        support = float(0.5+MagickEpsilon);
        scale = 1.0;
    }
    scale = 1.0/scale;

    for(y=0; y < destImg->height(); ++y){
        center = (float) (y+0.5)/y_factor;
        start = (int)qMax((double)center-support+0.5, (double)0.0);
        stop = (int)qMin((double)center+support+0.5, (double)srcImg->height());
        density=0.0;

        for(n=0; n < (stop-start); ++n){
            contribution[n].pixel = start+n;
            switch(filter){
            case Blitz::UndefinedFilter:
            default:
                contribution[n].weight =
                    Box(scale*((float)(start+n)-center+0.5),
                        filterSupport[filter]);
                break;
            case Blitz::PointFilter:
                contribution[n].weight =
                    Box(scale*((float)(start+n)-center+0.5),
                        filterSupport[filter]);
                break;
            case Blitz::BoxFilter:
                contribution[n].weight =
                    Box(scale*((float)(start+n)-center+0.5),
                        filterSupport[filter]);
                break;

            case Blitz::TriangleFilter:
                contribution[n].weight =
                    Triangle(scale*((float)(start+n)-center+0.5),
                             filterSupport[filter]);
                break;
            case Blitz::HermiteFilter:
                contribution[n].weight =
                    Hermite(scale*((float)(start+n)-center+0.5),
                            filterSupport[filter]);
                break;
            case Blitz::HanningFilter:
                contribution[n].weight =
                    Hanning(scale*((float)(start+n)-center+0.5),
                            filterSupport[filter]);
                break;
            case Blitz::HammingFilter:
                contribution[n].weight =
                    Hamming(scale*((float)(start+n)-center+0.5),
                            filterSupport[filter]);
                break;
            case Blitz::BlackmanFilter:
                contribution[n].weight =
                    Blackman(scale*((float)(start+n)-center+0.5),
                             filterSupport[filter]);
                break;
            case Blitz::GaussianFilter:
                contribution[n].weight =
                    Gaussian(scale*((float)(start+n)-center+0.5),
                             filterSupport[filter]);
                break;
            case Blitz::QuadraticFilter:
                contribution[n].weight =
                    Quadratic(scale*((float)(start+n)-center+0.5),
                              filterSupport[filter]);
                break;
            case Blitz::CubicFilter:
                contribution[n].weight =
                    Cubic(scale*((float)(start+n)-center+0.5),
                          filterSupport[filter]);
                break;
            case Blitz::CatromFilter:
                contribution[n].weight =
                    Catrom(scale*((float)(start+n)-center+0.5),
                           filterSupport[filter]);
                break;
            case Blitz::MitchellFilter:
                contribution[n].weight =
                    Mitchell(scale*((float)(start+n)-center+0.5),
                             filterSupport[filter]);
                break;
            case Blitz::LanczosFilter:
                contribution[n].weight =
                    Lanczos(scale*((float)(start+n)-center+0.5),
                        filterSupport[filter]);
                break;
            case Blitz::BesselFilter:
                contribution[n].weight =
                    BlackmanBessel(scale*((float)(start+n)-center+0.5),
                                   filterSupport[filter]);
                break;
            case Blitz::SincFilter:
                contribution[n].weight =
                    BlackmanSinc(scale*((float)(start+n)-center+0.5),
                                 filterSupport[filter]);
                break;
            }
            density += contribution[n].weight;
        }

        if((density != 0.0) && (density != 1.0)){
            // Normalize
            density = 1.0/density;
            for(i=0; i < n; ++i)
                contribution[i].weight *= density;
        }

        for(x=0; x < destImg->width(); ++x){
            r = g = b = a = 0;
            for(i=0; i < n; ++i){
                pixel = *(srcData+(contribution[i].pixel*sw)+x);
                r += qRed(pixel)*contribution[i].weight;
                g += qGreen(pixel)*contribution[i].weight;
                b += qBlue(pixel)*contribution[i].weight;
                a += qAlpha(pixel)*contribution[i].weight;
            }
            r = r < 0 ? 0 : r > 255 ? 255 : r + 0.5;
            g = g < 0 ? 0 : g > 255 ? 255 : g + 0.5;
            b = b < 0 ? 0 : b > 255 ? 255 : b + 0.5;
            a = a < 0 ? 0 : a > 255 ? 255 : a + 0.5;
            *(destData+(y*dw)+x) = qRgba((unsigned char)r,
                                         (unsigned char)g,
                                         (unsigned char)b,
                                         (unsigned char)a);
        }
    }
    return(true);
}

QImage Blitz::smoothScaleFilter(QImage &img, int w, int h,
                                float blur, ScaleFilterType filter,
                                Qt::AspectRatioMode aspectRatio)
{
    return(smoothScaleFilter(img, QSize(w, h), blur, filter, aspectRatio));
}

QImage Blitz::smoothScaleFilter(QImage &img, const QSize &sz,
                                float blur, ScaleFilterType filter,
                                Qt::AspectRatioMode aspectRatio)
{
    QSize destSize(img.size());
    destSize.scale(sz, aspectRatio);
    if(img.isNull() || !destSize.isValid())
        return(img);
    int dw = destSize.width();
    int dh = destSize.height();

    if(img.depth() != 32){
        img = img.convertToFormat(img.hasAlphaChannel() ?
                                  QImage::Format_ARGB32 :
                                  QImage::Format_RGB32);
    }
    else if(img.format() == QImage::Format_ARGB32_Premultiplied)
        img = img.convertToFormat(QImage::Format_ARGB32);

    QImage buffer(destSize, img.hasAlphaChannel() ?
                  QImage::Format_ARGB32 : QImage::Format_RGB32);

    ContributionInfo *contribution;
    float support, x_factor, x_support, y_factor, y_support;
    int i;

    //
    // Allocate filter contribution info.
    //
    x_factor= (float)buffer.width()/img.width();
    y_factor= (float)buffer.height()/img.height();
    i = (int)LanczosFilter;
    if(filter != UndefinedFilter)
        i = (int)filter;
    else
        if((x_factor == 1.0) && (y_factor == 1.0))
            i = (int)PointFilter;
        else
            i = (int)MitchellFilter;
    x_support = blur*qMax(1.0/x_factor, 1.0)*filterSupport[i];
    y_support = blur*qMax(1.0/y_factor, 1.0)*filterSupport[i];
    support = qMax(x_support, y_support);
    if(support < filterSupport[i])
        support = filterSupport[i];
    contribution =
        new ContributionInfo[(int)(2.0*qMax((double)support, (double)0.5)+3)];

    //
    // Scale
    //
    if((dw*(img.height()+dh)) > (dh*(img.width()+dw))){
        QImage tmp(dw, img.height(), buffer.format());
        horizontalFilter(&img, &tmp, x_factor, blur, contribution, filter);
        verticalFilter(&tmp, &buffer, y_factor, blur, contribution, filter);
    }
    else{
        QImage tmp(img.width(), dh, buffer.format());
        verticalFilter(&img, &tmp, y_factor, blur, contribution, filter);
        horizontalFilter(&tmp, &buffer, x_factor, blur, contribution, filter);
    }

    delete[] contribution;
    return(buffer);
}


