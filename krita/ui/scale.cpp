// This file includes code for scaling images, in two versions.
// One ported from ImageMagick (slower, but can achieve better quality),
// and from Imlib2 ported by Mosfet (very fast).


// ImageMagick code begin
// ----------------------

// This code is ImageMagick's resize code, adapted for QImage, with
// fastfloat class added as an optimization.
// The original license text follows.

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                 RRRR   EEEEE  SSSSS  IIIII  ZZZZZ  EEEEE                    %
%                 R   R  E      SS       I       ZZ  E                        %
%                 RRRR   EEE     SSS     I     ZZZ   EEE                      %
%                 R R    E         SS    I    ZZ     E                        %
%                 R  R   EEEEE  SSSSS  IIIII  ZZZZZ  EEEEE                    %
%                                                                             %
%                     ImageMagick Image Resize Methods                        %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright (C) 2003 ImageMagick Studio, a non-profit organization dedicated %
%  to making software imaging solutions freely available.                     %
%                                                                             %
%  Permission is hereby granted, free of charge, to any person obtaining a    %
%  copy of this software and associated documentation files ("ImageMagick"),  %
%  to deal in ImageMagick without restriction, including without limitation   %
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   %
%  and/or sell copies of ImageMagick, and to permit persons to whom the       %
%  ImageMagick is furnished to do so, subject to the following conditions:    %
%                                                                             %
%  The above copyright notice and this permission notice shall be included in %
%  all copies or substantial portions of ImageMagick.                         %
%                                                                             %
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/
#include "config.h"

// System
#ifdef HAVE_ENDIAN_H
#include <endian.h>
#else
#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif
#endif

// Qt
#include <qimage.h>
#include <qcolor.h>

#include <kdeversion.h>
#include <kcpuinfo.h>

#include <string.h>
#include <stdlib.h>

// Local
#include "imageutils.h"

// everything in namespace
namespace ImageUtils {


#define Max QMAX
#define Min QMIN

// mustn't be less than used precision (i.e. 1/fastfloat::RATIO)
#define MagickEpsilon 0.0002

// fastfloat begin
// this class stores floating point numbers as integers, with BITS shift,
// i.e. value XYZ is stored as XYZ * RATIO
struct fastfloat
    {
    private:
        enum { BITS = 12, RATIO = 4096 };
    public:
        fastfloat() {}
        fastfloat( long v ) : value( v << BITS ) {}
        fastfloat( int v ) : value( v << BITS ) {}
        fastfloat( double v ) : value( static_cast< long >( v * RATIO + 0.5 )) {}
        double toDouble() const { return static_cast< double >( value ) / RATIO; }
        long toLong() const { return value >> BITS; }
        fastfloat& operator += ( fastfloat r ) { value += r.value; return *this; }
        fastfloat& operator -= ( fastfloat r ) { value -= r.value; return *this; }
        fastfloat& operator *= ( fastfloat r ) { value = static_cast< long long >( value ) * r.value >> BITS; return *this; }
        fastfloat& operator /= ( fastfloat r ) { value = ( static_cast< long long >( value ) << BITS ) / r.value; return *this; }
        bool operator< ( fastfloat r ) const { return value < r.value; }
        bool operator<= ( fastfloat r ) const { return value <= r.value; }
        bool operator> ( fastfloat r ) const { return value > r.value; }
        bool operator>= ( fastfloat r ) const { return value >= r.value; }
        bool operator== ( fastfloat r ) const { return value == r.value; }
        bool operator!= ( fastfloat r ) const { return value != r.value; }
        fastfloat operator-() const { return fastfloat( -value, false ); }
    private:
        fastfloat( long v, bool ) : value( v ) {} // for operator-()
        long value;
    };

inline fastfloat operator+ ( fastfloat l, fastfloat r ) { return fastfloat( l ) += r; }
inline fastfloat operator- ( fastfloat l, fastfloat r ) { return fastfloat( l ) -= r; }
inline fastfloat operator* ( fastfloat l, fastfloat r ) { return fastfloat( l ) *= r; }
inline fastfloat operator/ ( fastfloat l, fastfloat r ) { return fastfloat( l ) /= r; }

inline bool operator< ( fastfloat l, double r ) { return l < fastfloat( r ); }
inline bool operator<= ( fastfloat l, double r ) { return l <= fastfloat( r ); }
inline bool operator> ( fastfloat l, double r ) { return l > fastfloat( r ); }
inline bool operator>= ( fastfloat l, double r ) { return l >= fastfloat( r ); }
inline bool operator== ( fastfloat l, double r ) { return l == fastfloat( r ); }
inline bool operator!= ( fastfloat l, double r ) { return l != fastfloat( r ); }

inline bool operator< ( double l, fastfloat r ) { return fastfloat( l ) < r ; }
inline bool operator<= ( double l, fastfloat r ) { return fastfloat( l ) <= r ; }
inline bool operator> ( double l, fastfloat r ) { return fastfloat( l ) > r ; }
inline bool operator>= ( double l, fastfloat r ) { return fastfloat( l ) >= r ; }
inline bool operator== ( double l, fastfloat r ) { return fastfloat( l ) == r ; }
inline bool operator!= ( double l, fastfloat r ) { return fastfloat( l ) != r ; }

inline double fasttodouble( fastfloat v ) { return v.toDouble(); }
inline long fasttolong( fastfloat v ) { return v.toLong(); }

#if 1  // change to 0 to turn fastfloat usage off
#else
#define fastfloat double
#define fasttodouble( v ) double( v )
#define fasttolong( v ) long( v )
#endif

//fastfloat end


typedef fastfloat (*Filter)(const fastfloat, const fastfloat);

typedef struct _ContributionInfo
{
  fastfloat
    weight;

  long
    pixel;
} ContributionInfo;


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeImage() scales an image to the desired dimensions with one of these
%  filters:
%
%    Bessel   Blackman   Box
%    Catrom   Cubic      Gaussian
%    Hanning  Hermite    Lanczos
%    Mitchell Point      Quandratic
%    Sinc     Triangle
%
%  Most of the filters are FIR (finite impulse response), however, Bessel,
%  Gaussian, and Sinc are IIR (infinite impulse response).  Bessel and Sinc
%  are windowed (brought down to zero) with the Blackman filter.
%
%  ResizeImage() was inspired by Paul Heckbert's zoom program.
%
%  The format of the ResizeImage method is:
%
%      Image *ResizeImage(Image *image,const unsigned long columns,
%        const unsigned long rows,const FilterTypes filter,const double blur,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o columns: The number of columns in the scaled image.
%
%    o rows: The number of rows in the scaled image.
%
%    o filter: Image filter to use.
%
%    o blur: The blur factor where > 1 is blurry, < 1 is sharp.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/

#if 0
static fastfloat Bessel(const fastfloat x,const fastfloat)
{
  if (x == 0.0)
    return(MagickPI/4.0);
  return(BesselOrderOne(MagickPI*x)/(2.0*x));
}

static fastfloat Sinc(const fastfloat x,const fastfloat)
{
  if (x == 0.0)
    return(1.0);
  return(sin(MagickPI*x)/(MagickPI*x));
}

static fastfloat Blackman(const fastfloat x,const fastfloat)
{
  return(0.42+0.5*cos(MagickPI*x)+0.08*cos(2*MagickPI*x));
}

static fastfloat BlackmanBessel(const fastfloat x,const fastfloat)
{
  return(Blackman(x/support,support)*Bessel(x,support));
}

static fastfloat BlackmanSinc(const fastfloat x,const fastfloat)
{
  return(Blackman(x/support,support)*Sinc(x,support));
}
#endif

static fastfloat Box(const fastfloat x,const fastfloat)
{
  if (x < -0.5)
    return(0.0);
  if (x < 0.5)
    return(1.0);
  return(0.0);
}

#if 0
static fastfloat Catrom(const fastfloat x,const fastfloat)
{
  if (x < -2.0)
    return(0.0);
  if (x < -1.0)
    return(0.5*(4.0+x*(8.0+x*(5.0+x))));
  if (x < 0.0)
    return(0.5*(2.0+x*x*(-5.0-3.0*x)));
  if (x < 1.0)
    return(0.5*(2.0+x*x*(-5.0+3.0*x)));
  if (x < 2.0)
    return(0.5*(4.0+x*(-8.0+x*(5.0-x))));
  return(0.0);
}

static fastfloat Cubic(const fastfloat x,const fastfloat)
{
  if (x < -2.0)
    return(0.0);
  if (x < -1.0)
    return((2.0+x)*(2.0+x)*(2.0+x)/6.0);
  if (x < 0.0)
    return((4.0+x*x*(-6.0-3.0*x))/6.0);
  if (x < 1.0)
    return((4.0+x*x*(-6.0+3.0*x))/6.0);
  if (x < 2.0)
    return((2.0-x)*(2.0-x)*(2.0-x)/6.0);
  return(0.0);
}

static fastfloat Gaussian(const fastfloat x,const fastfloat)
{
  return(exp(-2.0*x*x)*sqrt(2.0/MagickPI));
}

static fastfloat Hanning(const fastfloat x,const fastfloat)
{
  return(0.5+0.5*cos(MagickPI*x));
}

static fastfloat Hamming(const fastfloat x,const fastfloat)
{
  return(0.54+0.46*cos(MagickPI*x));
}

static fastfloat Hermite(const fastfloat x,const fastfloat)
{
  if (x < -1.0)
    return(0.0);
  if (x < 0.0)
    return((2.0*(-x)-3.0)*(-x)*(-x)+1.0);
  if (x < 1.0)
    return((2.0*x-3.0)*x*x+1.0);
  return(0.0);
}

static fastfloat Lanczos(const fastfloat x,const fastfloat support)
{
  if (x < -3.0)
    return(0.0);
  if (x < 0.0)
    return(Sinc(-x,support)*Sinc(-x/3.0,support));
  if (x < 3.0)
    return(Sinc(x,support)*Sinc(x/3.0,support));
  return(0.0);
}

static fastfloat Mitchell(const fastfloat x,const fastfloat)
{
#define B   (1.0/3.0)
#define C   (1.0/3.0)
#define P0  ((  6.0- 2.0*B       )/6.0)
#define P2  ((-18.0+12.0*B+ 6.0*C)/6.0)
#define P3  (( 12.0- 9.0*B- 6.0*C)/6.0)
#define Q0  ((       8.0*B+24.0*C)/6.0)
#define Q1  ((     -12.0*B-48.0*C)/6.0)
#define Q2  ((       6.0*B+30.0*C)/6.0)
#define Q3  ((     - 1.0*B- 6.0*C)/6.0)

  if (x < -2.0)
    return(0.0);
  if (x < -1.0)
    return(Q0-x*(Q1-x*(Q2-x*Q3)));
  if (x < 0.0)
    return(P0+x*x*(P2-x*P3));
  if (x < 1.0)
    return(P0+x*x*(P2+x*P3));
  if (x < 2.0)
    return(Q0+x*(Q1+x*(Q2+x*Q3)));
  return(0.0);

#undef B
#undef C
#undef P0
#undef P2
#undef P3
#undef Q0
#undef Q1
#undef Q2
#undef Q3
}
#endif

// this is the same like Mitchell, but it has different values
// for B and C, resulting in sharper images
// http://sourceforge.net/mailarchive/forum.php?thread_id=7445822&forum_id=1210
static fastfloat Bicubic(const fastfloat x,const fastfloat)
{
#define B   (0.0/3.0)
#define C   (2.0/3.0)
#define P0  ((  6.0- 2.0*B       )/6.0)
#define P2  ((-18.0+12.0*B+ 6.0*C)/6.0)
#define P3  (( 12.0- 9.0*B- 6.0*C)/6.0)
#define Q0  ((       8.0*B+24.0*C)/6.0)
#define Q1  ((     -12.0*B-48.0*C)/6.0)
#define Q2  ((       6.0*B+30.0*C)/6.0)
#define Q3  ((     - 1.0*B- 6.0*C)/6.0)

  if (x < -2.0)
    return(0.0);
  if (x < -1.0)
    return(Q0-x*(Q1-x*(Q2-x*Q3)));
  if (x < 0.0)
    return(P0+x*x*(P2-x*P3));
  if (x < 1.0)
    return(P0+x*x*(P2+x*P3));
  if (x < 2.0)
    return(Q0+x*(Q1+x*(Q2+x*Q3)));
  return(0.0);

#undef B
#undef C
#undef P0
#undef P2
#undef P3
#undef Q0
#undef Q1
#undef Q2
#undef Q3
}

#if 0
static fastfloat Quadratic(const fastfloat x,const fastfloat)
{
  if (x < -1.5)
    return(0.0);
  if (x < -0.5)
    return(0.5*(x+1.5)*(x+1.5));
  if (x < 0.5)
    return(0.75-x*x);
  if (x < 1.5)
    return(0.5*(x-1.5)*(x-1.5));
  return(0.0);
}
#endif

static fastfloat Triangle(const fastfloat x,const fastfloat)
{
  if (x < -1.0)
    return(0.0);
  if (x < 0.0)
    return(1.0+x);
  if (x < 1.0)
    return(1.0-x);
  return(0.0);
}

static void HorizontalFilter(const QImage& source,QImage& destination,
  const fastfloat x_factor,const fastfloat blur,
  ContributionInfo *contribution, Filter filter, fastfloat filtersupport)
{
  fastfloat
    center,
    density,
    scale,
    support;

  long
    n,
    start,
    stop,
    y;

  register long
    i,
    x;

  /*
    Apply filter to resize horizontally from source to destination.
  */
  scale=blur*Max(1.0/x_factor,1.0);
  support=scale* filtersupport;
  if (support <= 0.5)
    {
      /*
        Reduce to point sampling.
      */
      support=0.5+MagickEpsilon;
      scale=1.0;
    }
  scale=1.0/scale;
  for (x=0; x < (long) destination.width(); x++)
  {
    center=(fastfloat) (x+0.5)/x_factor;
    start= fasttolong(Max(center-support+0.5,0));
    stop= fasttolong(Min(center+support+0.5,source.width()));
    density=0.0;
    for (n=0; n < (stop-start); n++)
    {
      contribution[n].pixel=start+n;
      contribution[n].weight=
        filter (scale*(start+n-center+0.5), filtersupport );
      density+=contribution[n].weight;
    }
    if ((density != 0.0) && (density != 1.0))
      {
        /*
          Normalize.
        */
        density=1.0/density;
        for (i=0; i < n; i++)
          contribution[i].weight*=density;
      }
//    p=AcquireImagePixels(source,contribution[0].pixel,0,contribution[n-1].pixel-
//      contribution[0].pixel+1,source->rows,exception);
//    q=SetImagePixels(destination,x,0,1,destination->rows);
    for (y=0; y < (long) destination.height(); y++)
    {
      fastfloat red = 0;
      fastfloat green = 0;
      fastfloat blue = 0;
      fastfloat alpha = 0;
      for (i=0; i < n; i++)
      {
        int px = contribution[i].pixel;
        int py = y;
        QRgb p = reinterpret_cast< QRgb* >( source.jumpTable()[ py ])[ px ];
        red+=contribution[i].weight*qRed(p);
        green+=contribution[i].weight*qGreen(p);
        blue+=contribution[i].weight*qBlue(p);
        alpha+=contribution[i].weight*qAlpha(p);
      }
      QRgb pix = qRgba(
          fasttolong( red < 0 ? 0 : red > 255 ? 255 : red + 0.5 ),
          fasttolong( green < 0 ? 0 : green > 255 ? 255 : green + 0.5 ),
          fasttolong( blue < 0 ? 0 : blue > 255 ? 255 : blue + 0.5 ),
          fasttolong( alpha < 0 ? 0 : alpha > 255 ? 255 : alpha + 0.5 ));
      reinterpret_cast< QRgb* >( destination.jumpTable()[ y ])[ x ] = pix;
    }
  }
}

static void VerticalFilter(const QImage& source,QImage& destination,
  const fastfloat y_factor,const fastfloat blur,
  ContributionInfo *contribution, Filter filter, fastfloat filtersupport )
{
  fastfloat
    center,
    density,
    scale,
    support;

  long
    n,
    start,
    stop,
    x;

  register long
    i,
    y;

  /*
    Apply filter to resize vertically from source to destination.
  */
  scale=blur*Max(1.0/y_factor,1.0);
  support=scale* filtersupport;
  if (support <= 0.5)
    {
      /*
        Reduce to point sampling.
      */
      support=0.5+MagickEpsilon;
      scale=1.0;
    }
  scale=1.0/scale;
  for (y=0; y < (long) destination.height(); y++)
  {
    center=(fastfloat) (y+0.5)/y_factor;
    start= fasttolong(Max(center-support+0.5,0));
    stop= fasttolong(Min(center+support+0.5,source.height()));
    density=0.0;
    for (n=0; n < (stop-start); n++)
    {
      contribution[n].pixel=start+n;
      contribution[n].weight=
        filter (scale*(start+n-center+0.5), filtersupport);
      density+=contribution[n].weight;
    }
    if ((density != 0.0) && (density != 1.0))
      {
        /*
          Normalize.
        */
        density=1.0/density;
        for (i=0; i < n; i++)
          contribution[i].weight*=density;
      }
//    p=AcquireImagePixels(source,0,contribution[0].pixel,source->columns,
//      contribution[n-1].pixel-contribution[0].pixel+1,exception);
//    q=SetImagePixels(destination,0,y,destination->columns,1);
    for (x=0; x < (long) destination.width(); x++)
    {
      fastfloat red = 0;
      fastfloat green = 0;
      fastfloat blue = 0;
      fastfloat alpha = 0;
      for (i=0; i < n; i++)
      {
        int px = x;
        int py = contribution[i].pixel;
        QRgb p = reinterpret_cast< QRgb* >( source.jumpTable()[ py ])[ px ];
        red+=contribution[i].weight*qRed(p);
        green+=contribution[i].weight*qGreen(p);
        blue+=contribution[i].weight*qBlue(p);
        alpha+=contribution[i].weight*qAlpha(p);
      }
      QRgb pix = qRgba(
          fasttolong( red < 0 ? 0 : red > 255 ? 255 : red + 0.5 ),
          fasttolong( green < 0 ? 0 : green > 255 ? 255 : green + 0.5 ),
          fasttolong( blue < 0 ? 0 : blue > 255 ? 255 : blue + 0.5 ),
          fasttolong( alpha < 0 ? 0 : alpha > 255 ? 255 : alpha + 0.5 ));
      reinterpret_cast< QRgb* >( destination.jumpTable()[ y ])[ x ] = pix;
    }
  }
}

static QImage ResizeImage(const QImage& image,const int columns,
  const int rows, Filter filter, fastfloat filtersupport, double blur)
{
  ContributionInfo
    *contribution;

  fastfloat
    support,
    x_factor,
    x_support,
    y_factor,
    y_support;

  /*
    Initialize resize image attributes.
  */
  if ((columns == image.width()) && (rows == image.height()) && (blur == 1.0))
    return image.copy();
  QImage resize_image( columns, rows, 32 );
  resize_image.setAlphaBuffer( image.hasAlphaBuffer());
  /*
    Allocate filter contribution info.
  */
  x_factor=(fastfloat) resize_image.width()/image.width();
  y_factor=(fastfloat) resize_image.height()/image.height();
//  i=(long) LanczosFilter;
//  if (image->filter != UndefinedFilter)
//    i=(long) image->filter;
//  else
//    if ((image->storage_class == PseudoClass) || image->matte ||
//        ((x_factor*y_factor) > 1.0))
//      i=(long) MitchellFilter;
  x_support=blur*Max(1.0/x_factor,1.0)*filtersupport;
  y_support=blur*Max(1.0/y_factor,1.0)*filtersupport;
  support=Max(x_support,y_support);
  if (support < filtersupport)
    support=filtersupport;
  contribution=new ContributionInfo[ fasttolong( 2.0*Max(support,0.5)+3 ) ];
  Q_CHECK_PTR( contribution );
  /*
    Resize image.
  */
  if (((fastfloat) columns*(image.height()+rows)) >
      ((fastfloat) rows*(image.width()+columns)))
    {
      QImage source_image( columns, image.height(), 32 );
      source_image.setAlphaBuffer( image.hasAlphaBuffer());
      HorizontalFilter(image,source_image,x_factor,blur,
        contribution,filter,filtersupport);
      VerticalFilter(source_image,resize_image,y_factor,
        blur,contribution,filter,filtersupport);
    }
  else
    {
      QImage source_image( image.width(), rows, 32 );
      source_image.setAlphaBuffer( image.hasAlphaBuffer());
      VerticalFilter(image,source_image,y_factor,blur,
        contribution,filter,filtersupport);
      HorizontalFilter(source_image,resize_image,x_factor,
        blur,contribution,filter,filtersupport);
    }
  /*
    Free allocated memory.
  */
  delete[] contribution;
  return(resize_image);
}


#undef Max
#undef Min
#undef MagickEpsilon


// filters and their matching support values
#if 0
  static const FilterInfo
    filters[SincFilter+1] =
    {
      { Box, 0.0 },
      { Box, 0.0 },
      { Box, 0.5 },
      { Triangle, 1.0 },
      { Hermite, 1.0 },
      { Hanning, 1.0 },
      { Hamming, 1.0 },
      { Blackman, 1.0 },
      { Gaussian, 1.25 },
      { Quadratic, 1.5 },
      { Cubic, 2.0 },
      { Catrom, 2.0 },
      { Mitchell, 2.0 },
      { Lanczos, 3.0 },
      { BlackmanBessel, 3.2383 },
      { BlackmanSinc, 4.0 }
    };
#endif


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S a m p l e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SampleImage() scales an image to the desired dimensions with pixel
%  sampling.  Unlike other scaling methods, this method does not introduce
%  any additional color into the scaled image.
%
%  The format of the SampleImage method is:
%
%      Image *SampleImage(const Image *image,const unsigned long columns,
%        const unsigned long rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o columns: The number of columns in the sampled image.
%
%    o rows: The number of rows in the sampled image.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
QImage SampleImage(const QImage& image,const int columns,
  const int rows)
{
  int
    *x_offset,
    *y_offset;

  long
    j,
    y;

  uchar
    *pixels;

  register const uchar
    *p;

  register long
    x;

  register uchar
    *q;

  /*
    Initialize sampled image attributes.
  */
  if ((columns == image.width()) && (rows == image.height()))
    return image;
  // This function is modified to handle any image depth, not only
  // 32bit like the ImageMagick original. This avoids the relatively
  // expensive conversion.
  const int d = image.depth() / 8;
  QImage sample_image( columns, rows, image.depth());
  sample_image.setAlphaBuffer( image.hasAlphaBuffer());
  /*
    Allocate scan line buffer and column offset buffers.
  */
  pixels= new uchar[ image.width() * d ];
  x_offset= new int[ sample_image.width() ];
  y_offset= new int[ sample_image.height() ];
  /*
    Initialize pixel offsets.
  */
// In the following several code 0.5 needs to be added, otherwise the image
// would be moved by half a pixel to bottom-right, just like
// with Qt's QImage::scale()
  for (x=0; x < (long) sample_image.width(); x++)
  {
    x_offset[x]=int((x+0.5)*image.width()/sample_image.width());
  }
  for (y=0; y < (long) sample_image.height(); y++)
  {
    y_offset[y]=int((y+0.5)*image.height()/sample_image.height());
  }
  /*
    Sample each row.
  */
  j=(-1);
  for (y=0; y < (long) sample_image.height(); y++)
  {
    q= sample_image.scanLine( y );
    if (j != y_offset[y] )
      {
        /*
          Read a scan line.
        */
        j= y_offset[y];
        p= image.scanLine( j );
        (void) memcpy(pixels,p,image.width()*d);
      }
    /*
      Sample each column.
    */
    switch( d )
    {
    case 1: // 8bit
      for (x=0; x < (long) sample_image.width(); x++)
      {
        *q++=pixels[ x_offset[x] ];
      }
      break;
    case 4: // 32bit
      for (x=0; x < (long) sample_image.width(); x++)
      {
        *(QRgb*)q=((QRgb*)pixels)[ x_offset[x] ];
        q += d;
      }
      break;
    default:
      for (x=0; x < (long) sample_image.width(); x++)
      {
        memcpy( q, pixels + x_offset[x] * d, d );
        q += d;
      }
      break;
    }
  }
  if( d != 4 ) // != 32bit
  {
    sample_image.setNumColors( image.numColors());
    for( int i = 0; i < image.numColors(); ++i )
      sample_image.setColor( i, image.color( i ));
  }
  delete[] y_offset;
  delete[] x_offset;
  delete[] pixels;
  return sample_image;
}


// ImageMagick code end


// Imlib2/Mosfet code begin
// ------------------------

// This code is Imlib2 code, additionally modified by Mosfet, and with few small
// modifications for Gwenview. The MMX scaling code also belongs to it.

// The original license texts follow.

/**
 * This is the normal smoothscale method, based on Imlib2's smoothscale.
 *
 * Originally I took the algorithm used in NetPBM and Qt and added MMX/3dnow
 * optimizations. It ran in about 1/2 the time as Qt. Then I ported Imlib's
 * C algorithm and it ran at about the same speed as my MMX optimized one...
 * Finally I ported Imlib's MMX version and it ran in less than half the
 * time as my MMX algorithm, (taking only a quarter of the time Qt does).
 *
 * Changes include formatting, namespaces and other C++'ings, removal of old
 * #ifdef'ed code, and removal of unneeded border calculation code.
 *
 * Imlib2 is (C) Carsten Haitzler and various contributors. The MMX code
 * is by Willem Monsuwe <willem@stack.nl>. All other modifications are
 * (C) Daniel M. Duley.
 */

/*
    Copyright (C) 2004 Daniel M. Duley <dan.duley@verizon.net>

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
Copyright (C) 2000 Carsten Haitzler and various contributors (see AUTHORS)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its Copyright notices. In addition publicly
documented acknowledgment must be given that this software has been used if no
source code of this software is made available publicly. This includes
acknowledgments in either Copyright notices, Manuals, Publicity and Marketing
documents or any documentation provided with any product containing this
software. This License does not apply to any software that links to the
libraries provided by this software (statically or dynamically), but only to
the software provided.

Please see the COPYING.PLAIN for a plain-english explanation of this notice
and it's intent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

namespace MImageScale{
    typedef struct __mimage_scale_info
    {
        int *xpoints;
        unsigned int **ypoints;
        int *xapoints, *yapoints;
        int xup_yup;
    } MImageScaleInfo;

    unsigned int** mimageCalcYPoints(unsigned int *src, int sow, int sh,
                                     int dh);
    int* mimageCalcXPoints(int sw, int dw);
    int* mimageCalcApoints(int s, int d, int up);
    MImageScaleInfo* mimageFreeScaleInfo(MImageScaleInfo *isi);
    MImageScaleInfo *mimageCalcScaleInfo(QImage &img, int sw, int sh,
                                         int dw, int dh, char aa, int sow);
    void mimageSampleRGBA(MImageScaleInfo *isi, unsigned int *dest, int dxx,
                          int dyy, int dx, int dy, int dw, int dh, int dow);
    void mimageScaleAARGBA(MImageScaleInfo *isi, unsigned int *dest, int dxx,
                           int dyy, int dx, int dy, int dw, int dh, int dow,
                           int sow);
    void mimageScaleAARGB(MImageScaleInfo *isi, unsigned int *dest, int dxx,
                          int dyy, int dx, int dy, int dw, int dh, int dow, int
                          sow);
    QImage smoothScale(const QImage& img, int dw, int dh);
}

#ifdef HAVE_X86_MMX
extern "C" {
    void __mimageScale_mmx_AARGBA(MImageScale::MImageScaleInfo *isi,
                                  unsigned int *dest, int dxx, int dyy,
                                  int dx, int dy, int dw, int dh,
                                  int dow, int sow);
}
#endif

using namespace MImageScale;

QImage MImageScale::smoothScale(const QImage& image, int dw, int dh)
{
    QImage img = image.depth() < 32 ? image.convertDepth( 32 ) : image;
    int w = img.width();
    int h = img.height();

    int sow = img.bytesPerLine();
    // handle CroppedQImage
    if( img.height() > 1 && sow != img.scanLine( 1 ) - img.scanLine( 0 ))
        sow = img.scanLine( 1 ) - img.scanLine( 0 );
    sow = sow / ( img.depth() / 8 );

    MImageScaleInfo *scaleinfo =
        mimageCalcScaleInfo(img, w, h, dw, dh, true, sow);
    if(!scaleinfo)
        return QImage();

    QImage buffer(dw, dh, 32);
    buffer.setAlphaBuffer(img.hasAlphaBuffer());

#ifdef HAVE_X86_MMX
//#warning Using MMX Smoothscale
    bool haveMMX = KCPUInfo::haveExtension( KCPUInfo::IntelMMX );
    if(haveMMX){
        __mimageScale_mmx_AARGBA(scaleinfo, (unsigned int *)buffer.scanLine(0),
                                 0, 0, 0, 0, dw, dh, dw, sow);
    }
    else
#endif
    {
        if(img.hasAlphaBuffer())
            mimageScaleAARGBA(scaleinfo, (unsigned int *)buffer.scanLine(0), 0, 0,
                              0, 0, dw, dh, dw, sow);
        else
            mimageScaleAARGB(scaleinfo, (unsigned int *)buffer.scanLine(0), 0, 0,
                             0, 0, dw, dh, dw, sow);
    }
    mimageFreeScaleInfo(scaleinfo);
    return(buffer);
}

//
// Code ported from Imlib...
//

// FIXME: replace with mRed, etc... These work on pointers to pixels, not
// pixel values
#if BYTE_ORDER == BIG_ENDIAN
#define A_VAL(p) ((unsigned char *)(p))[0]
#define R_VAL(p) ((unsigned char *)(p))[1]
#define G_VAL(p) ((unsigned char *)(p))[2]
#define B_VAL(p) ((unsigned char *)(p))[3]
#elif BYTE_ORDER == LITTLE_ENDIAN
#define A_VAL(p) ((unsigned char *)(p))[3]
#define R_VAL(p) ((unsigned char *)(p))[2]
#define G_VAL(p) ((unsigned char *)(p))[1]
#define B_VAL(p) ((unsigned char *)(p))[0]
#else
#error "BYTE_ORDER is not defined"
#endif

#define INV_XAP                   (256 - xapoints[x])
#define XAP                       (xapoints[x])
#define INV_YAP                   (256 - yapoints[dyy + y])
#define YAP                       (yapoints[dyy + y])

unsigned int** MImageScale::mimageCalcYPoints(unsigned int *src,
                                              int sow, int sh, int dh)
{
    unsigned int **p;
    int i, j = 0;
    int val, inc, rv = 0;

    if(dh < 0){
        dh = -dh;
        rv = 1;
    }
    p = new unsigned int* [dh+1];

    val = 0;
    inc = (sh << 16) / dh;
    for(i = 0; i < dh; i++){
        p[j++] = src + ((val >> 16) * sow);
        val += inc;
    }
    if(rv){
        for(i = dh / 2; --i >= 0; ){
            unsigned int *tmp = p[i];
            p[i] = p[dh - i - 1];
            p[dh - i - 1] = tmp;
        }
    }
    return(p);
}

int* MImageScale::mimageCalcXPoints(int sw, int dw)
{
    int *p, i, j = 0;
    int val, inc, rv = 0;

    if(dw < 0){
        dw = -dw;
        rv = 1;
    }
    p = new int[dw+1];

    val = 0;
    inc = (sw << 16) / dw;
    for(i = 0; i < dw; i++){
        p[j++] = (val >> 16);
        val += inc;
    }

    if(rv){
        for(i = dw / 2; --i >= 0; ){
            int tmp = p[i];
            p[i] = p[dw - i - 1];
            p[dw - i - 1] = tmp;
        }
    }
   return(p);
}

int* MImageScale::mimageCalcApoints(int s, int d, int up)
{
    int *p, i, j = 0, rv = 0;

    if(d < 0){
        rv = 1;
        d = -d;
    }
    p = new int[d];

    /* scaling up */
    if(up){
        int val, inc;

        val = 0;
        inc = (s << 16) / d;
        for(i = 0; i < d; i++){
            p[j++] = (val >> 8) - ((val >> 8) & 0xffffff00);
            if((val >> 16) >= (s - 1))
                p[j - 1] = 0;
            val += inc;
        }
    }
    /* scaling down */
    else{
        int val, inc, ap, Cp;
        val = 0;
        inc = (s << 16) / d;
        Cp = ((d << 14) / s) + 1;
        for(i = 0; i < d; i++){
            ap = ((0x100 - ((val >> 8) & 0xff)) * Cp) >> 8;
            p[j] = ap | (Cp << 16);
            j++;
            val += inc;
        }
    }
    if(rv){
        int tmp;
        for(i = d / 2; --i >= 0; ){
            tmp = p[i];
            p[i] = p[d - i - 1];
            p[d - i - 1] = tmp;
        }
    }
    return(p);
}

MImageScaleInfo* MImageScale::mimageFreeScaleInfo(MImageScaleInfo *isi)
{
    if(isi){
        delete[] isi->xpoints;
        delete[] isi->ypoints;
        delete[] isi->xapoints;
        delete[] isi->yapoints;
        delete isi;
    }
    return(NULL);
}

MImageScaleInfo* MImageScale::mimageCalcScaleInfo(QImage &img, int sw, int sh,
                                                  int dw, int dh, char aa, int sow)
{
    MImageScaleInfo *isi;
    int scw, sch;

    scw = dw * img.width() / sw;
    sch = dh * img.height() / sh;

    isi = new MImageScaleInfo;
    if(!isi)
        return(NULL);
    memset(isi, 0, sizeof(MImageScaleInfo));

    isi->xup_yup = (abs(dw) >= sw) + ((abs(dh) >= sh) << 1);

    isi->xpoints = mimageCalcXPoints(img.width(), scw);
    if(!isi->xpoints)
        return(mimageFreeScaleInfo(isi));
    isi->ypoints = mimageCalcYPoints((unsigned int *)img.scanLine(0),
                                     sow, img.height(), sch );
    if (!isi->ypoints)
        return(mimageFreeScaleInfo(isi));
    if(aa){
        isi->xapoints = mimageCalcApoints(img.width(), scw, isi->xup_yup & 1);
        if(!isi->xapoints)
            return(mimageFreeScaleInfo(isi));
        isi->yapoints = mimageCalcApoints(img.height(), sch, isi->xup_yup & 2);
        if(!isi->yapoints)
            return(mimageFreeScaleInfo(isi));
    }
    return(isi);
}

/* scale by pixel sampling only */
void MImageScale::mimageSampleRGBA(MImageScaleInfo *isi, unsigned int *dest,
                                   int dxx, int dyy, int dx, int dy, int dw,
                                   int dh, int dow)
{
    unsigned int *sptr, *dptr;
    int x, y, end;
    unsigned int **ypoints = isi->ypoints;
    int *xpoints = isi->xpoints;

    /* whats the last pixel ont he line so we stop there */
    end = dxx + dw;
    /* go through every scanline in the output buffer */
    for(y = 0; y < dh; y++){
        /* get the pointer to the start of the destination scanline */
        dptr = dest + dx + ((y + dy) * dow);
        /* calculate the source line we'll scan from */
        sptr = ypoints[dyy + y];
        /* go thru the scanline and copy across */
        for(x = dxx; x < end; x++)
            *dptr++ = sptr[xpoints[x]];
    }
}

/* FIXME: NEED to optimise ScaleAARGBA - currently its "ok" but needs work*/

/* scale by area sampling */
void MImageScale::mimageScaleAARGBA(MImageScaleInfo *isi, unsigned int *dest,
                                    int dxx, int dyy, int dx, int dy, int dw,
                                    int dh, int dow, int sow)
{
    unsigned int *sptr, *dptr;
    int x, y, end;
    unsigned int **ypoints = isi->ypoints;
    int *xpoints = isi->xpoints;
    int *xapoints = isi->xapoints;
    int *yapoints = isi->yapoints;

    end = dxx + dw;
    /* scaling up both ways */
    if(isi->xup_yup == 3){
        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            /* calculate the source line we'll scan from */
            dptr = dest + dx + ((y + dy) * dow);
            sptr = ypoints[dyy + y];
            if(YAP > 0){
                for(x = dxx; x < end; x++){
                    int r, g, b, a;
                    int rr, gg, bb, aa;
                    unsigned int *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_XAP;
                        g = G_VAL(pix) * INV_XAP;
                        b = B_VAL(pix) * INV_XAP;
                        a = A_VAL(pix) * INV_XAP;
                        pix++;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        a += A_VAL(pix) * XAP;
                        pix += sow;
                        rr = R_VAL(pix) * XAP;
                        gg = G_VAL(pix) * XAP;
                        bb = B_VAL(pix) * XAP;
                        aa = A_VAL(pix) * XAP;
                        pix--;
                        rr += R_VAL(pix) * INV_XAP;
                        gg += G_VAL(pix) * INV_XAP;
                        bb += B_VAL(pix) * INV_XAP;
                        aa += A_VAL(pix) * INV_XAP;
                        r = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b = ((bb * YAP) + (b * INV_YAP)) >> 16;
                        a = ((aa * YAP) + (a * INV_YAP)) >> 16;
                        *dptr++ = qRgba(r, g, b, a);
                    }
                    else{
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_YAP;
                        g = G_VAL(pix) * INV_YAP;
                        b = B_VAL(pix) * INV_YAP;
                        a = A_VAL(pix) * INV_YAP;
                        pix += sow;
                        r += R_VAL(pix) * YAP;
                        g += G_VAL(pix) * YAP;
                        b += B_VAL(pix) * YAP;
                        a += A_VAL(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;
                        *dptr++ = qRgba(r, g, b, a);
                    }
                }
            }
            else{
                for(x = dxx; x < end; x++){
                    int r, g, b, a;
                    unsigned int *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_XAP;
                        g = G_VAL(pix) * INV_XAP;
                        b = B_VAL(pix) * INV_XAP;
                        a = A_VAL(pix) * INV_XAP;
                        pix++;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        a += A_VAL(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        a >>= 8;
                        *dptr++ = qRgba(r, g, b, a);
                    }
                    else
                        *dptr++ = sptr[xpoints[x] ];
                }
            }
        }
    }
    /* if we're scaling down vertically */
    else if(isi->xup_yup == 1){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cy, j;
        unsigned int *pix;
        int r, g, b, a, rr, gg, bb, aa;
        int yap;
		 
        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL(pix) * yap) >> 10;
                g = (G_VAL(pix) * yap) >> 10;
                b = (B_VAL(pix) * yap) >> 10;
                a = (A_VAL(pix) * yap) >> 10;
                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix += sow;
                    r += (R_VAL(pix) * Cy) >> 10;
                    g += (G_VAL(pix) * Cy) >> 10;
                    b += (B_VAL(pix) * Cy) >> 10;
                    a += (A_VAL(pix) * Cy) >> 10;
                }
                if(j > 0){
                    pix += sow;
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                    a += (A_VAL(pix) * j) >> 10;
                }
                if(XAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr = (R_VAL(pix) * yap) >> 10;
                    gg = (G_VAL(pix) * yap) >> 10;
                    bb = (B_VAL(pix) * yap) >> 10;
                    aa = (A_VAL(pix) * yap) >> 10;
                    for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                        pix += sow;
                        rr += (R_VAL(pix) * Cy) >> 10;
                        gg += (G_VAL(pix) * Cy) >> 10;
                        bb += (B_VAL(pix) * Cy) >> 10;
                        aa += (A_VAL(pix) * Cy) >> 10;
                    }
                    if(j > 0){
                        pix += sow;
                        rr += (R_VAL(pix) * j) >> 10;
                        gg += (G_VAL(pix) * j) >> 10;
                        bb += (B_VAL(pix) * j) >> 10;
                        aa += (A_VAL(pix) * j) >> 10;
                    }
                    r = r * INV_XAP;
                    g = g * INV_XAP;
                    b = b * INV_XAP;
                    a = a * INV_XAP;
                    r = (r + ((rr * XAP))) >> 12;
                    g = (g + ((gg * XAP))) >> 12;
                    b = (b + ((bb * XAP))) >> 12;
                    a = (a + ((aa * XAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }
                *dptr = qRgba(r, g, b, a);
                dptr++;
            }
        }
    }
    /* if we're scaling down horizontally */
    else if(isi->xup_yup == 2){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, j;
        unsigned int *pix;
        int r, g, b, a, rr, gg, bb, aa;
        int xap;

        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL(pix) * xap) >> 10;
                g = (G_VAL(pix) * xap) >> 10;
                b = (B_VAL(pix) * xap) >> 10;
                a = (A_VAL(pix) * xap) >> 10;
                for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                    pix++;
                    r += (R_VAL(pix) * Cx) >> 10;
                    g += (G_VAL(pix) * Cx) >> 10;
                    b += (B_VAL(pix) * Cx) >> 10;
                    a += (A_VAL(pix) * Cx) >> 10;
                }
                if(j > 0){
                    pix++;
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                    a += (A_VAL(pix) * j) >> 10;
                }
                if(YAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr = (R_VAL(pix) * xap) >> 10;
                    gg = (G_VAL(pix) * xap) >> 10;
                    bb = (B_VAL(pix) * xap) >> 10;
                    aa = (A_VAL(pix) * xap) >> 10;
                    for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                        pix++;
                        rr += (R_VAL(pix) * Cx) >> 10;
                        gg += (G_VAL(pix) * Cx) >> 10;
                        bb += (B_VAL(pix) * Cx) >> 10;
                        aa += (A_VAL(pix) * Cx) >> 10;
                    }
                    if(j > 0){
                        pix++;
                        rr += (R_VAL(pix) * j) >> 10;
                        gg += (G_VAL(pix) * j) >> 10;
                        bb += (B_VAL(pix) * j) >> 10;
                        aa += (A_VAL(pix) * j) >> 10;
                    }
                    r = r * INV_YAP;
                    g = g * INV_YAP;
                    b = b * INV_YAP;
                    a = a * INV_YAP;
                    r = (r + ((rr * YAP))) >> 12;
                    g = (g + ((gg * YAP))) >> 12;
                    b = (b + ((bb * YAP))) >> 12;
                    a = (a + ((aa * YAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                    a >>= 4;
                }
                *dptr = qRgba(r, g, b, a);
                dptr++;
            }
        }
    }
    /* if we're scaling down horizontally & vertically */
    else{
        /*\ 'Correct' version, with math units prepared for MMXification:
         |*|  The operation 'b = (b * c) >> 16' translates to pmulhw,
         |*|  so the operation 'b = (b * c) >> d' would translate to
         |*|  psllw (16 - d), %mmb; pmulh %mmc, %mmb
         \*/
        int Cx, Cy, i, j;
        unsigned int *pix;
        int a, r, g, b, ax, rx, gx, bx;
        int xap, yap;

        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                sptr = ypoints[dyy + y] + xpoints[x];
                pix = sptr;
                sptr += sow;
                rx = (R_VAL(pix) * xap) >> 9;
                gx = (G_VAL(pix) * xap) >> 9;
                bx = (B_VAL(pix) * xap) >> 9;
                ax = (A_VAL(pix) * xap) >> 9;
                pix++;
                for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                    rx += (R_VAL(pix) * Cx) >> 9;
                    gx += (G_VAL(pix) * Cx) >> 9;
                    bx += (B_VAL(pix) * Cx) >> 9;
                    ax += (A_VAL(pix) * Cx) >> 9;
                    pix++;
                }
                if(i > 0){
                    rx += (R_VAL(pix) * i) >> 9;
                    gx += (G_VAL(pix) * i) >> 9;
                    bx += (B_VAL(pix) * i) >> 9;
                    ax += (A_VAL(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;
                a = (ax * yap) >> 14;

                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL(pix) * xap) >> 9;
                    gx = (G_VAL(pix) * xap) >> 9;
                    bx = (B_VAL(pix) * xap) >> 9;
                    ax = (A_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        ax += (A_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                        ax += (A_VAL(pix) * i) >> 9;
                    }

                    r += (rx * Cy) >> 14;
                    g += (gx * Cy) >> 14;
                    b += (bx * Cy) >> 14;
                    a += (ax * Cy) >> 14;
                }
                if(j > 0){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL(pix) * xap) >> 9;
                    gx = (G_VAL(pix) * xap) >> 9;
                    bx = (B_VAL(pix) * xap) >> 9;
                    ax = (A_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        ax += (A_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                        ax += (A_VAL(pix) * i) >> 9;
                    }

                    r += (rx * j) >> 14;
                    g += (gx * j) >> 14;
                    b += (bx * j) >> 14;
                    a += (ax * j) >> 14;
                }

                R_VAL(dptr) = r >> 5;
                G_VAL(dptr) = g >> 5;
                B_VAL(dptr) = b >> 5;
                A_VAL(dptr) = a >> 5;
                dptr++;
            }
        }
    }
}

/* scale by area sampling - IGNORE the ALPHA byte*/
void MImageScale::mimageScaleAARGB(MImageScaleInfo *isi, unsigned int *dest,
                                   int dxx, int dyy, int dx, int dy, int dw,
                                   int dh, int dow, int sow)
{
    unsigned int *sptr, *dptr;
    int x, y, end;
    unsigned int **ypoints = isi->ypoints;
    int *xpoints = isi->xpoints;
    int *xapoints = isi->xapoints;
    int *yapoints = isi->yapoints;

    end = dxx + dw;
    /* scaling up both ways */
    if(isi->xup_yup == 3){
        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            /* calculate the source line we'll scan from */
            dptr = dest + dx + ((y + dy) * dow);
            sptr = ypoints[dyy + y];
            if(YAP > 0){
                for(x = dxx; x < end; x++){
                    int r = 0, g = 0, b = 0;
                    int rr = 0, gg = 0, bb = 0;
                    unsigned int *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_XAP;
                        g = G_VAL(pix) * INV_XAP;
                        b = B_VAL(pix) * INV_XAP;
                        pix++;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        pix += sow;
                        rr = R_VAL(pix) * XAP;
                        gg = G_VAL(pix) * XAP;
                        bb = B_VAL(pix) * XAP;
                        pix --;
                        rr += R_VAL(pix) * INV_XAP;
                        gg += G_VAL(pix) * INV_XAP;
                        bb += B_VAL(pix) * INV_XAP;
                        r = ((rr * YAP) + (r * INV_YAP)) >> 16;
                        g = ((gg * YAP) + (g * INV_YAP)) >> 16;
                        b = ((bb * YAP) + (b * INV_YAP)) >> 16;
                        *dptr++ = qRgba(r, g, b, 0xff);
                    }
                    else{
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_YAP;
                        g = G_VAL(pix) * INV_YAP;
                        b = B_VAL(pix) * INV_YAP;
                        pix += sow;
                        r += R_VAL(pix) * YAP;
                        g += G_VAL(pix) * YAP;
                        b += B_VAL(pix) * YAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        *dptr++ = qRgba(r, g, b, 0xff);
                    }
                }
            }
            else{
                for(x = dxx; x < end; x++){
                    int r = 0, g = 0, b = 0;
                    unsigned int *pix;

                    if(XAP > 0){
                        pix = ypoints[dyy + y] + xpoints[x];
                        r = R_VAL(pix) * INV_XAP;
                        g = G_VAL(pix) * INV_XAP;
                        b = B_VAL(pix) * INV_XAP;
                        pix++;
                        r += R_VAL(pix) * XAP;
                        g += G_VAL(pix) * XAP;
                        b += B_VAL(pix) * XAP;
                        r >>= 8;
                        g >>= 8;
                        b >>= 8;
                        *dptr++ = qRgba(r, g, b, 0xff);
                    }
                    else
                        *dptr++ = sptr[xpoints[x] ];
                }
            }
        }
    }
    /* if we're scaling down vertically */
    else if(isi->xup_yup == 1){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cy, j;
        unsigned int *pix;
        int r, g, b, rr, gg, bb;
        int yap;

        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL(pix) * yap) >> 10;
                g = (G_VAL(pix) * yap) >> 10;
                b = (B_VAL(pix) * yap) >> 10;
                pix += sow;
                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    r += (R_VAL(pix) * Cy) >> 10;
                    g += (G_VAL(pix) * Cy) >> 10;
                    b += (B_VAL(pix) * Cy) >> 10;
                    pix += sow;
                }
                if(j > 0){
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                }
                if(XAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + 1;
                    rr = (R_VAL(pix) * yap) >> 10;
                    gg = (G_VAL(pix) * yap) >> 10;
                    bb = (B_VAL(pix) * yap) >> 10;
                    pix += sow;
                    for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                        rr += (R_VAL(pix) * Cy) >> 10;
                        gg += (G_VAL(pix) * Cy) >> 10;
                        bb += (B_VAL(pix) * Cy) >> 10;
                        pix += sow;
                    }
                    if(j > 0){
                        rr += (R_VAL(pix) * j) >> 10;
                        gg += (G_VAL(pix) * j) >> 10;
                        bb += (B_VAL(pix) * j) >> 10;
                    }
                    r = r * INV_XAP;
                    g = g * INV_XAP;
                    b = b * INV_XAP;
                    r = (r + ((rr * XAP))) >> 12;
                    g = (g + ((gg * XAP))) >> 12;
                    b = (b + ((bb * XAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }
                *dptr = qRgba(r, g, b, 0xff);
                dptr++;
            }
        }
    }
    /* if we're scaling down horizontally */
    else if(isi->xup_yup == 2){
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, j;
        unsigned int *pix;
        int r, g, b, rr, gg, bb;
        int xap;

        /* go through every scanline in the output buffer */
        for(y = 0; y < dh; y++){
            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                pix = ypoints[dyy + y] + xpoints[x];
                r = (R_VAL(pix) * xap) >> 10;
                g = (G_VAL(pix) * xap) >> 10;
                b = (B_VAL(pix) * xap) >> 10;
                pix++;
                for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                    r += (R_VAL(pix) * Cx) >> 10;
                    g += (G_VAL(pix) * Cx) >> 10;
                    b += (B_VAL(pix) * Cx) >> 10;
                    pix++;
                }
                if(j > 0){
                    r += (R_VAL(pix) * j) >> 10;
                    g += (G_VAL(pix) * j) >> 10;
                    b += (B_VAL(pix) * j) >> 10;
                }
                if(YAP > 0){
                    pix = ypoints[dyy + y] + xpoints[x] + sow;
                    rr = (R_VAL(pix) * xap) >> 10;
                    gg = (G_VAL(pix) * xap) >> 10;
                    bb = (B_VAL(pix) * xap) >> 10;
                    pix++;
                    for(j = (1 << 14) - xap; j > Cx; j -= Cx){
                        rr += (R_VAL(pix) * Cx) >> 10;
                        gg += (G_VAL(pix) * Cx) >> 10;
                        bb += (B_VAL(pix) * Cx) >> 10;
                        pix++;
                    }
                    if(j > 0){
                        rr += (R_VAL(pix) * j) >> 10;
                        gg += (G_VAL(pix) * j) >> 10;
                        bb += (B_VAL(pix) * j) >> 10;
                    }
                    r = r * INV_YAP;
                    g = g * INV_YAP;
                    b = b * INV_YAP;
                    r = (r + ((rr * YAP))) >> 12;
                    g = (g + ((gg * YAP))) >> 12;
                    b = (b + ((bb * YAP))) >> 12;
                }
                else{
                    r >>= 4;
                    g >>= 4;
                    b >>= 4;
                }
                *dptr = qRgba(r, g, b, 0xff);
                dptr++;
            }
        }
    }
    /* fully optimized (i think) - onyl change of algorithm can help */
    /* if we're scaling down horizontally & vertically */
    else{
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, Cy, i, j;
        unsigned int *pix;
        int r, g, b, rx, gx, bx;
        int xap, yap;

        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                sptr = ypoints[dyy + y] + xpoints[x];
                pix = sptr;
                sptr += sow;
                rx = (R_VAL(pix) * xap) >> 9;
                gx = (G_VAL(pix) * xap) >> 9;
                bx = (B_VAL(pix) * xap) >> 9;
                pix++;
                for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                    rx += (R_VAL(pix) * Cx) >> 9;
                    gx += (G_VAL(pix) * Cx) >> 9;
                    bx += (B_VAL(pix) * Cx) >> 9;
                    pix++;
                }
                if(i > 0){
                    rx += (R_VAL(pix) * i) >> 9;
                    gx += (G_VAL(pix) * i) >> 9;
                    bx += (B_VAL(pix) * i) >> 9;
                }

                r = (rx * yap) >> 14;
                g = (gx * yap) >> 14;
                b = (bx * yap) >> 14;

                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL(pix) * xap) >> 9;
                    gx = (G_VAL(pix) * xap) >> 9;
                    bx = (B_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                    }

                    r += (rx * Cy) >> 14;
                    g += (gx * Cy) >> 14;
                    b += (bx * Cy) >> 14;
                }
                if(j > 0){
                    pix = sptr;
                    sptr += sow;
                    rx = (R_VAL(pix) * xap) >> 9;
                    gx = (G_VAL(pix) * xap) >> 9;
                    bx = (B_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += (R_VAL(pix) * Cx) >> 9;
                        gx += (G_VAL(pix) * Cx) >> 9;
                        bx += (B_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        rx += (R_VAL(pix) * i) >> 9;
                        gx += (G_VAL(pix) * i) >> 9;
                        bx += (B_VAL(pix) * i) >> 9;
                    }

                    r += (rx * j) >> 14;
                    g += (gx * j) >> 14;
                    b += (bx * j) >> 14;
                }

                R_VAL(dptr) = r >> 5;
                G_VAL(dptr) = g >> 5;
                B_VAL(dptr) = b >> 5;
                dptr++;
            }
        }
    }
}

// Imlib2/Mosfet code end


// public functions :
// ------------------

// This function returns how many pixels around the zoomed area should be
// included in the image. This is used when doing incremental painting, because
// some smoothing algorithms use surrounding pixels and not including them
// could sometimes make the edges between incremental steps visible.
int extraScalePixels( SmoothAlgorithm alg, double zoom, double blur )
{
	double filtersupport = 0;
	Filter filter = NULL;
	switch( alg ) {
	case SMOOTH_NONE:
		filter = NULL;
		filtersupport = 0.0;
		break;
	case SMOOTH_FAST:
		filter = Box;
		filtersupport = 0.5;
		break;
	case SMOOTH_NORMAL:
		filter = Triangle;
		filtersupport = 1.0;
		break;
	case SMOOTH_BEST:
//		filter = Mitchell;
		filter = Bicubic;
		filtersupport = 2.0;
		break;
	}
	if( zoom == 1.0 || filtersupport == 0.0 ) return 0;
	// Imlib2/Mosfet scale - I have really no idea how many pixels it needs
	if( filter == Box && blur == 1.0 ) return int( 3 / zoom + 1 );
// This is support size for ImageMagick's scaling.
	double scale=blur*QMAX(1.0/zoom,1.0);
	double support=scale* filtersupport;
	if (support <= 0.5) support=0.5+0.000001;
	return int( support + 1 );
}

QImage scale(const QImage& image, int width, int height,
	SmoothAlgorithm alg, QImage::ScaleMode mode, double blur )
{
	if( image.isNull()) return image.copy();
	
	QSize newSize( image.size() );
	newSize.scale( QSize( width, height ), (QSize::ScaleMode)mode ); // ### remove cast in Qt 4.0
	newSize = newSize.expandedTo( QSize( 1, 1 )); // make sure it doesn't become null

	if ( newSize == image.size() ) return image.copy();
	
	width = newSize.width();
	height = newSize.height();
	Filter filter = NULL;
	fastfloat filtersupport;
	
	switch( alg ) {
	case SMOOTH_NONE:
		filter = NULL;
		filtersupport = 0.0;
		break;
	case SMOOTH_FAST:
		filter = Box;
		filtersupport = 0.5;
		break;
	case SMOOTH_NORMAL:
	default:
		filter = Triangle;
		filtersupport = 1.0;
		break;
	case SMOOTH_BEST:
//		filter = Mitchell;
		filter = Bicubic;
		filtersupport = 2.0;
		break;
	}
	
	if( filter == Box && blur == 1.0 )
		return MImageScale::smoothScale( image, width, height );

	if( filter == Box && width > image.width() && height > image.height() && blur == 1.0 ) {
		filter = NULL; // Box doesn't really smooth when enlarging
	}

	if( filter == NULL ) {
		return SampleImage( image, width, height ); // doesn't need 32bit
	}

	return ResizeImage( image.convertDepth( 32 ), width, height, filter, filtersupport, blur );
        // unlike Qt's smoothScale() this function introduces new colors to grayscale images ... oh well
}


} // namespace
