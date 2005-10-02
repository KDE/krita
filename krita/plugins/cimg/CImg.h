/*------------------------------------------------------------------------------------------------------
  
  File        : CImg.h
  
  Description : The C++ Template Image Processing Library
                ( http://cimg.sourceforge.net )

  Copyright   : David Tschumperle
                ( http://www.greyc.ensicaen.fr/~dtschump/ )
   
  This software is governed by the CeCILL  license under French law and
  abiding by the rules of distribution of free software.  You can  use, 
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info". 
  
  As a counterpart to the access to the source code and  rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty  and the software's author,  the holder of the
  economic rights,  and the successive licensors  have only  limited
  liability. 
  
  In this respect, the user's attention is drawn to the risks associated
  with loading,  using,  modifying and/or developing or reproducing the
  software by the user in light of its specific status of free software,
  that may mean  that it is complicated to manipulate,  and  that  also
  therefore means  that it is reserved for developers  and  experienced
  professionals having in-depth computer knowledge. Users are therefore
  encouraged to load and test the software's suitability as regards their
  requirements in conditions enabling the security of their systems and/or 
  data to be ensured and,  more generally, to use and operate it in the 
  same conditions as regards security. 
  
  The fact that you are presently reading this means that you have had
  knowledge of the CeCILL license and that you accept its terms.
  
  ----------------------------------------------------------------------------------------------------*/

#ifndef cimg_version
#define cimg_version 1.09
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cmath>
#include <ctime>

// Overcome VisualC++ 6.0 and DMC compilers namespace 'std::' bug
#if ( defined(_MSC_VER) && _MSC_VER<=1200 ) || defined(__DMC__)
#define std
#endif

/*-------------------------------------------------------------
  
  Set CImg configuration flags.
    
  If compilation flags are not adapted to your system,
  you may override their values, before including
  the header file "CImg.h" (use the #define directive).
  
  -------------------------------------------------------------*/

#ifndef cimg_OS
#if defined(sun)        || defined(__sun)       || defined(linux)       || defined(__linux)     \
 || defined(__CYGWIN__) || defined(__FreeBSD__) || defined(__NetBSD__)  || defined(__OPENBSD__) \
 || defined(__MACOSX__) || defined(__APPLE__)   || defined(sgi)         || defined(__sgi)
// Unix (Linux,Solaris,BSD,Irix,...)
#define cimg_OS            1
#ifndef cimg_display_type
#define cimg_display_type  1
#endif
#ifndef cimg_color_terminal
#define cimg_color_terminal
#endif
#elif defined(_WIN32) || defined(__WIN32__)
// Windows
#define cimg_OS            2
#ifndef cimg_display_type
#define cimg_display_type  2
#endif
#else
// Unknown configuration : compile with minimal dependencies.
#define cimg_OS            0
#ifndef cimg_display_type
#define cimg_display_type  0
#endif
#endif
#endif

// Debug configuration.
//--------------------
// Define 'cimg_debug' to : 0 to remove dynamic debug messages (exceptions are still thrown)
//                          1 to display dynamic debug messages (default behavior).
//                          2 to add extra memory access controls (may slow down the code)
#ifndef cimg_debug
#define cimg_debug         1
#endif

// Architecture-dependent includes
//---------------------------------
#if cimg_OS==1
#include <sys/time.h>
#include <unistd.h>
#else
#include <windows.h>
// Discard unuseful macros in windows.h
#ifdef min
#undef min
#undef max
#undef abs
#endif
#endif
#if cimg_display_type==1
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <pthread.h>
#endif

// Native PNG and JPEG support
//-----------------------------
// Define 'cimg_use_png' or 'cimg_use_jpeg' to enable native PNG or JPEG files support.
// This requires you link your code either with the zlib/png libraries or the jpeg library.
#ifdef cimg_use_png
#include "png.h"
#endif
#ifdef cimg_use_jpeg
#include "jpeglib.h"
#endif

/*-----------------------------------------------------------------------------------


   Define some macros. Macros of the CImg Library are prefixed by 'cimg_'
   Documented macros below may be safely used in your own code.


   ---------------------------------------------------------------------------------*/

// Macros used to describe the program usage, and retrieve command line arguments
// (See corresponding module 'Retrieving command line arguments' in the generated documentation).
#define cimg_usage(usage) cimg_library::cimg::option((char*)NULL,(unsigned int)argc,(char**)argv,(char*)NULL,(char*)usage)
#define cimg_option(name,defaut,usage) cimg_library::cimg::option((char*)name,(unsigned int)argc,(char**)argv,defaut,(char*)usage)

// Macros used for dynamic debug messages. Shouldn't be used in your own source code.
#define cimg_test(x,func)                        \
  if(!(x).width || !(x).height || !(x).depth || !(x).dim || !(x).data)    \
    throw CImgInstanceException("(Instance error) : In function '%s()' ('%s', line %d), CImg<%s> %s = (%d,%d,%d,%d,%p) is empty", \
                                func,__FILE__,__LINE__,(x).pixel_type(),#x,(x).width,(x).height,(x).depth,(x).dim,(x).data)
#define cimgl_test(x,func) \
  if(!(x).size || !(x).data) \
    throw CImgInstanceException("(Instance error) : In function '%s()' ('%s', line %d), CImgl<%s> %s = (%d,%p) is empty", \
                                func,__FILE__,__LINE__,(x).pixel_type(),#x,(x).size,(x).data)
#define cimg_test_scalar(x,func) \
  if(!(x).width || !(x).height || !(x).depth || (x).dim!=1 || !(x).data) \
    throw CImgInstanceException("(Instance error) : In function '%s()' ('%s', line %d), CImg<%s> %s = (%d,%d,%d,%d,%p) is not scalar", \
                                func,__FILE__,__LINE__,(x).pixel_type(),#x,(x).width,(x).height,(x).depth,(x).dim,(x).data)
#define cimg_test_matrix(x,func) \
  if(!(x).width || !(x).height || (x).depth!=1 || (x).dim!=1 || !(x).data) \
    throw CImgInstanceException("(Instance error) : In function '%s()' ('%s', line %d), CImg<%s> %s = (%d,%d,%d,%d,%p) is not a matrix", \
                                func,__FILE__,__LINE__,(x).pixel_type(),#x,(x).width,(x).height,(x).depth,(x).dim,(x).data)
#define cimg_test_square(x,func) \
  if(!(x).width || !(x).height || (x).depth!=1 || (x).dim!=1 || (x).width!=(x).height || !(x).data) \
    throw CImgInstanceException("(Instance error) : In function '%s()' ('%s', line %d), CImg<%s> %s = (%d,%d,%d,%d,%p) is not a square matrix", \
                                func,__FILE__,__LINE__,(x).pixel_type,#x,(x).width,(x).height,(x).depth,(x).dim,(x).data)
#define cimg_test_display(x,func) \
  if (!(x).width || !(x).height) \
    throw CImgInstanceException("(Instance error) : In function '%s()' ('%s', l.%d), CImgDisplay %s = (%d,%d) is not a valid display", \
                                func,__FILE__,__LINE__,#x,(x).width,(x).height)
  
// Macros used for neighborhood definitions and manipulations (see module 'Using Image Loops' in the generated documentation).
#define CImg_2x2(I,T)     T I##cc,I##nc=0,I##cn,I##nn=0
#define CImg_3x3(I,T)     T I##pp,I##cp,I##np=0,I##pc,I##cc,I##nc=0,I##pn,I##cn,I##nn=0
#define CImg_4x4(I,T)     T I##pp,I##cp,I##np=0,I##ap=0, \
                            I##pc,I##cc,I##nc=0,I##ac=0, \
                            I##pn,I##cn,I##nn=0,I##an=0, \
                            I##pa,I##ca,I##na=0,I##aa=0
#define CImg_5x5(I,T)     T I##bb,I##pb,I##cb,I##nb=0,I##ab=0, \
                            I##bp,I##pp,I##cp,I##np=0,I##ap=0, \
                            I##bc,I##pc,I##cc,I##nc=0,I##ac=0, \
                            I##bn,I##pn,I##cn,I##nn=0,I##an=0, \
                            I##ba,I##pa,I##ca,I##na=0,I##aa=0
#define CImg_2x2x2(I,T)   T I##ccc,I##ncc=0,I##cnc,I##nnc=0, \
                            I##ccn,I##ncn=0,I##cnn,I##nnn=0
#define CImg_3x3x3(I,T)   T I##ppp,I##cpp,I##npp=0,I##pcp,I##ccp,I##ncp=0,I##pnp,I##cnp,I##nnp=0, \
                            I##ppc,I##cpc,I##npc=0,I##pcc,I##ccc,I##ncc=0,I##pnc,I##cnc,I##nnc=0, \
                            I##ppn,I##cpn,I##npn=0,I##pcn,I##ccn,I##ncn=0,I##pnn,I##cnn,I##nnn=0

#define CImg_2x2_ref(I,T,tab)   T &I##cc=(tab)[0],&I##nc=(tab)[1],&I##cn=(tab)[2],&I##nn=(tab)[3];
#define CImg_3x3_ref(I,T,tab)   T &I##pp=(tab)[0],&I##cp=(tab)[1],&I##np=(tab)[2], \
                                  &I##pc=(tab)[3],&I##cc=(tab)[4],&I##nc=(tab)[5], \
                                  &I##pn=(tab)[6],&I##cn=(tab)[7],&I##nn=(tab)[8]
#define CImg_4x4_ref(I,T,tab)   T &I##pp=(tab)[0],&I##cp=(tab)[1],&I##np=(tab)[2],&I##ap=(tab)[3], \
                                  &I##pc=(tab)[4],&I##cc=(tab)[5],&I##nc=(tab)[6],&I##ap=(tab)[7], \
                                  &I##pn=(tab)[8],&I##cn=(tab)[9],&I##nn=(tab)[10],&I##aa=(tab)[11], \
                                  &I##pa=(tab)[12],&I##ca=(tab)[13],&I##na=(tab)[14],&I##aa=(tab)[15]
#define CImg_5x5_ref(I,T,tab)   T &I##bb=(tab)[0],&I##pb=(tab)[1],&I##cb=(tab)[2],&I##nb=(tab)[3],&I##ab=(tab)[4], \
                                  &I##bp=(tab)[5],&I##pp=(tab)[6],&I##cp=(tab)[7],&I##np=(tab)[8],&I##ap=(tab)[9], \
                                  &I##bc=(tab)[10],&I##pc=(tab)[11],&I##cc=(tab)[12],&I##nc=(tab)[13],&I##ac=(tab)[14], \
                                  &I##bn=(tab)[15],&I##pn=(tab)[16],&I##cn=(tab)[17],&I##nn=(tab)[18],&I##an=(tab)[19], \
                                  &I##ba=(tab)[20],&I##pa=(tab)[21],&I##ca=(tab)[22],&I##na=(tab)[23],&I##aa=(tab)[24]
#define CImg_2x2x2_ref(I,T,tab) T &I##ccc=(tab)[0],&I##ncc=(tab)[1],&I##cnc=(tab)[2],&I##nnc=(tab)[3], \
                                  &I##ccn=(tab)[4],&I##ncn=(tab)[5],&I##cnn=(tab)[6],&I##nnn=(tab)[7]
#define CImg_3x3x3_ref(I,T,tab) T &I##ppp=(tab)[0],&I##cpp=(tab)[1],&I##npp=(tab)[2], \
                                  &I##pcp=(tab)[3],&I##ccp=(tab)[4],&I##ncp=(tab)[5], \
                                  &I##pnp=(tab)[6],&I##cnp=(tab)[7],&I##nnp=(tab)[8], \
                                  &I##ppc=(tab)[9],&I##cpc=(tab)[10],&I##npc=(tab)[11], \
                                  &I##pcc=(tab)[12],&I##ccc=(tab)[13],&I##ncc=(tab)[14], \
                                  &I##pnc=(tab)[15],&I##cnc=(tab)[16],&I##nnc=(tab)[17], \
                                  &I##ppn=(tab)[18],&I##cpn=(tab)[19],&I##npn=(tab)[20], \
                                  &I##pcn=(tab)[21],&I##ccn=(tab)[22],&I##ncn=(tab)[23], \
                                  &I##pnn=(tab)[24],&I##cnn=(tab)[25],&I##nnn=(tab)[26]

#define cimg_squaresum2x2(I) ( I##cc*I##cc + I##nc*I##nc + I##cn*I##cn + I##nn*I##nn )
#define cimg_squaresum3x3(I) ( I##pp*I##pp + I##cp*I##cp + I##np*I##np + \
                               I##pc*I##pc + I##cc*I##cc + I##nc*I##nc + \
                               I##pn*I##pn + I##cn*I##cn + I##nn*I##nn )
#define cimg_squaresum4x4(I) ( I##pp*I##pp + I##cp*I##cp + I##np*I##np + I##ap*I##ap + \
                               I##pc*I##pc + I##cc*I##cc + I##nc*I##nc + I##ac*I##ac + \
                               I##pn*I##pn + I##cn*I##cn + I##nn*I##nn + I##an*I##an + \
                               I##pa*I##pa + I##ca*I##ca + I##na*I##na + I##aa*I##aa )
#define cimg_squaresum5x5(I) ( I##bb*I##bb + I##pb*I##pb + I##cb*I##cb + I##nb*I##nb + I##ab*I##ab + \
                               I##bp*I##bp + I##pp*I##pp + I##cp*I##cp + I##np*I##np + I##ap*I##ap + \
                               I##bc*I##bc + I##pc*I##pc + I##cc*I##cc + I##nc*I##nc + I##ac*I##ac + \
                               I##bn*I##bn + I##pn*I##pn + I##cn*I##cn + I##nn*I##nn + I##an*I##an + \
                               I##ba*I##ba + I##pa*I##pa + I##ca*I##ca + I##na*I##na + I##aa*I##aa )
#define cimg_squaresum2x2x2(I) ( I##ccc*I##ccc + I##ncc*I##ncc + I##cnc*I##cnc + I##nnc*I##nnc + \
                                 I##ccn*I##ccn + I##ncn*I##ncn + I##cnn*I##cnn + I##nnn*I##nnn )
#define cimg_squaresum3x3x3(I) ( I##ppp*I##ppp + I##cpp*I##cpp + I##npp*I##npp + \
                                 I##pcp*I##pcp + I##ccp*I##ccp + I##ncp*I##ncp + \
                                 I##pnp*I##pnp + I##cnp*I##cnp + I##nnp*I##nnp + \
                                 I##ppc*I##ppc + I##cpc*I##cpc + I##npc*I##npc + \
                                 I##pcc*I##pcc + I##ccc*I##ccc + I##ncc*I##ncc + \
                                 I##pnc*I##pnc + I##cnc*I##cnc + I##nnc*I##nnc + \
                                 I##ppn*I##ppn + I##cpn*I##cpn + I##npn*I##npn + \
                                 I##pcn*I##pcn + I##ccn*I##ccn + I##ncn*I##ncn + \
                                 I##pnn*I##pnn + I##cnn*I##cnn + I##nnn*I##nnn )

#define cimg_corr2x2(I,m) ( I##cc*(m)(0,0)+I##nc*(m)(1,0)+I##cn*(m)(0,1)+I##nn*(m)(1,1) )
#define cimg_corr3x3(I,m) ( I##pp*(m)(0,0)+I##cp*(m)(1,0)+I##np*(m)(2,0) + \
                            I##pc*(m)(0,1)+I##cc*(m)(1,1)+I##nc*(m)(2,1) + \
                            I##pn*(m)(0,2)+I##cn*(m)(1,2)+I##nn*(m)(2,2) )
#define cimg_corr4x4(I,m) ( I##pp*(m)(0,0)+I##cp*(m)(1,0)+I##np*(m)(2,0)+I##ap*(m)(3,0) + \
                            I##pc*(m)(0,1)+I##cc*(m)(1,1)+I##nc*(m)(2,1)+I##ac*(m)(3,1) + \
                            I##pn*(m)(0,2)+I##cn*(m)(1,2)+I##nn*(m)(2,2)+I##an*(m)(3,2) + \
                            I##pa*(m)(0,3)+I##ca*(m)(1,3)+I##na*(m)(2,3)+I##aa*(m)(3,3) )
#define cimg_corr5x5(I,m) ( I##bb*(m)(0,0)+I##pb*(m)(1,0)+I##cb*(m)(2,0)+I##nb*(m)(3,0)+I##ab*(m)(4,0) + \
                            I##bp*(m)(0,1)+I##pp*(m)(1,1)+I##cp*(m)(2,1)+I##np*(m)(3,1)+I##ap*(m)(4,1) + \
                            I##bc*(m)(0,2)+I##pc*(m)(1,2)+I##cc*(m)(2,2)+I##nc*(m)(3,2)+I##ac*(m)(4,2) + \
                            I##bn*(m)(0,3)+I##pn*(m)(1,3)+I##cn*(m)(2,3)+I##nn*(m)(3,3)+I##an*(m)(4,3) + \
                            I##ba*(m)(0,4)+I##pa*(m)(1,4)+I##ca*(m)(2,4)+I##na*(m)(3,4)+I##aa*(m)(4,4) )
#define cimg_corr2x2x2(I,m) ( I##ccc*(m)(0,0,0)+I##ncc*(m)(1,0,0)+I##cnc*(m)(0,1,0)+I##nnc*(m)(1,1,0) + \
                              I##ccn*(m)(0,0,1)+I##ncn*(m)(1,0,1)+I##cnn*(m)(0,1,1)+I##nnn*(m)(1,1,1) )
#define cimg_corr3x3x3(I,m) ( I##ppp*(m)(0,0,0)+I##cpp*(m)(1,0,0)+I##npp*(m)(2,0,0) + \
                              I##pcp*(m)(0,1,0)+I##ccp*(m)(1,1,0)+I##ncp*(m)(2,1,0) + \
                              I##pnp*(m)(0,2,0)+I##cnp*(m)(1,2,0)+I##nnp*(m)(2,2,0) + \
                              I##ppc*(m)(0,0,1)+I##cpc*(m)(1,0,1)+I##npc*(m)(2,0,1) + \
                              I##pcc*(m)(0,1,1)+I##ccc*(m)(1,1,1)+I##ncc*(m)(2,1,1) + \
                              I##pnc*(m)(0,2,1)+I##cnc*(m)(1,2,1)+I##nnc*(m)(2,2,1) + \
                              I##ppn*(m)(0,0,2)+I##cpn*(m)(1,0,2)+I##npn*(m)(2,0,2) + \
                              I##pcn*(m)(0,1,2)+I##ccn*(m)(1,1,2)+I##ncn*(m)(2,1,2) + \
                              I##pnn*(m)(0,2,2)+I##cnn*(m)(1,2,2)+I##nnn*(m)(2,2,2) )

#define cimg_conv2x2(I,m) ( I##cc*(m)(1,1)+I##nc*(m)(0,1)+I##cn*(m)(1,0)+I##nn*(m)(0,0) )
#define cimg_conv3x3(I,m) ( I##pp*(m)(2,2)+I##cp*(m)(1,2)+I##np*(m)(0,2) + \
                            I##pc*(m)(2,1)+I##cc*(m)(1,1)+I##nc*(m)(0,1) + \
                            I##pn*(m)(2,0)+I##cn*(m)(1,0)+I##nn*(m)(0,0) )
#define cimg_conv4x4(I,m) ( I##pp*(m)(3,3)+I##cp*(m)(2,3)+I##np*(m)(1,3)+I##ap*(m)(0,3) + \
                            I##pc*(m)(3,2)+I##cc*(m)(2,2)+I##nc*(m)(1,2)+I##ac*(m)(0,2) + \
                            I##pn*(m)(3,1)+I##cn*(m)(2,1)+I##nn*(m)(1,1)+I##an*(m)(0,1) + \
                            I##pa*(m)(3,0)+I##ca*(m)(2,0)+I##na*(m)(1,0)+I##aa*(m)(0,0) )
#define cimg_conv5x5(I,m) ( I##bb*(m)(4,4)+I##pb*(m)(3,4)+I##cb*(m)(2,4)+I##nb*(m)(1,4)+I##ab*(m)(0,4) + \
                            I##bp*(m)(4,3)+I##pp*(m)(3,3)+I##cp*(m)(2,3)+I##np*(m)(1,3)+I##ap*(m)(0,3) + \
                            I##bc*(m)(4,2)+I##pc*(m)(3,2)+I##cc*(m)(2,2)+I##nc*(m)(1,2)+I##ac*(m)(0,2) + \
                            I##bn*(m)(4,1)+I##pn*(m)(3,1)+I##cn*(m)(2,1)+I##nn*(m)(1,1)+I##an*(m)(0,1) + \
                            I##ba*(m)(4,0)+I##pa*(m)(3,0)+I##ca*(m)(2,0)+I##na*(m)(1,0)+I##aa*(m)(0,0) )
#define cimg_conv2x2x2(I,m) ( I##ccc*(m)(1,1,1)+I##ncc*(m)(0,1,1)+I##cnc*(m)(1,0,1)+I##nnc*(m)(0,0,1) + \
                              I##ccn*(m)(1,1,0)+I##ncn*(m)(0,1,0)+I##cnn*(m)(1,0,0)+I##nnn*(m)(0,0,0) )
#define cimg_conv3x3x3(I,m) ( I##ppp*(m)(2,2,2)+I##cpp*(m)(1,2,2)+I##npp*(m)(0,2,2) + \
                              I##pcp*(m)(2,1,2)+I##ccp*(m)(1,1,2)+I##ncp*(m)(0,1,2) + \
                              I##pnp*(m)(2,0,2)+I##cnp*(m)(1,0,2)+I##nnp*(m)(0,0,2) + \
                              I##ppc*(m)(2,2,1)+I##cpc*(m)(1,2,1)+I##npc*(m)(0,2,1) + \
                              I##pcc*(m)(2,1,1)+I##ccc*(m)(1,1,1)+I##ncc*(m)(0,1,1) + \
                              I##pnc*(m)(2,0,1)+I##cnc*(m)(1,0,1)+I##nnc*(m)(0,0,1) + \
                              I##ppn*(m)(2,2,0)+I##cpn*(m)(1,2,0)+I##npn*(m)(0,2,0) + \
                              I##pcn*(m)(2,1,0)+I##ccn*(m)(1,1,0)+I##ncn*(m)(0,1,0) + \
                              I##pnn*(m)(2,0,0)+I##cnn*(m)(1,0,0)+I##nnn*(m)(0,0,0) )

#define cimg_get2x2(img,x,y,z,v,I) I##cc=(img)(x,    y,z,v), I##nc=(img)(_n##x,    y,z,v), \
    I##cn=(img)(x,_n##y,z,v), I##nn=(img)(_n##x,_n##y,z,v)
#define cimg_get3x3(img,x,y,z,v,I) I##pp=(img)(_p##x,_p##y,z,v), I##cp=(img)(x,_p##y,z,v), I##np=(img)(_n##x,_p##y,z,v), \
    I##pc=(img)(_p##x,    y,z,v), I##cc=(img)(x,    y,z,v), I##nc=(img)(_n##x,    y,z,v), \
    I##pn=(img)(_p##x,_n##y,z,v), I##cn=(img)(x,_n##y,z,v), I##nn=(img)(_n##x,_n##y,z,v)
#define cimg_get4x4(img,x,y,z,v,I)                                      \
  I##pp=(img)(_p##x,_p##y,z,v), I##cp=(img)(x,_p##y,z,v), I##np=(img)(_n##x,_p##y,z,v), I##ap=(img)(_a##x,_p##y,z,v), \
    I##pc=(img)(_p##x,    y,z,v), I##cc=(img)(x,    y,z,v), I##nc=(img)(_n##x,    y,z,v), I##ac=(img)(_a##x,    y,z,v), \
    I##pn=(img)(_p##x,_n##y,z,v), I##cn=(img)(x,_n##y,z,v), I##nn=(img)(_n##x,_n##y,z,v), I##an=(img)(_a##x,_n##y,z,v), \
    I##pa=(img)(_p##x,_a##y,z,v), I##ca=(img)(x,_a##y,z,v), I##na=(img)(_n##x,_a##y,z,v), I##aa=(img)(_a##x,_a##y,z,v)
#define cimg_get5x5(img,x,y,z,v,I)                                      \
  I##bb=(img)(_b##x,_b##y,z,v), I##pb=(img)(_p##x,_b##y,z,v), I##cb=(img)(x,_b##y,z,v), I##nb=(img)(_n##x,_b##y,z,v), I##ab=(img)(_a##x,_b##y,z,v), \
    I##bp=(img)(_b##x,_p##y,z,v), I##pp=(img)(_p##x,_p##y,z,v), I##cp=(img)(x,_p##y,z,v), I##np=(img)(_n##x,_p##y,z,v), I##ap=(img)(_a##x,_p##y,z,v), \
    I##bc=(img)(_b##x,    y,z,v), I##pc=(img)(_p##x,    y,z,v), I##cc=(img)(x,    y,z,v), I##nc=(img)(_n##x,    y,z,v), I##ac=(img)(_a##x,    y,z,v), \
    I##bn=(img)(_b##x,_n##y,z,v), I##pn=(img)(_p##x,_n##y,z,v), I##cn=(img)(x,_n##y,z,v), I##nn=(img)(_n##x,_n##y,z,v), I##an=(img)(_a##x,_n##y,z,v), \
    I##ba=(img)(_b##x,_a##y,z,v), I##pa=(img)(_p##x,_a##y,z,v), I##ca=(img)(x,_a##y,z,v), I##na=(img)(_n##x,_a##y,z,v), I##aa=(img)(_a##x,_a##y,z,v)
#define cimg_get2x2x2(img,x,y,z,v,I)                                    \
  I##ccc=(img)(x,y,    z,v), I##ncc=(img)(_n##x,y,    z,v), I##cnc=(img)(x,_n##y,    z,v), I##nnc=(img)(_n##x,_n##y,    z,v), \
    I##ccc=(img)(x,y,_n##z,v), I##ncc=(img)(_n##x,y,_n##z,v), I##cnc=(img)(x,_n##y,_n##z,v), I##nnc=(img)(_n##x,_n##y,_n##z,v)
#define cimg_get3x3x3(img,x,y,z,v,I)                                    \
  I##ppp=(img)(_p##x,_p##y,_p##z,v), I##cpp=(img)(x,_p##y,_p##z,v), I##npp=(img)(_n##x,_p##y,_p##z,v), \
    I##pcp=(img)(_p##x,    y,_p##z,v), I##ccp=(img)(x,    y,_p##z,v), I##ncp=(img)(_n##x,    y,_p##z,v), \
    I##pnp=(img)(_p##x,_n##y,_p##z,v), I##cnp=(img)(x,_n##y,_p##z,v), I##nnp=(img)(_n##x,_n##y,_p##z,v), \
    I##ppc=(img)(_p##x,_p##y,    z,v), I##cpc=(img)(x,_p##y,    z,v), I##npc=(img)(_n##x,_p##y,    z,v), \
    I##pcc=(img)(_p##x,    y,    z,v), I##ccc=(img)(x,    y,    z,v), I##ncc=(img)(_n##x,    y,    z,v), \
    I##pnc=(img)(_p##x,_n##y,    z,v), I##cnc=(img)(x,_n##y,    z,v), I##nnc=(img)(_n##x,_n##y,    z,v), \
    I##ppn=(img)(_p##x,_p##y,_n##z,v), I##cpn=(img)(x,_p##y,_n##z,v), I##npn=(img)(_n##x,_p##y,_n##z,v), \
    I##pcn=(img)(_p##x,    y,_n##z,v), I##ccn=(img)(x,    y,_n##z,v), I##ncn=(img)(_n##x,    y,_n##z,v), \
    I##pnn=(img)(_p##x,_n##y,_n##z,v), I##cnn=(img)(x,_n##y,_n##z,v), I##nnn=(img)(_n##x,_n##y,_n##z,v)

#define cimg_3x3to5x5(I,u) u##bb=I##pp,u##cb=I##cp,u##ab=I##np,u##bc=I##pc,u##cc=I##cc,u##ac=I##nc,u##ba=I##pn,u##ca=I##cn,u##aa=I##nn, \
    u##pb=0.5*(u##bb+u##cb),u##nb=0.5*(u##cb+u##ab),u##pc=0.5*(u##bc+u##cc),u##nc=0.5*(u##cc+u##ac),u##pa=0.5*(u##ba+u##ca),u##na=0.5*(u##ca+u##aa), \
    u##bp=0.5*(u##bb+u##bc),u##bn=0.5*(u##bc+u##ba),u##cp=0.5*(u##cb+u##cc),u##cn=0.5*(u##cc+u##ca),u##ap=0.5*(u##ab+u##ac),u##an=0.5*(u##ac+u##aa), \
    u##pp=0.5*(u##bp+u##cp),u##np=0.5*(u##cp+u##ap),u##pn=0.5*(u##bn+u##cn),u##nn=0.5*(u##cn+u##an)

// Macros used to define special image loops (see module 'Using Image Loops' in the generated documentation).
#define cimg_map(img,ptr,T_ptr)   for (T_ptr *ptr=(img).data+(img).size()-1; ptr>=(img).data; ptr--)
#define cimgl_map(list,l)         for (unsigned int l=0; l<(list).size; l++)
#define cimg_mapoff(img,off)      for (unsigned int off=0; off<(img).size(); off++)
#define cimg_mapX(img,x)          for (int x=0; x<(int)((img).width); x++)
#define cimg_mapY(img,y)          for (int y=0; y<(int)((img).height);y++)
#define cimg_mapZ(img,z)          for (int z=0; z<(int)((img).depth); z++)
#define cimg_mapV(img,v)          for (int v=0; v<(int)((img).dim);   v++)
#define cimg_mapXY(img,x,y)       cimg_mapY(img,y) cimg_mapX(img,x)
#define cimg_mapXZ(img,x,z)       cimg_mapZ(img,z) cimg_mapX(img,x)
#define cimg_mapYZ(img,y,z)       cimg_mapZ(img,z) cimg_mapY(img,y)
#define cimg_mapXV(img,x,v)       cimg_mapV(img,v) cimg_mapX(img,x)
#define cimg_mapYV(img,y,v)       cimg_mapV(img,v) cimg_mapY(img,y)
#define cimg_mapZV(img,z,v)       cimg_mapV(img,v) cimg_mapZ(img,z)
#define cimg_mapXYZ(img,x,y,z)    cimg_mapZ(img,z) cimg_mapXY(img,x,y)
#define cimg_mapXYV(img,x,y,v)    cimg_mapV(img,v) cimg_mapXY(img,x,y)
#define cimg_mapXZV(img,x,z,v)    cimg_mapV(img,v) cimg_mapXZ(img,x,z)
#define cimg_mapYZV(img,y,z,v)    cimg_mapV(img,v) cimg_mapYZ(img,y,z)
#define cimg_mapXYZV(img,x,y,z,v) cimg_mapV(img,v) cimg_mapXYZ(img,x,y,z)
#define cimg_imapX(img,x,n)       for (int x=n; x<(int)((img).width-n); x++)
#define cimg_imapY(img,y,n)       for (int y=n; y<(int)((img).height-n); y++)
#define cimg_imapZ(img,z,n)       for (int z=n; z<(int)((img).depth-n); z++)
#define cimg_imapV(img,v,n)       for (int v=n; v<(int)((img).dim-n); v++)
#define cimg_imapXY(img,x,y,n)    cimg_imapY(img,y,n) cimg_imapX(img,x,n)
#define cimg_imapXYZ(img,x,y,z,n) cimg_imapZ(img,z,n) cimg_imapXY(img,x,y,n)
#define cimg_bmapX(img,x,n)       for (int x=0; x<(int)((img).width);  x==(n)-1?(x=(img).width-(n)): x++)
#define cimg_bmapY(img,y,n)       for (int y=0; y<(int)((img).height); y==(n)-1?(x=(img).height-(n)):y++)
#define cimg_bmapZ(img,z,n)       for (int z=0; z<(int)((img).depth);  z==(n)-1?(x=(img).depth-(n)): z++)
#define cimg_bmapV(img,v,n)       for (int v=0; v<(int)((img).dim);    v==(n)-1?(x=(img).dim-(n)):   v++)
#define cimg_bmapXY(img,x,y,n)    cimg_mapY(img,y) for (int x=0; x<(int)((img).width); (y<(n) || y>=(int)((img).height)-(n))?x++: \
                                                          ((x<(n)-1 || x>=(int)((img).width)-(n))?x++:(x=(img).width-(n))))
#define cimg_bmapXYZ(img,x,y,z,n) cimg_mapYZ(img,y,z) for (int x=0; x<(int)((img).width); (y<(n) || y>=(int)((img).height)-(n) || z<(n) || z>=(int)((img).depth)-(n))?x++: \
                                                             ((x<(n)-1 || x>=(int)((img).width)-(n))?x++:(x=(img).width-(n))))
#define cimg_2mapX(img,x)         for (int x=0,_n##x=1; _n##x<(int)((img).width)   || x==--_n##x; x++, _n##x++)
#define cimg_2mapY(img,y)         for (int y=0,_n##y=1; _n##y<(int)((img).height)  || y==--_n##y; y++, _n##y++)
#define cimg_2mapZ(img,z)         for (int z=0,_n##z=1; _n##z<(int)((img).depth)   || z==--_n##z; z++, _n##z++)
#define cimg_2mapXY(img,x,y)      cimg_2mapY(img,y) cimg_2mapX(img,x)
#define cimg_2mapXZ(img,x,z)      cimg_2mapZ(img,z) cimg_2mapX(img,x)
#define cimg_2mapYZ(img,y,z)      cimg_2mapZ(img,z) cimg_2mapY(img,y)
#define cimg_2mapXYZ(img,x,y,z)   cimg_2mapZ(img,z) cimg_2mapXY(img,x,y)
#define cimg_3mapX(img,x)         for (int x=0,_p##x=0,_n##x=1; _n##x<(int)((img).width)  || x==--_n##x;  _p##x=x++,_n##x++)
#define cimg_3mapY(img,y)         for (int y=0,_p##y=0,_n##y=1; _n##y<(int)((img).height) || y==--_n##y;  _p##y=y++,_n##y++)
#define cimg_3mapZ(img,z)         for (int z=0,_p##z=0,_n##z=1; _n##z<(int)((img).depth)  || z==--_n##z;  _p##z=z++,_n##z++)
#define cimg_3mapXY(img,x,y)      cimg_3mapY(img,y) cimg_3mapX(img,x)
#define cimg_3mapXZ(img,x,z)      cimg_3mapZ(img,z) cimg_3mapX(img,x)
#define cimg_3mapYZ(img,y,z)      cimg_3mapZ(img,z) cimg_3mapY(img,y)
#define cimg_3mapXYZ(img,x,y,z)   cimg_3mapZ(img,z) cimg_3mapXY(img,x,y)
#define cimg_4mapX(img,x)         for (int _p##x=0,x=0,_n##x=1,_a##x=2; \
                                       _a##x<(int)((img).width)  || _n##x==--_a##x || x==(_a##x=--_n##x); \
                                       _p##x=x++,_n##x++,_a##x++)
#define cimg_4mapY(img,y)         for (int _p##y=0,y=0,_n##y=1,_a##y=2; \
                                       _a##y<(int)((img).height) || _n##y==--_a##y || y==(_a##y=--_n##y); \
                                       _p##y=y++,_n##y++,_a##y++)
#define cimg_4mapZ(img,z)         for (int _p##z=0,z=0,_n##z=1,_a##z=2; \
                                       _a##z<(int)((img).depth)  || _n##z==--_a##z || z==(_a##z=--_n##z); \
                                       _p##z=z++,_n##z++,_a##z++)
#define cimg_4mapXY(img,x,y)      cimg_4mapY(img,y) cimg_4mapX(img,x)
#define cimg_4mapXZ(img,x,z)      cimg_4mapZ(img,z) cimg_4mapX(img,x)
#define cimg_4mapYZ(img,y,z)      cimg_4mapZ(img,z) cimg_4mapY(img,y)
#define cimg_4mapXYZ(img,x,y,z)   cimg_4mapZ(img,z) cimg_4mapXY(img,x,y)
#define cimg_5mapX(img,x)         for (int _b##x=0,_p##x=0,x=0,_n##x=1,_a##x=2; \
                                       _a##x<(int)((img).width)  || _n##x==--_a##x || x==(_a##x=--_n##x); \
                                       _b##x=_p##x,_p##x=x++,_n##x++,_a##x++)
#define cimg_5mapY(img,y)         for (int _b##y=0,_p##y=0,y=0,_n##y=1,_a##y=2; \
                                       _a##y<(int)((img).height) || _n##y==--_a##y || y==(_a##y=--_n##y); \
                                       _b##y=_p##y,_p##y=y++,_n##y++,_a##y++)
#define cimg_5mapZ(img,z)         for (int _b##z=0,_p##z=0,z=0,_n##z=1,_a##z=2; \
                                       _a##z<(int)((img).depth)  || _n##z==--_a##z || z==(_a##z=--_n##z); \
                                       _b##z=_p##z,_p##z=z++,_n##z++,_a##z++)
#define cimg_5mapXY(img,x,y)      cimg_5mapY(img,y) cimg_5mapX(img,x)
#define cimg_5mapXZ(img,x,z)      cimg_5mapZ(img,z) cimg_5mapX(img,x)
#define cimg_5mapYZ(img,y,z)      cimg_5mapZ(img,z) cimg_5mapY(img,y)
#define cimg_5mapXYZ(img,x,y,z)   cimg_5mapZ(img,z) cimg_5mapXY(img,x,y)

#define cimg_map2x2(img,x,y,z,v,I) cimg_2mapY(img,y)                    \
       for (int _n##x=1, x=((int)(I##cc=(img)(0,  y,z,v),               \
                                  I##cn=(img)(0,_n##y,z,v)),0);         \
            (_n##x<(int)((img).width) && (                              \
                                          I##nc=(img)(_n##x,    y,z,v), \
                                          I##nn=(img)(_n##x,_n##y,z,v), \
                                          1)) || x==--_n##x;            \
            I##cc=I##nc, I##cn=I##nn,                                   \
              x++,_n##x++ )

#define cimg_map3x3(img,x,y,z,v,I) cimg_3mapY(img,y)                    \
       for (int _n##x=1, _p##x=(int)(I##cp=I##pp=(img)(0,_p##y,z,v),    \
                                     I##cc=I##pc=(img)(0,  y,z,v),      \
                                     I##cn=I##pn=(img)(0,_n##y,z,v)     \
                                     ), x=_p##x=0;                      \
            (_n##x<(int)((img).width) && (                              \
                                          I##np=(img)(_n##x,_p##y,z,v), \
                                          I##nc=(img)(_n##x,    y,z,v), \
                                          I##nn=(img)(_n##x,_n##y,z,v), \
                                          1)) || x==--_n##x;            \
            I##pp=I##cp, I##pc=I##cc, I##pn=I##cn,                      \
              I##cp=I##np, I##cc=I##nc, I##cn=I##nn,                    \
              _p##x=x++,_n##x++ )

#define cimg_map4x4(img,x,y,z,v,I) cimg_4mapY(img,y)                    \
       for (int _a##x=2, _n##x=1, x=((int)(I##cp=I##pp=(img)(0,_p##y,z,v), \
                                           I##cc=I##pc=(img)(0,    y,z,v), \
                                           I##cn=I##pn=(img)(0,_n##y,z,v), \
                                           I##ca=I##pa=(img)(0,_a##y,z,v), \
                                           I##np=(img)(_n##x,_p##y,z,v), \
                                           I##nc=(img)(_n##x,    y,z,v), \
                                           I##nn=(img)(_n##x,_n##y,z,v), \
                                           I##na=(img)(_n##x,_a##y,z,v)),0), \
              _p##x=0;                                                  \
            (_a##x<(int)((img).width) && (                              \
                                          I##ap=(img)(_a##x,_p##y,z,v), \
                                          I##ac=(img)(_a##x,    y,z,v), \
                                          I##an=(img)(_a##x,_n##y,z,v), \
                                          I##aa=(img)(_a##x,_a##y,z,v), \
                                          1)) || _n##x==--_a##x || x==(_a##x=--_n##x); \
            I##pp=I##cp, I##pc=I##cc, I##pn=I##cn, I##pa=I##ca,         \
              I##cp=I##np, I##cc=I##nc, I##cn=I##nn, I##ca=I##na,       \
              I##np=I##ap, I##nc=I##ac, I##nn=I##an, I##na=I##aa,       \
              _p##x=x++, _n##x++, _a##x++ )

#define cimg_map5x5(img,x,y,z,v,I) cimg_5mapY(img,y)                    \
       for (int _a##x=2, _n##x=1, _b##x=(int)(I##cb=I##pb=I##bb=(img)(0,_b##y,z,v), \
                                              I##cp=I##pp=I##bp=(img)(0,_p##y,z,v), \
                                              I##cc=I##pc=I##bc=(img)(0,    y,z,v), \
                                              I##cn=I##pn=I##bn=(img)(0,_n##y,z,v), \
                                              I##ca=I##pa=I##ba=(img)(0,_a##y,z,v), \
                                              I##nb=(img)(_n##x,_b##y,z,v), \
                                              I##np=(img)(_n##x,_p##y,z,v), \
                                              I##nc=(img)(_n##x,   y,z,v), \
                                              I##nn=(img)(_n##x,_n##y,z,v), \
                                              I##na=(img)(_n##x,_a##y,z,v)), \
              x=0, _p##x=_b##x=0;                                       \
            (_a##x<(int)((img).width) && (                              \
                                          I##ab=(img)(_a##x,_b##y,z,v), \
                                          I##ap=(img)(_a##x,_p##y,z,v), \
                                          I##ac=(img)(_a##x,    y,z,v), \
                                          I##an=(img)(_a##x,_n##y,z,v), \
                                          I##aa=(img)(_a##x,_a##y,z,v), \
                                          1)) || _n##x==--_a##x || x==(_a##x=--_n##x); \
            I##bb=I##pb, I##bp=I##pp, I##bc=I##pc, I##bn=I##pn, I##ba=I##pa, \
              I##pb=I##cb, I##pp=I##cp, I##pc=I##cc, I##pn=I##cn, I##pa=I##ca, \
              I##cb=I##nb, I##cp=I##np, I##cc=I##nc, I##cn=I##nn, I##ca=I##na, \
              I##nb=I##ab, I##np=I##ap, I##nc=I##ac, I##nn=I##an, I##na=I##aa, \
              _b##x=_p##x, _p##x=x++, _n##x++, _a##x++ )

#define cimg_map2x2x2(img,x,y,z,v,I) cimg_2mapYZ(img,y,z)               \
       for (int _n##x=1, x=((int)(I##ccc=(img)(0,    y,    z,v),        \
                                  I##cnc=(img)(0,_n##y,    z,v),        \
                                  I##ccn=(img)(0,    y,_n##z,v),        \
                                  I##cnn=(img)(0,_n##y,_n##z,v)),0);    \
            (_n##x<(int)((img).width) && (                              \
                                          I##ncc=(img)(_n##x,    y,    z,v), \
                                          I##nnc=(img)(_n##x,_n##y,    z,v), \
                                          I##ncn=(img)(_n##x,    y,_n##z,v), \
                                          I##nnn=(img)(_n##x,_n##y,_n##z,v), \
                                          1)) || x==--_n##x;            \
            I##ccc=I##ncc, I##cnc=I##nnc,                               \
              I##ccn=I##ncn, I##cnn=I##nnn,                             \
              x++, _n##x++ )

#define cimg_map3x3x3(img,x,y,z,v,I) cimg_3mapYZ(img,y,z)               \
       for (int _n##x=1, _p##x=(int)(I##cpp=I##ppp=(img)(0,_p##y,_p##z,v), \
                                     I##ccp=I##pcp=(img)(0,    y,_p##z,v), \
                                     I##cnp=I##pnp=(img)(0,_n##y,_p##z,v), \
                                     I##cpc=I##ppc=(img)(0,_p##y,    z,v), \
                                     I##ccc=I##pcc=(img)(0,    y,    z,v), \
                                     I##cnc=I##pnc=(img)(0,_n##y,    z,v), \
                                     I##cpn=I##ppn=(img)(0,_p##y,_n##z,v), \
                                     I##ccn=I##pcn=(img)(0,    y,_n##z,v), \
                                     I##cnn=I##pnn=(img)(0,_n##y,_n##z,v)),\
              x=_p##x=0;                                                \
            (_n##x<(int)((img).width) && (                              \
                                          I##npp=(img)(_n##x,_p##y,_p##z,v), \
                                          I##ncp=(img)(_n##x,    y,_p##z,v), \
                                          I##nnp=(img)(_n##x,_n##y,_p##z,v), \
                                          I##npc=(img)(_n##x,_p##y,    z,v), \
                                          I##ncc=(img)(_n##x,    y,    z,v), \
                                          I##nnc=(img)(_n##x,_n##y,    z,v), \
                                          I##npn=(img)(_n##x,_p##y,_n##z,v), \
                                          I##ncn=(img)(_n##x,    y,_n##z,v), \
                                          I##nnn=(img)(_n##x,_n##y,_n##z,v), \
                                          1)) || x==--_n##x;            \
            I##ppp=I##cpp, I##pcp=I##ccp, I##pnp=I##cnp,                \
              I##cpp=I##npp, I##ccp=I##ncp, I##cnp=I##nnp,              \
              I##ppc=I##cpc, I##pcc=I##ccc, I##pnc=I##cnc,              \
              I##cpc=I##npc, I##ccc=I##ncc, I##cnc=I##nnc,              \
              I##ppn=I##cpn, I##pcn=I##ccn, I##pnn=I##cnn,              \
              I##cpn=I##npn, I##ccn=I##ncn, I##cnn=I##nnn,              \
              _p##x=x++, _n##x++ )

/*-------------------------------------------------
  -------------------------------------------------


    Definition of the cimg_library:: namespace


  -------------------------------------------------
  -------------------------------------------------*/

//! The <tt>\ref cimg_library::</tt> namespace encompasses all classes and functions of the CImg library.
/**
   This namespace is defined to avoid class names collisions that could happen
   with the include of other C++ header files. Anyway, it shouldn't happen
   very often and you may start most of your programs with
   \code
   #include "CImg.h"
   using namespace cimg_library;
   \endcode
   to simplify the declaration of CImg class variables afterward.
**/
namespace cimg_library {
  struct CImgStats;
  struct CImgDisplay;
  struct CImgException;
  template<typename T=float> struct CImg;
  template<typename T=float> struct CImgl;
  template<typename T=float> struct CImgROI;
 
  namespace cimg {
    static int dialog(const char *title,const char *msg,const char *button1_txt="OK",
              const char *button2_txt=NULL,const char *button3_txt=NULL,
              const char *button4_txt=NULL,const char *button5_txt=NULL,
              const char *button6_txt=NULL);
  }
  
  /*----------------------------------------------------
    
  
  
  Definition of the CImgException structures
  
  
  
  -------------------------------------------------*/

#define cimg_exception_err(etype,disp_flag)                         \
  if (cimg_debug>=1) {                                              \
    char tmp[1024];                                                 \
    std::va_list ap;                                                \
    va_start(ap,format);                                            \
    std::vsprintf(message,format,ap);                               \
    va_end(ap);                                                     \
    std::sprintf(tmp,"==> %s \n\nGeneral : %s\n\n", message,etype); \
    if (disp_flag) cimg::dialog("CImg Error",tmp,"Abort");          \
    else std::fprintf(stderr,tmp);                                  \
  }
    
  //! The \ref CImgException class is used to throw general exceptions when an error occurs in a library call.
  /** 
      The \ref CImgException class is the base class of all CImg exceptions.
      Exceptions are thrown by the CImg Library when an error occurs during the execution of a CImg function.
      The CImgException is seldom thrown itself, children classes (that specify the type of error)
      are generally used instead.
      It may be thrown anyway for non-specialized exception types.

      \see CImgInstanceException, CImgArgumentException, CImgIOException and CImgDisplayException.

      By default, when an error occurs, the CImg Library displays an error message on the standart error output \e stderr
      (on Unix), or opens a pop-up window displaying the error message (on Windows).
      Then, it throws an instance of an exception class, generally leading the program to stop (this is the
      default behavior of the C++ exception mechanism).
      You can always bypass this behavior by handling the exceptions by yourself (using a code block <tt>try { ... } catch() { ... }</tt>).
      Then, if you don't want the CImg Library to display error messages, you can define the environment variable
      <tt>cimg_debug</tt> to 0 before including the header file <tt>CImg.h</tt> (see \ref cimg_environment).
      
      The <tt>CImgException</tt> class owns a member variable <tt>char* message</tt> that contains the exception message
      describing precisely the error that occured.

      The example above shows how to manually handle CImg Library errors properly :
      \code
      #define cimg_debug 0    // Disable error message display by CImg.
      #define "CImg.h"
      int main() {  
        try {
          ...; // Here, do what you want.
        }
        catch (CImgException &e) {
          std::fprintf(stderr,"CImg Library Error : %s",e.message);  // Display error message
          ...                                                        // Do what you want to save the ship !
        }
      }
      \endcode      
  **/
  struct CImgException {
    char message[1024]; //!< Error message
    CImgException() { message[0]='\0'; }
    CImgException(const char *format,...) {
      cimg_exception_err("This error has been generated by a 'CImgException' throw,\n"
             "corresponding to a general exception problem.",true); 
    }
  };

  //! The \ref CImgInstanceException class is used to throw an exception related
  //! to a non suitable instance encountered in a library function call.  
  /**
     This class will be thrown when trying to call a class function from an
     instance which is \e empty, or \e badly \e defined.
     Typically, this exception occurs when using empty images :
     \code
     CImg<float> img; // define an empty image
     img.blur(10);    // trying to blur an empty image will generate a CImgInstanceException.
     \endcode
     
     \see CImgException, CImgArgumentException, CImgIOException and CImgDisplayException.
  **/
  struct CImgInstanceException : CImgException { 
    CImgInstanceException(const char *format,...) {
      cimg_exception_err("This error has been generated by a 'CImgInstanceException' throw.\n"
             "The instance passed through the function above has a bad structure\n"
             "(perhaps an empty image, list or display object ?)",true);
    }};

  //! The \ref CImgArgumentException class is used to throw an exception related
  //! to invalid arguments encountered in a library function call.
  /**
     This class will be thrown when one passes one or several invalid arguments to
     a library function. This may happen for instance in the following case :
     \code
     CImg<float> img(100,100); // define a 100x100 scalar image with float pixels
     img.get_channel(1);       // trying to retrieve the vector channel v=1, will generate a CImgArgumentException.
     \endcode
     As the image <tt>img</tt> is scalar, it has only one vector channel (<tt>img.dim=1</tt>), and one cannot
     retrieve the channel 1, (only the channel 0).
     \see CImgException, CImgInstanceException, CImgIOException and CImgDisplayException.
  **/
  struct CImgArgumentException : CImgException { 
    CImgArgumentException(const char *format,...) { 
      cimg_exception_err("This error has been generated by a 'CImgArgumentException' throw.\n"
             "At least one argument passed to the function above has been\n"
             "considered as not valid.",true);
    }};

  //! The \ref CImgIOException class is used to throw an exception related 
  //! to Input/Output file problems encountered in a library function call.
  /**
     This class will be thrown when one Input/Output problem has been encountered
     during the execution of a library function. This may particularly happen when using <tt>load()</tt> and <tt>save()</tt>
     functions, with invalid files :
     \code
     CImg<float> img("not_here.jpg");
     \endcode
     This code will throw a \c CImgIOException instance if the file <tt>not_here.jpg</tt> is not in the current directory.
     \see CImgException, CImgInstanceException, CImgArgumentException and CImgDisplayException.
  **/
  struct CImgIOException : CImgException { 
    CImgIOException(const char *format,...) {
      cimg_exception_err("This error has been generated by a 'CImgIOException' throw.\n"
             "When trying to load or save a file, the function above has\n"
             "encountered a problem.",true);
    }};

  //! The CImgDisplayException class is used to throw an exception related to display problems
  //! encountered in a library function call.
  /**
     This class will be thrown when a library function encounters a problem
     when trying to open or close a display window. This shouldn't happen very much if your display is 
     detected properly. This happens for instance when trying to open a display window on a 8bits screen
     depth, under Unix/X11.

     \see CImgException, CImgInstanceException, CImgArgumentException and CImgIOException.
  **/
  struct CImgDisplayException : CImgException {
    CImgDisplayException(const char *format,...) {
      cimg_exception_err("This error has been generated by a 'CImgDisplayException' throw.\n"
             "When trying to operate on a CImgDisplay instance, the function above\n"
             "has encountered a problem.",false); 
    }};
  

  /*----------------------------------------
    
  
  
    Definition of the namespace 'cimg'
  
  
  
  --------------------------------------*/
  
  //! The <tt>\ref cimg::</tt> namespace encompasses \e low-level functions and variables of the CImg Library.
  /**
     Most of the functions and variables within this namespace are used by the library for low-level purposes.
     Nevertheless, documented variables and functions below may be used by the user in its own source code.
     \warning Never write <tt>"using namespace cimg_library::cimg;"</tt> in your source, since a lot of functions of the
     \ref cimg:: namespace have the same name than standart C functions defined in the global namespace <tt>::</tt>.
     
     \see CImg, CImgl, CImgDisplay, CImgStats and CImgException.
  **/
  namespace cimg {

    // Define internal library variables.
    const unsigned int lblock=1024;
#if cimg_display_type==1
    struct X11info {
      pthread_mutex_t* mutex;
      pthread_t*       event_thread;
      CImgDisplay*     wins[1024];
      Display*         display;
      unsigned int     nb_wins;
      bool             thread_finished;
      unsigned int     nb_bits;
      GC*              gc;
      bool             endian;
      X11info():mutex(NULL),event_thread(NULL),display(NULL),nb_wins(0),
        thread_finished(false),nb_bits(0),gc(NULL),endian(false) {};
    };
#if defined(cimg_module)
    X11info& X11attr();
#elif defined(cimg_main)
    X11info& X11attr() { static X11info val; return val; }
#else
    inline X11info& X11attr() { static X11info val; return val; }
#endif
#endif
#ifdef cimg_color_terminal
    const char t_normal[9]  = {0x1b,'[','0',';','0',';','0','m','\0'};
    const char t_red[11]    = {0x1b,'[','4',';','3','1',';','5','9','m','\0'};
    const char t_bold[5]    = {0x1b,'[','1','m','\0'};
    const char t_purple[11] = {0x1b,'[','0',';','3','5',';','5','9','m','\0'};
#else
    const char t_normal[1]  = {'\0'};
    static const char *t_red = cimg::t_normal, *t_bold = cimg::t_normal, *t_purple = cimg::t_normal;
#endif
    
#if cimg_display_type==1
    // Keycodes for X11-based graphical systems
    const unsigned int keyESC        = XK_Escape;
    const unsigned int keyF1         = XK_F1;
    const unsigned int keyF2         = XK_F2;
    const unsigned int keyF3         = XK_F3;
    const unsigned int keyF4         = XK_F4;
    const unsigned int keyF5         = XK_F5;
    const unsigned int keyF6         = XK_F6;
    const unsigned int keyF7         = XK_F7;
    const unsigned int keyF8         = XK_F8;
    const unsigned int keyF9         = XK_F9;
    const unsigned int keyF10        = XK_F10;
    const unsigned int keyF11        = XK_F11;
    const unsigned int keyF12        = XK_F12;
    const unsigned int keyPAUSE      = XK_Pause;
    const unsigned int key1          = XK_1;
    const unsigned int key2          = XK_2;
    const unsigned int key3          = XK_3;
    const unsigned int key4          = XK_4;
    const unsigned int key5          = XK_5;
    const unsigned int key6          = XK_6;
    const unsigned int key7          = XK_7;
    const unsigned int key8          = XK_8;
    const unsigned int key9          = XK_9;
    const unsigned int key0          = XK_0;
    const unsigned int keyBACKSPACE  = XK_BackSpace;
    const unsigned int keyINSERT     = XK_Insert;
    const unsigned int keyHOME       = XK_Home;
    const unsigned int keyPAGEUP     = XK_Page_Up;
    const unsigned int keyTAB        = XK_Tab;
    const unsigned int keyQ          = XK_q;
    const unsigned int keyW          = XK_w;
    const unsigned int keyE          = XK_e;
    const unsigned int keyR          = XK_r;
    const unsigned int keyT          = XK_t;
    const unsigned int keyY          = XK_y;
    const unsigned int keyU          = XK_u;
    const unsigned int keyI          = XK_i;
    const unsigned int keyO          = XK_o;
    const unsigned int keyP          = XK_p;
    const unsigned int keyDELETE     = XK_Delete;
    const unsigned int keyEND        = XK_End;
    const unsigned int keyPAGEDOWN   = XK_Page_Down;
    const unsigned int keyCAPSLOCK   = XK_Caps_Lock;
    const unsigned int keyA          = XK_a;
    const unsigned int keyS          = XK_s;
    const unsigned int keyD          = XK_d;
    const unsigned int keyF          = XK_f;
    const unsigned int keyG          = XK_g;
    const unsigned int keyH          = XK_h;
    const unsigned int keyJ          = XK_j;
    const unsigned int keyK          = XK_k;
    const unsigned int keyL          = XK_l;
    const unsigned int keyENTER      = XK_Return;
    const unsigned int keySHIFTLEFT  = XK_Shift_L;
    const unsigned int keyZ          = XK_z;
    const unsigned int keyX          = XK_x;
    const unsigned int keyC          = XK_c;
    const unsigned int keyV          = XK_v;
    const unsigned int keyB          = XK_b;
    const unsigned int keyN          = XK_n;
    const unsigned int keyM          = XK_m;
    const unsigned int keySHIFTRIGHT = XK_Shift_R;
    const unsigned int keyARROWUP    = XK_Up;
    const unsigned int keyCTRLLEFT   = XK_Control_L;
    const unsigned int keyAPPLEFT    = XK_Super_L;
    const unsigned int keySPACE      = XK_space;
    const unsigned int keyALTGR      = XK_Alt_R;
    const unsigned int keyAPPRIGHT   = XK_Super_R;
    const unsigned int keyMENU       = XK_Menu;
    const unsigned int keyCTRLRIGHT  = XK_Control_R;
    const unsigned int keyARROWLEFT  = XK_Left;
    const unsigned int keyARROWDOWN  = XK_Down;
    const unsigned int keyARROWRIGHT = XK_Right;  
#endif

#if cimg_display_type==2 && cimg_OS==2
    // Keycodes for Windows-OS
    //@{
    //!\name Keycodes.
    /** 
    Keycodes are used to detect keyboard events occuring on display windows \c CImgDisplay.
    The field \c key of the \c CImgDisplay structure is updated at real-time with the corresponding keycode
    of the pressed key (or 0 if no keys have been pressed). The keycodes values are given by the variables
    whose names are of the form <tt>cimg::key*</tt>. Above is the keycode for the 'ESC' key, but
    almost all keycodes are thus defined.
        Using CImg-defined keycodes ensures a better portability of your program for other architectures.
    **/
    const unsigned int keyESC        = 27;
    const unsigned int keyF1         = 112;
    const unsigned int keyF2         = 113;
    const unsigned int keyF3         = 114;
    const unsigned int keyF4         = 115;
    const unsigned int keyF5         = 116;
    const unsigned int keyF6         = 117;
    const unsigned int keyF7         = 118;
    const unsigned int keyF8         = 119;
    const unsigned int keyF9         = 120;
    const unsigned int keyF10        = 121;
    const unsigned int keyF11        = 122;
    const unsigned int keyF12        = 123;
    const unsigned int keyPAUSE      = 19;
    const unsigned int key1          = 49;
    const unsigned int key2          = 50;
    const unsigned int key3          = 51;
    const unsigned int key4          = 52;
    const unsigned int key5          = 53;
    const unsigned int key6          = 54;
    const unsigned int key7          = 55;
    const unsigned int key8          = 56;
    const unsigned int key9          = 57;
    const unsigned int key0          = 48;
    const unsigned int keyBACKSPACE  = 8;
    const unsigned int keyINSERT     = 45;
    const unsigned int keyHOME       = 36;
    const unsigned int keyPAGEUP     = 33;
    const unsigned int keyTAB        = 9;
    const unsigned int keyQ          = 81;
    const unsigned int keyW          = 87;
    const unsigned int keyE          = 69;
    const unsigned int keyR          = 82;
    const unsigned int keyT          = 84;
    const unsigned int keyY          = 89;
    const unsigned int keyU          = 85;
    const unsigned int keyI          = 73;
    const unsigned int keyO          = 79;
    const unsigned int keyP          = 80;
    const unsigned int keyDELETE     = 8;
    const unsigned int keyEND        = 35;
    const unsigned int keyPAGEDOWN   = 34;
    const unsigned int keyCAPSLOCK   = 20;
    const unsigned int keyA          = 65;
    const unsigned int keyS          = 83;
    const unsigned int keyD          = 68;
    const unsigned int keyF          = 70;
    const unsigned int keyG          = 71;
    const unsigned int keyH          = 72;
    const unsigned int keyJ          = 74;
    const unsigned int keyK          = 75;
    const unsigned int keyL          = 76;
    const unsigned int keyENTER      = 13;
    const unsigned int keySHIFTLEFT  = 16;
    const unsigned int keyZ          = 90;
    const unsigned int keyX          = 88;
    const unsigned int keyC          = 67;
    const unsigned int keyV          = 86;
    const unsigned int keyB          = 66;
    const unsigned int keyN          = 78;
    const unsigned int keyM          = 77;
    const unsigned int keySHIFTRIGHT = 16;
    const unsigned int keyARROWUP    = 38;
    const unsigned int keyCTRLLEFT   = 17;
    const unsigned int keyAPPLEFT    = 91;
    const unsigned int keySPACE      = 32;
    const unsigned int keyALTGR      = 17;
    const unsigned int keyAPPRIGHT   = 92;
    const unsigned int keyMENU       = 93;
    const unsigned int keyCTRLRIGHT  = 17;
    const unsigned int keyARROWLEFT  = 37;
    const unsigned int keyARROWDOWN  = 40;
    const unsigned int keyARROWRIGHT = 39;
#endif
    //@}

#ifdef PI
#undef PI
#endif
    const double PI = 3.14159265358979323846;   //!< Definition of the mathematical constant PI
    const int infinity_int  = 0x7f800000;
    const double infinity = (double)*(float*)&cimg::infinity_int;
    
    // Definition of a 10x13 font (used in dialog boxes).
    const unsigned int font10x13[256*10*13/8] = {
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80100c0,
      0x68000300,0x801,0xc00010,0x100c000,0x68100,0x100c0680,0x2,0x403000,0x1000000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xfc,0x0,0x0,0x0,0x0,0x0,0x4020120,
      0x58120480,0x402,0x1205008,0x2012050,0x58080,0x20120581,0x40000001,0x804812,0x2000000,0x0,0x300,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x140,0x80000,0x200402,0x800000,0x10,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x7010,0x7000000,0x8000200,0x20000,0xc0002000,0x8008,0x0,0x0,0x0,0x0,0x808,0x4000000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x80000000,0x0,0x0,0x0,0x40000,0x0,0x0,0x0,0x0,0x480,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x70,0x80100c0,0x68000480,0x1001,
      0xc00010,0x1018000,0x68100,0x100c0680,0x4,0x403000,0x1020000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20140,0x28081883,0x200801,
      0x2a00000,0x10,0x1c0201c0,0x70040f80,0xc0f81c07,0x0,0x70,0x3e0303c0,0x3c3c0f83,0xe03c2107,0xe08810,0x18c31070,0x3c0703c0,
      0x783e0842,0x22222208,0x83e04010,0x1008000,0x4000200,0x20001,0x2002,0x408008,0x0,0x0,0x100000,0x0,0x1008,0x2000000,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20080,0x38000880,0x8078140f,0x81c00000,0x3e000,0xc020180,0x60080001,0xe0000002,0xc00042,0x108e2010,
      0xc0300c0,0x300c0303,0xf83c3e0f,0x83e0f81c,0x701c070,0x3c0c41c0,0x701c0701,0xc0001d08,0x42108421,0x8820088,0x4020120,0x58140480,
      0x802,0x1205008,0x3014050,0xc058080,0x20120581,0x40000002,0x804814,0x2020050,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20140,
      0x281e2484,0x80200801,0x1c02000,0x10,0x22060220,0x880c0801,0x82208,0x80000001,0x20008,0x41030220,0x40220802,0x402102,0x209010,
      0x18c31088,0x22088220,0x80080842,0x22222208,0x80204010,0x1014000,0x200,0x20001,0x2000,0x8008,0x0,0x0,0x100000,0x0,0x1008,
      0x2000000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,0x40000500,0x80800010,0x40200000,0x41000,0x12020040,0x10000003,0xa0000006,
      0x12000c4,0x31014000,0xc0300c0,0x300c0302,0x80402008,0x2008008,0x2008020,0x220c4220,0x88220882,0x20002208,0x42108421,0x8820088,
      0x0,0x300,0x0,0x0,0x0,0x14000000,0x0,0x200200,0x0,0x20000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20000,0xfc282504,0x80001000,
      0x82a02000,0x20,0x22020020,0x8140802,0x102208,0x80801006,0x18008,0x9c848220,0x80210802,0x802102,0x20a010,0x15429104,0x22104220,
      0x80080842,0x22221405,0x404008,0x1022000,0x703c0,0x381e0701,0xc0783c02,0xc09008,0x1d83c070,0x3c078140,0x381c0882,0x21242208,
      0x81e01008,0x2000000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x201e0,0x40220500,0x80800027,0x20e02800,0x9c800,0x12020040,
      0x20000883,0xa0200002,0x120a044,0x11064010,0x12048120,0x48120484,0x80802008,0x2008008,0x2008020,0x210a4411,0x4411044,0x10884508,
      0x42108421,0x503c0b0,0x1c0701c0,0x701c0707,0x70381c07,0x1c07008,0x2008020,0x20f01c0,0x701c0701,0xc0201c08,0x82208822,0x883c088,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20000,0x50281903,0x20001000,0x80802000,0x20,0x22020040,0x30240f03,0xc0101c08,0x80801018,
      0x1fc06010,0xa48483c0,0x80210f03,0xe0803f02,0x20c010,0x15429104,0x22104220,0x70080841,0x41540805,0x804008,0x1041000,0x8220,
      0x40220881,0x882202,0x40a008,0x12422088,0x22088180,0x40100882,0x21241408,0x80201008,0x2031000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x20280,0x401c0200,0x700028,0x21205000,0x92800,0xc1fc080,0x10000883,0xa0200002,0x1205049,0x12c19010,0x12048120,0x48120484,
      0xf0803c0f,0x3c0f008,0x2008020,0x790a4411,0x4411044,0x10504908,0x42108421,0x5022088,0x2008020,0x8020080,0x88402208,0x82208808,
      0x2008020,0x1e088220,0x88220882,0x20002608,0x82208822,0x8822088,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20000,0x501c0264,
      0xa0001000,0x8001fc00,0x7000020,0x22020080,0x83e0082,0x20202207,0x80000020,0x1020,0xa4848220,0x80210802,0x9c2102,0x20c010,
      0x12425104,0x3c1043c0,0x8080841,0x41540802,0x804008,0x1000000,0x78220,0x40220f81,0x882202,0x40c008,0x12422088,0x22088100,
      0x60100881,0x41540805,0x406008,0x1849000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20280,0xf0140200,0x880028,0x20e0a03f,0x709c800,
      0x201c0,0x60000881,0xa0000007,0xc0284b,0x122eb020,0x12048120,0x48120487,0x80802008,0x2008008,0x2008020,0x21094411,0x4411044,
      0x10204908,0x42108421,0x2022088,0x1e0781e0,0x781e0787,0xf8403e0f,0x83e0f808,0x2008020,0x22088220,0x88220882,0x21fc2a08,0x82208822,
      0x5022050,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20001,0xf80a0294,0x40001000,0x80002000,0x20,0x22020100,0x8040082,0x20202200,
      0x80000018,0x1fc06020,0xa48fc220,0x80210802,0x842102,0x20a010,0x12425104,0x20104240,0x8080841,0x41541402,0x1004008,0x1000000,
      0x88220,0x40220801,0x882202,0x40a008,0x12422088,0x22088100,0x18100881,0x41540805,0x801008,0x2046000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x0,0x20280,0x401c0f80,0x80880028,0x20005001,0x94800,0x20000,0x880,0xa0000000,0x5015,0x4215040,0x3f0fc3f0,0xfc3f0fc8,
      0x80802008,0x2008008,0x2008020,0x21094411,0x4411044,0x10505108,0x42108421,0x203c088,0x22088220,0x88220888,0x80402008,0x2008008,
      0x2008020,0x22088220,0x88220882,0x20002a08,0x82208822,0x5022050,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xa00a0494,0x60001000,
      0x80002004,0x8020,0x22020200,0x88040882,0x20402201,0x801006,0x18000,0x9f084220,0x40220802,0x442102,0x209010,0x10423088,0x20088220,
      0x8080840,0x80882202,0x2004008,0x1000000,0x88220,0x40220881,0x882202,0x409008,0x12422088,0x22088100,0x8100880,0x80881402,
      0x1001008,0x2000000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20280,0x40220200,0x80700027,0x20002801,0x92800,0x1fc000,0x980,
      0xa0000000,0xa017,0x84417840,0x21084210,0x84210848,0x80402008,0x2008008,0x2008020,0x2208c220,0x88220882,0x20882208,0x42108421,
      0x2020088,0x22088220,0x88220888,0xc8402208,0x82208808,0x2008020,0x22088220,0x88220882,0x20203208,0x82208822,0x2022020,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20000,0xa03c0463,0x90000801,0x2004,0x8040,0x1c0703e0,0x70040701,0xc0401c06,0x801001,0x20020,
      0x400843c0,0x3c3c0f82,0x3c2107,0x1c0881e,0x10423070,0x20070210,0xf0080780,0x80882202,0x3e04004,0x1000000,0x783c0,0x381e0701,
      0x782202,0x408808,0x12422070,0x3c078100,0x700c0780,0x80882202,0x1e01008,0x2000000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x201e0,
      0xf8000200,0x80080010,0x40000001,0x41000,0x0,0xe80,0xa0000000,0x21,0x8e21038,0x21084210,0x84210848,0xf83c3e0f,0x83e0f81c,
      0x701c070,0x3c08c1c0,0x701c0701,0xc0005c07,0x81e0781e,0x20200b0,0x1e0781e0,0x781e0787,0x30381c07,0x1c07008,0x2008020,0x1c0881c0,
      0x701c0701,0xc0201c07,0x81e0781e,0x203c020,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80000,0x801,0x4,0x40,0x0,0x0,0x0,0x1000,
      0x0,0x3c000000,0x0,0x0,0x0,0x0,0x10000,0x0,0x0,0x4004,0x1000000,0x0,0x0,0x80000,0x400000,0x0,0x20008000,0x0,0x4,0x1008,0x2000000,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,0x0,0x8008000f,0x80000000,0x3e000,0x0,0x800,0xa0000400,0x0,0x0,0x0,0x0,0x80000,0x0,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x100000,0x0,0x0,0x0,0x0,0x2000,0x0,0x4020040,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80000,
      0x402,0x8,0x40,0x0,0x0,0x0,0x2000,0x0,0x0,0x0,0x0,0x0,0x0,0xc000,0x0,0x0,0x7004,0x70000fc,0x0,0x0,0x700000,0x800000,0x0,0x20008000,
      0x0,0x4,0x808,0x4000000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,0x0,0x80f00000,0x0,0x0,0x0,0x800,0xa0001800,0x0,0x0,0x0,0x0,
      0x300000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x600000,0x0,0x0,0x0,0x0,0x0,0x0,0x4020040
    };
    
    // Definition of a 7x11 font, used to return a default font for drawing text.
    const unsigned int font7x11[7*11*256/8] = {
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80000000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,0x0,0x90,0x0,0x7f0000,0x40000,0x0,0x0,0x4010c0a4,0x82000040,0x11848402,0x18480050,0x80430292,0x8023,0x9008000,
      0x40218140,0x4000040,0x21800402,0x18000051,0x1060500,0x8083,0x10000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x24002,0x4031,0x80000000,0x10000,
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x7,0x81c0400,0x40020000,0x80070080,0x40440e00,0x0,0x0,0x1,0x88180000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x200000,0x0,0x0,0x80000,0x0,0x0,0x20212140,0x5000020,0x22400204,0x240000a0,0x40848500,0x4044,0x80010038,0x20424285,0xa000020,
      0x42428204,0x2428e0a0,0x82090a14,0x4104,0x85022014,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x10240a7,0x88484040,0x40800000,0x270c3,0x87811e0e,
      0x7c70e000,0x78,0x3c23c1ef,0x1f3e1e89,0xf1c44819,0xa23cf0f3,0xc3cff120,0xc18307f4,0x4040400,0x20000,0x80080080,0x40200,0x0,
      0x40000,0x2,0x8040000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x8188,0x50603800,0xf3c00000,0x1c004003,0xc700003e,0x18180,0xc993880,0x10204081,
      0x2071ef9,0xf3e7cf9f,0x3e7c7911,0xe3c78f1e,0x7d1224,0x48906048,0x0,0x4000000,0x0,0x9000,0x0,0x0,0x2000,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x10240aa,0x14944080,0x23610000,0x68940,0x40831010,0x8891306,0x802044,0x44522208,0x90202088,0x40448819,0xb242890a,0x24011111,
      0x49448814,0x4040a00,0xe2c3c7,0x8e3f3cb9,0xc1c44216,0xee38b0f2,0xe78f9120,0xc18507e2,0x8040000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x101c207,0x88a04001,0x9c00000,0x2200a041,0x8200113a,0x8240,0x50a3110,0x2850a142,0x850c2081,0x2040204,0x8104592,0x142850a1,
      0x42cd1224,0x4888bc48,0x70e1c387,0xe3b3c70,0xe1c38e1c,0x38707171,0xc3870e1c,0x10791224,0x48906c41,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x10003ee,0x15140080,0x21810000,0x48840,0x40851020,0x8911306,0x31fd804,0x9c522408,0x90204088,0x4045081a,0xba42890a,0x24011111,
      0x49285024,0x2041b00,0x132408,0x910844c8,0x4044821b,0x7244c913,0x24041111,0x49488822,0x8040000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x28204,0x85006001,0x6a414000,0x3a004043,0xc700113a,0x8245,0x50a3a00,0x2850a142,0x850c4081,0x2040204,0x81045d2,0x142850a1,
      0x24951224,0x48852250,0x8102040,0x81054089,0x12244204,0x8108992,0x24489122,0x991224,0x4888b222,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x1000143,0xa988080,0x2147c01f,0x88840,0x83091c2c,0x1070f000,0xc000608,0xa48bc408,0x9e3c46f8,0x40460816,0xaa42f10b,0xc3811111,
      0x35102044,0x1041100,0xf22408,0x9f084488,0x40470212,0x62448912,0x6041111,0x55308846,0x8061c80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x1028704,0x8f805801,0x4be28fdf,0x220001f0,0x111a,0x60000182,0x82c5c710,0x44891224,0x489640f1,0xe3c78204,0x810e552,0x142850a1,
      0x18a51224,0x48822250,0x78f1e3c7,0x8f1f40f9,0xf3e7c204,0x8108912,0x24489122,0x7ea91224,0x4888a222,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x10007e2,0x85648080,0x20010000,0x88841,0x8f8232,0x20881000,0xc1fc610,0xbefa2408,0x90204288,0x40450816,0xa642810a,0x4041110a,
      0x36282084,0x1042080,0x1122408,0x90084488,0x40450212,0x62448912,0x184110a,0x55305082,0x8042700,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x1028207,0x82004801,0x68050040,0x1c000040,0x110a,0x60000001,0x45484d10,0x7cf9f3e7,0xcf944081,0x2040204,0x8104532,0x142850a1,
      0x18a51224,0x48822248,0x89122448,0x91244081,0x2040204,0x8108912,0x24489122,0xc91224,0x48852214,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x282,
      0x89630080,0x20010c00,0x30108842,0x810222,0x20882306,0x3001800,0x408a2208,0x90202288,0x40448814,0xa642810a,0x2041110a,0x26442104,
      0x840000,0x1122408,0x90084488,0x40448212,0x62448912,0x84130a,0x36485102,0x8040000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x101c208,0x4f802801,
      0x8028040,0x40,0x130a,0x2,0x85e897a0,0x44891224,0x489c2081,0x2040204,0x8104532,0x142850a1,0x24cd1224,0x48823c44,0x89122448,
      0x91244081,0x2040204,0x8108912,0x24489122,0xc93264,0xc9852214,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x100028f,0x109f0080,0x20010c00,
      0x303071f3,0xc7011c1c,0x4071c306,0x802010,0x3907c1ef,0x1f201e89,0xf3844f90,0xa23c80f2,0x17810e04,0x228223f4,0x840000,0xfbc3c7,
      0x8f083c88,0x40444212,0x6238f0f2,0x7039d04,0x228423e2,0x8040000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1008780,0x2201800,0xf0014000,0x1f0,
      0x1d0a,0x5,0x851e140,0x83060c18,0x30671ef9,0xf3e7cf9f,0x3e7c7911,0xe3c78f1e,0x42f8e1c3,0x8702205c,0x7cf9f3e7,0xcf9b3c78,0xf1e3c204,
      0x8107111,0xc3870e1c,0x10f1d3a7,0x4e823c08,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2,0x40,0x40000400,0x200000,0x0,0x2,0x0,0x0,0x0,0x0,0x18,
      0x0,0x4,0x44007f,0x0,0x400,0x400000,0x8010,0x0,0x6002,0x8040000,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1000000,0x200800,0x0,0x0,0x100a,
      0x400000,0x44,0x0,0x400,0x0,0x0,0x0,0x0,0x0,0x0,0x800,0x0,0x0,0x0,0x0,0x62018,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x31,0x80000800,
      0x400000,0x0,0x4,0x0,0x0,0x0,0x0,0xc,0x0,0x7,0x3c0000,0x0,0x3800,0x3800000,0x8010,0x0,0x1c001,0x881c0000,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x207000,0x0,0x0,0x100a,0xc00000,0x3c,0x0,0xc00,0x0,0x0,0x0,0x0,0x0,0x0,0x1800,0x0,0x0,0x0,0x0,0x1c2070
    };

    // Definition of a 40x38 'danger' color logo
    const unsigned char logo40x38[4576] = {
      177,200,200,200,3,123,123,0,36,200,200,200,1,123,123,0,2,255,255,0,1,189,189,189,1,0,0,0,34,200,200,200,
      1,123,123,0,4,255,255,0,1,189,189,189,1,0,0,0,1,123,123,123,32,200,200,200,1,123,123,0,5,255,255,0,1,0,0,
      0,2,123,123,123,30,200,200,200,1,123,123,0,6,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,29,200,200,200,
      1,123,123,0,7,255,255,0,1,0,0,0,2,123,123,123,28,200,200,200,1,123,123,0,8,255,255,0,1,189,189,189,1,0,0,0,
      2,123,123,123,27,200,200,200,1,123,123,0,9,255,255,0,1,0,0,0,2,123,123,123,26,200,200,200,1,123,123,0,10,255,
      255,0,1,189,189,189,1,0,0,0,2,123,123,123,25,200,200,200,1,123,123,0,3,255,255,0,1,189,189,189,3,0,0,0,1,189,
      189,189,3,255,255,0,1,0,0,0,2,123,123,123,24,200,200,200,1,123,123,0,4,255,255,0,5,0,0,0,3,255,255,0,1,189,
      189,189,1,0,0,0,2,123,123,123,23,200,200,200,1,123,123,0,4,255,255,0,5,0,0,0,4,255,255,0,1,0,0,0,2,123,123,123,
      22,200,200,200,1,123,123,0,5,255,255,0,5,0,0,0,4,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,21,200,200,200,
      1,123,123,0,5,255,255,0,5,0,0,0,5,255,255,0,1,0,0,0,2,123,123,123,20,200,200,200,1,123,123,0,6,255,255,0,5,0,0,
      0,5,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,19,200,200,200,1,123,123,0,6,255,255,0,1,123,123,0,3,0,0,0,1,
      123,123,0,6,255,255,0,1,0,0,0,2,123,123,123,18,200,200,200,1,123,123,0,7,255,255,0,1,189,189,189,3,0,0,0,1,189,
      189,189,6,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,17,200,200,200,1,123,123,0,8,255,255,0,3,0,0,0,8,255,255,
      0,1,0,0,0,2,123,123,123,16,200,200,200,1,123,123,0,9,255,255,0,1,123,123,0,1,0,0,0,1,123,123,0,8,255,255,0,1,189,
      189,189,1,0,0,0,2,123,123,123,15,200,200,200,1,123,123,0,9,255,255,0,1,189,189,189,1,0,0,0,1,189,189,189,9,255,255,
      0,1,0,0,0,2,123,123,123,14,200,200,200,1,123,123,0,11,255,255,0,1,0,0,0,10,255,255,0,1,189,189,189,1,0,0,0,2,123,
      123,123,13,200,200,200,1,123,123,0,23,255,255,0,1,0,0,0,2,123,123,123,12,200,200,200,1,123,123,0,11,255,255,0,1,189,
      189,189,2,0,0,0,1,189,189,189,9,255,255,0,1,189,189,189,1,0,0,0,2,123,123,123,11,200,200,200,1,123,123,0,11,255,255,
      0,4,0,0,0,10,255,255,0,1,0,0,0,2,123,123,123,10,200,200,200,1,123,123,0,12,255,255,0,4,0,0,0,10,255,255,0,1,189,189,
      189,1,0,0,0,2,123,123,123,9,200,200,200,1,123,123,0,12,255,255,0,1,189,189,189,2,0,0,0,1,189,189,189,11,255,255,0,1,
      0,0,0,2,123,123,123,9,200,200,200,1,123,123,0,27,255,255,0,1,0,0,0,3,123,123,123,8,200,200,200,1,123,123,0,26,255,
      255,0,1,189,189,189,1,0,0,0,3,123,123,123,9,200,200,200,1,123,123,0,24,255,255,0,1,189,189,189,1,0,0,0,4,123,123,
      123,10,200,200,200,1,123,123,0,24,0,0,0,5,123,123,123,12,200,200,200,27,123,123,123,14,200,200,200,25,123,123,123,86,
      200,200,200,91,49,124,118,124,71,32,124,95,49,56,114,52,82,121,0};
  
    // Return a 'stringification' of standart integral types.
    const char* const bool_st    = "bool";
    const char* const uchar_st   = "unsigned char";
    const char* const char_st    = "char";
    const char* const ushort_st  = "unsigned short";
    const char* const short_st   = "short";
    const char* const uint_st    = "unsigned int";
    const char* const int_st     = "int";
    const char* const ulong_st   = "unsigned long";
    const char* const long_st    = "long";
    const char* const float_st   = "float";
    const char* const double_st  = "double";
    const char* const unknown_st = "unknown";
    template<typename t> inline const char* get_type(const t&) { return cimg::unknown_st; }
    inline const char* get_type(const bool&          ) { return cimg::bool_st;   }
    inline const char* get_type(const unsigned char& ) { return cimg::uchar_st;  }
    inline const char* get_type(const char&          ) { return cimg::char_st;   }
    inline const char* get_type(const unsigned short&) { return cimg::ushort_st; }
    inline const char* get_type(const short&         ) { return cimg::short_st;  }
    inline const char* get_type(const unsigned int&  ) { return cimg::uint_st;   }
    inline const char* get_type(const int&           ) { return cimg::int_st;    }
    inline const char* get_type(const unsigned long& ) { return cimg::ulong_st;  }
    inline const char* get_type(const long&          ) { return cimg::long_st;   }
    inline const char* get_type(const float&         ) { return cimg::float_st;  }
    inline const char* get_type(const double&        ) { return cimg::double_st; }
    
    //! Return an approximation of the minimum value of a type (necessary because of buggy <limits> on VC 6.0)
    template<typename t> inline const t get_type_min(const t&) {
      static const double p = std::pow(2.0,8.0*sizeof(t)-1.0);
      static const t res = (t)(((t)-1)>=0?0:(-p)); 
      return res;
    }
    inline const float get_type_min(const float&)   { return -(float)cimg::infinity; }
    inline const double get_type_min(const double&) { return -cimg::infinity; }

    //! Return an approximation of the maximum value of a type (necessary because of buggy <limits> on VC 6.0)
    template<typename t> inline const t get_type_max(const t&) {
      static const double p = std::pow(2.0,8.0*sizeof(t)-1.0);
      static const t res = (t)(((t)-1)>=0?(2*p-1):(p-1));
      return res;
    }
    inline const float get_type_max(const float&)   { return (float)cimg::infinity; }
    inline const double get_type_max(const double&) { return cimg::infinity; }
                                        
    // Display a warning message
#if cimg_debug>=1    
    static void warn(const bool cond,const char *format,...) {
      if (cond) {
        std::va_list ap;
        va_start(ap,format);
        std::fprintf(stderr,"<CImg Warning> ");
        std::vfprintf(stderr,format,ap);
        std::fputc('\n',stderr);
        va_end(ap);
      }
    }
#else
    inline void warn(const bool cond,const char *format,...) {}
#endif

    inline int xln(const int x) { return x>0?(int)(1+std::log10((double)x)):1; }
    inline char uncase(const char x) { return (char)((x<'A'||x>'Z')?x:x-'A'+'a'); }
    inline float atof(const char *str) {
      float x=0,y=1;
      if (!str) return 0; else { std::sscanf(str,"%g/%g",&x,&y); return x/y; }
    }
    inline int strlen(const char *s) { if (s) { int k; for (k=0; s[k]; k++) ; return k; } return -1; }
    inline int strncmp(const char *s1,const char *s2,const int l) {
      if (s1 && s2) { int n=0; for (int k=0; k<l; k++) n+=std::abs(s1[k] - s2[k]); return n; }
      return 0;
    }
    inline int strncasecmp(const char *s1,const char *s2,const int l) {
      if (s1 && s2) { int n=0; for (int k=0; k<l; k++) n+=std::abs(uncase(s1[k])-uncase(s2[k])); return n; }
      return 0;
    }
    inline int strcmp(const char *s1,const char *s2) { 
      const int l1 = cimg::strlen(s1), l2 = cimg::strlen(s2);
      return cimg::strncmp(s1,s2,1+(l1<l2?l1:l2));
    }
    inline int strcasecmp(const char *s1,const char *s2) { 
      const int l1 = cimg::strlen(s1), l2 = cimg::strlen(s2);
      return cimg::strncasecmp(s1,s2,1+(l1<l2?l1:l2));
    }
    inline int strfind(const char *s,const char c) {
      if (s) { 
        int l; for (l=cimg::strlen(s); l>=0 && s[l]!=c; l--) ;
        return l; 
      }
      return -1; 
    }
    inline const char* basename(const char *s)  {
      return (cimg_OS!=2)?(s?s+1+cimg::strfind(s,'/'):NULL):(s?s+1+cimg::strfind(s,'\\'):NULL); 
    }

    inline void system(const char *command) {
#if cimg_OS==2
      PROCESS_INFORMATION pi;
      STARTUPINFO si;
      GetStartupInfo(&si);
      si.wShowWindow = SW_HIDE;
      si.dwFlags |= SW_HIDE;
      BOOL res = CreateProcess(NULL,(LPTSTR)command,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
      if (res) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
      }
#else
      std::system(command);
#endif
    }
    
    //! Return the path of the ImageMagick's <tt>convert</tt> tool.
    /**
       If you have installed the <a href="http://www.imagemagick.org">ImageMagick package</a>
       in a standart directory, this function returns the correct path of the \c convert tool
       used to load and save compressed image formats.
       Conversely, if the \c convert executable is not auto-detected by the function,
       you can define the macro \c cimg_convert_path with the correct path
       of the \c convert executable, before including <tt>"CImg.h"</tt> in your program :
       \code
       #define cimg_convert_path "/users/thatsme/local/bin/convert"
       #include "CImg.h"
       
       int main() {
         CImg<> img("my_image.jpg");
     return 0;
       }
       \endcode
       \note \c convert is needed to read and write compressed image formats. Other formats do not need \c convert.
       \see temporary_path, CImg::load_convert, CImg::save_convert.
    **/
    inline const char* convert_path() {
      static char *convert_path = NULL;
      if (!convert_path) {
#if cimg_OS==2 || defined(cimg_convert_path)
        bool stopflag = false;
        std::FILE *file;
#endif
        convert_path = new char[1024];
#ifdef cimg_convert_path
        std::strcpy(convert_path,cimg_convert_path);
        if ((file=std::fopen(convert_path,"r"))!=NULL) { std::fclose(file); stopflag = true; }
#endif
#if cimg_OS==2
        for (unsigned int k=0; k<=9 && !stopflag; k++) {
          std::sprintf(convert_path,"C:\\PROGRA~1\\IMAGEM~1.%u-Q\\convert.exe",k);
          if ((file=std::fopen(convert_path,"r"))!=NULL) { std::fclose(file); stopflag = true; }
        }
        if (!stopflag) for (unsigned int k=0; k<=9 && !stopflag; k++) {
          std::sprintf(convert_path,"C:\\PROGRA~1\\IMAGEM~1.%u\\convert.exe",k);
          if ((file=std::fopen(convert_path,"r"))!=NULL) { std::fclose(file); stopflag = true; }
        }
        if (!stopflag) std::strcpy(convert_path,"convert.exe");
#else
        std::strcpy(convert_path,"convert");
#endif
      }
      return convert_path;
    }
    
    //! Return a path to store temporary files.
    /**
       If you are running on a standart Unix or Windows system, this function should return a correct path
       where temporary files can be stored.
       If the path is not auto-detected, you need to define the macro \c cimg_temporary_path,
       before including <tt>"CImg.h"</tt> in your program :
       \code
       #define cimg_temporary_path "/users/toto/tmp"
       #include "CImg.h"

       int main() {
         CImg<> img("my_image.jpg");
     return 0;
       }
       \endcode
       \note A temporary path is necessary to load and save compressed image formats, using \c convert.
       \see convert_path, CImg::load_convert, CImg::save_convert.
    **/
    inline const char* temporary_path() {
      static char *st_temporary_path = NULL;
      if (!st_temporary_path) {
        st_temporary_path = new char[1024];
#ifdef cimg_temporary_path
        std::strcpy(st_temporary_path,cimg_temporary_path);
        const char* testing_path[7] = { st_temporary_path, "/tmp","C:\\WINNT\\Temp", "C:\\WINDOWS\\Temp","","C:",NULL };
#else
        const char* testing_path[6] = { "/tmp","C:\\WINNT\\Temp", "C:\\WINDOWS\\Temp","","C:",NULL };
#endif
        char filetmp[1024];
        std::FILE *file=NULL;
        int i=-1;
        while (!file && testing_path[++i]) {
          std::sprintf(filetmp,"%s/CImg%.4d.ppm",testing_path[i],std::rand()%10000);
          if ((file=std::fopen(filetmp,"w"))!=NULL) { std::fclose(file); std::remove(filetmp); }
        }
        if (!file) 
      throw CImgIOException("cimg::temporary_path() : Unable to find a temporary path accessible for writing\n"
                "you have to set the macro 'cimg_temporary_path' to a valid path where you have writing access :\n"
                "#define cimg_temporary_path \"path\" (before including 'CImg.h')");
        std::strcpy(st_temporary_path,testing_path[i]);
      }
      return st_temporary_path;
    }
    
    inline const char *filename_split(const char *const filename, char *const body=NULL) {
      if (!filename) throw CImgArgumentException("cimg::filename_split() : Can't split the (null) filename");
      int l = cimg::strfind(filename,'.');
      if (l>=0) { if (body) { std::strncpy(body,filename,l); body[l]='\0'; }}
      else { if (body) std::strcpy(body,filename); l=(int)std::strlen(filename)-1; }
      return filename+l+1;
    }
    
    inline char* filename_number(const char *filename,const int number,const unsigned int n,char *const string) {
      char format[1024],body[1024];
      const char *ext = cimg::filename_split(filename,body);
      if (n>0) std::sprintf(format,"%s_%%.%ud.%s",body,n,ext);
      else std::sprintf(format,"%s_%%d.%s",body,ext);
      std::sprintf(string,format,number);
      return string;
    }
    inline std::FILE *fopen(const char *const path,const char *const mode) {
      if(!path || !mode) throw CImgArgumentException("cimg::fopen() : Can't open file '%s' with mode '%s'",path,mode);
      if (path[0]=='-') return (mode[0]=='r')?stdin:stdout; else {
        std::FILE *dest=std::fopen(path,mode);
        if(!dest) throw CImgIOException("cimg::fopen() : File '%s' cannot be opened %s",
                                        path,mode[0]=='r'?"for reading":(mode[0]=='w'?"for writing":""),path);
        return dest;
      }
    }
    inline int fclose(std::FILE *file) {
      warn(!file,"cimg::fclose() : Can't close (null) file");
      if (!file || file==stdin || file==stdout) return 0;
      const int errn=std::fclose(file);
      warn(errn!=0,"cimg::fclose() : Error %d during file closing",errn);
      return errn;
    }
    template<typename T> inline int fread(T *ptr,const unsigned int size,const unsigned int nmemb,std::FILE *stream) {
      if (!ptr || size<=0 || nmemb<=0 || !stream)
        throw CImgArgumentException("cimg::fread() : Can't read %u x %u bytes of file pointer '%p' in buffer '%p'",
                    nmemb,size,stream,ptr);
      const unsigned int errn = (unsigned int)std::fread((void*)ptr,size,nmemb,stream);
      cimg::warn(errn!=nmemb,"cimg::fread() : File reading problems, only %u/%u elements read",errn,nmemb);
      return errn;
    }
    inline int fwrite(const void *ptr,const unsigned int size,const unsigned int nmemb,std::FILE *stream) {
      if (!ptr || size<=0 || nmemb<=0 || !stream)
        throw CImgArgumentException("cimg::fwrite() : Can't write %u x %u bytes of file pointer '%p' from buffer '%p'",
                    nmemb,size,stream,ptr);
      const unsigned int errn = (unsigned int)std::fwrite(ptr,size,nmemb,stream);
      if(errn!=nmemb)
        throw CImgIOException("cimg::fwrite() : File writing problems, only %u/%u elements written",errn,nmemb);
      return errn;
    }
    
    // Exchange the values of variables \p a and \p b
    template<typename T> inline void swap(T& a,T& b) { T t=a; a=b; b=t; }
    template<typename T> inline void swap(T& a1,T& b1,T& a2,T& b2) {
      cimg::swap(a1,b1); cimg::swap(a2,b2); 
    }
    template<typename T> inline void swap(T& a1,T& b1,T& a2,T& b2,T& a3,T& b3) {
      cimg::swap(a1,b1,a2,b2); cimg::swap(a3,b3); 
    }
    template<typename T> inline void swap(T& a1,T& b1,T& a2,T& b2,T& a3,T& b3,T& a4,T& b4) {
      cimg::swap(a1,b1,a2,b2,a3,b3); cimg::swap(a4,b4); 
    }
    template<typename T> inline void swap(T& a1,T& b1,T& a2,T& b2,T& a3,T& b3,T& a4,T& b4,T& a5,T& b5) {
      cimg::swap(a1,b1,a2,b2,a3,b3,a4,b4); cimg::swap(a5,b5); 
    }
    template<typename T> inline void swap(T& a1,T& b1,T& a2,T& b2,T& a3,T& b3,T& a4,T& b4,T& a5,T& b5,T& a6,T& b6) {
      cimg::swap(a1,b1,a2,b2,a3,b3,a4,b4,a5,b5); cimg::swap(a6,b6);
    }
    
    template<typename T> inline T& endian_swap(T& a) {  
      if (sizeof(a)!=1) {
    unsigned char *pb=(unsigned char*)&a, *pe=pb+sizeof(a);
    for (int i=0; i<(int)sizeof(a)/2; i++) cimg::swap(*(pb++),*(--pe));
      }
      return a;
    }

    template<typename T> inline void endian_swap(T *const buffer,const unsigned int size) {
      T *ptr = buffer;
      for (unsigned int i=0; i<size; i++) cimg::endian_swap(*(ptr++));
    }

    inline const char* option(const char *const name,const unsigned int argc,char **argv,const char *const defaut,
                              const char *const usage=NULL) {
      static bool first=true, visu=false;
      const char *res = NULL;
      if (first) { first=false; visu = cimg::option("-h",argc,argv,(const char*)NULL)!=NULL; }
      if (!name && visu) {
        std::fprintf(stderr,"\n %s%s%s",cimg::t_red,cimg::basename(argv[0]),cimg::t_normal);
        if (usage) std::fprintf(stderr," : %s",usage);
        std::fprintf(stderr," (%s, %s)\n\n",__DATE__,__TIME__);
      }
      if (name) {
        if (argc>0) {
          unsigned int k=0,i;
          while (k<argc && cimg::strcmp(argv[k],name)) k++;
          i=k;
          res=(k++==argc?defaut:(k==argc?argv[--k]:argv[k]));
        } else res = defaut;
        if (visu && usage) std::fprintf(stderr,"    %s%-8s%s = %-12s : %s%s%s\n",
                                        cimg::t_bold,name,cimg::t_normal,res?res:"NULL",cimg::t_purple,usage,cimg::t_normal);
      }
      return res;
    }

    inline bool option(const char *const name,const unsigned int argc,char **argv,
                       const bool defaut,const char *const usage=NULL) {
      const char *s = cimg::option(name,argc,argv,(const char*)NULL);
      const bool res = s?(cimg::strcasecmp(s,"false") && cimg::strcasecmp(s,"off") && cimg::strcasecmp(s,"0")):defaut;
      cimg::option(name,0,NULL,res?"true":"false",usage);
      return res;
    }

    inline int option(const char *const name,const unsigned int argc,char **argv,
                      const int defaut,const char *const usage=NULL) {
      const char *s = cimg::option(name,argc,argv,(const char*)NULL);
      const int res = s?std::atoi(s):defaut;
      char tmp[256];
      std::sprintf(tmp,"%d",res);
      cimg::option(name,0,NULL,tmp,usage);
      return res;
    }

    inline char option(const char *const name,const unsigned int argc,char **argv,
               const char defaut,const char *const usage=NULL) {
      const char *s = cimg::option(name,argc,argv,(const char*)NULL);
      const char res = s?s[0]:defaut;
      char tmp[8];
      tmp[0] = res;
      tmp[1] ='\0';
      cimg::option(name,0,NULL,tmp,usage);
      return res;
    }

    inline double option(const char *const name,const unsigned int argc,char **argv,
                         const double defaut,const char *const usage=NULL) {
      const char *s = cimg::option(name,argc,argv,(const char*)NULL);
      const double res = s?cimg::atof(s):defaut;
      char tmp[256];
      std::sprintf(tmp,"%g",res);
      cimg::option(name,0,NULL,tmp,usage);
      return res;
    }
    
    //! Return \c false for little endian CPUs (Intel), \c true for big endian CPUs (Motorola).
    inline const bool endian() { const int x=1; return ((unsigned char*)&x)[0]?false:true; }

    //! Display informations about CImg compilation variables, on the standart error output \e stderr.
    inline void info() {
      std::fprintf(stderr,"\n %sCImg Library %g%s, compiled %s ( %s ) with the following flags :\n\n",
                   cimg::t_red,cimg_version,cimg::t_normal,__DATE__,__TIME__);
      std::fprintf(stderr,"  > Architecture   : %s%-12s%s %s(cimg_OS=%d)\n%s",
                   cimg::t_bold,
                   cimg_OS==1?"Unix":(cimg_OS==2?"Windows":"Unknown"),
                   cimg::t_normal,cimg::t_purple,cimg_OS,cimg::t_normal);
      std::fprintf(stderr,"  > Display type   : %s%-12s%s %s(cimg_display_type=%d)%s\n",
                   cimg::t_bold,cimg_display_type==0?"No":
           (cimg_display_type==1?"X11":
            (cimg_display_type==2?"WindowsGDI":
             "Unknown")),
           cimg::t_normal,cimg::t_purple,cimg_display_type,cimg::t_normal);
#ifdef cimg_color_terminal
      std::fprintf(stderr,"  > Color terminal : %s%-12s%s %s(cimg_color_terminal defined)%s\n",
           cimg::t_bold,"Yes",cimg::t_normal,cimg::t_purple,cimg::t_normal);
#else
      std::fprintf(stderr,"  > Color terminal : %-12s (cimg_color_terminal undefined)\n","No");
#endif
      std::fprintf(stderr,"  > Debug messages : %s%-12s%s %s(cimg_debug=%d)%s\n",cimg::t_bold,
           cimg_debug==2?"High":(cimg_debug==1?"Yes":"No"),
                   cimg::t_normal,cimg::t_purple,cimg_debug,cimg::t_normal);
      std::fprintf(stderr,"\n");
    }
    
    //! Get the value of a system timer with a millisecond precision.
    inline long time() {
#if cimg_OS==1
      struct timeval st_time;
      gettimeofday(&st_time,NULL);
      return (long)(st_time.tv_usec/1000 + st_time.tv_sec*1000);
#elif cimg_OS==2
      static SYSTEMTIME st_time;
      GetSystemTime(&st_time);
      return (long)(st_time.wMilliseconds + 1000*(st_time.wSecond + 60*(st_time.wMinute + 60*st_time.wHour)));
#else 
      return 0;
#endif
    }

    //! Sleep for a certain numbers of milliseconds.
    /**
       This function frees the CPU ressources during the sleeping time.
       May be used to temporize your program properly, without wasting CPU time.
       \see wait, time.
    **/
    inline void sleep(const int milliseconds) {
#if cimg_OS==1
      struct timespec tv;
      tv.tv_sec = milliseconds/1000;
      tv.tv_nsec = (milliseconds%1000)*1000000;
      nanosleep(&tv,NULL);
#elif cimg_OS==2
      Sleep(milliseconds);
#endif
    }

    //! Wait for a certain number of milliseconds since the last call of \ref wait().
    /**
       If the desired delay has expired, this function returns immediately else it sleeps till the correct time.
       May be used to temporize your program properly.
       \see sleep, time.
    **/
    inline long wait(const int milliseconds=20,long reference_time=-1) {
      static long latest_time = cimg::time();
      if (reference_time>=0) latest_time = reference_time;
      const long current_time = cimg::time(), time_diff = milliseconds + latest_time - current_time;
      if (time_diff>0) { cimg::sleep(time_diff); return (latest_time = current_time + time_diff); }
      else return (latest_time = current_time);
    }

    //! Bitwise rotation on the left
    template<typename T> inline const T rol(const T& a,const unsigned int n=1) { return (a<<n)|(a>>((sizeof(T)<<3)-n)); }
    //! Bitwise rotation on the right
    template<typename T> inline const T ror(const T& a,const unsigned int n=1) { return (a>>n)|(a<<((sizeof(T)<<3)-n)); }

#if ( !defined(_MSC_VER) || _MSC_VER>1200 )
    //! Return the absolute value of \p a
    template<typename T> inline const T abs(const T& a) { return a>=0?a:-a; }
    inline const double abs(const double& a) { return std::fabs(a); }
    inline const float abs(const float& a)   { return (float)std::fabs((double)a); }
    inline const int abs(const int& a)       { return std::abs(a); }
    
    //! Return the minimum between \p a and \p b.
    template<typename T> inline const T& min(const T& a,const T& b) { return a<=b?a:b; }
    //! Return the minimum between \p a,\p b and \a c.
    template<typename T> inline const T& min(const T& a,const T& b,const T& c) { return cimg::min(cimg::min(a,b),c); }
    //! Return the minimum between \p a,\p b,\p c and \p d.
    template<typename T> inline const T& min(const T& a,const T& b,const T& c,const T& d) { return cimg::min(cimg::min(a,b,c),d); }
    //! Return the maximum between \p a and \p b.
    template<typename T> inline const T& max(const T& a,const T& b) { return a>=b?a:b; }
    //! Return the maximum between \p a,\p b and \p c.
    template<typename T> inline const T& max(const T& a,const T& b,const T& c) { return cimg::max(cimg::max(a,b),c); }
    //! Return the maximum between \p a,\p b,\p c and \p d.
    template<typename T> inline const T& max(const T& a,const T& b,const T& c,const T& d) { return cimg::max(cimg::max(a,b,c),d); }
    //! Return the sign of \p x.
    template<typename T> inline char sign(const T& x) { return (x<0)?-1:(x==0?0:1); }
#else
    // Special versions due to object reference bug in VisualC++ 6.0.
    template<typename T> inline const T abs(const T a) { return a>=0?a:-a; }
    template<typename T> inline const T min(const T a,const T b) { return a<=b?a:b; }
    template<typename T> inline const T min(const T a,const T b,const T c) { return cimg::min(cimg::min(a,b),c); }
    template<typename T> inline const T min(const T a,const T b,const T c,const T& d) { return cimg::min(cimg::min(a,b,c),d); }
    template<typename T> inline const T max(const T a,const T b) { return a>=b?a:b; }
    template<typename T> inline const T max(const T a,const T b,const T c) { return cimg::max(cimg::max(a,b),c); }
    template<typename T> inline const T max(const T a,const T b,const T c,const T& d) { return cimg::max(cimg::max(a,b,c),d); }
    template<typename T> inline char sign(const T x) { return (x<0)?-1:(x==0?0:1); }
#endif

    //! Return \p x modulo \p m (generic modulo).
    /**
       This modulo function accepts negative and floating-points modulo numbers \p m.
    **/
    inline double mod(const double& x,const double& m) { return x-m*std::floor(x/m); }
    inline float  mod(const float& x,const float& m)   { return (float)(x-m*std::floor((double)x/m)); }
    inline int    mod(const int x,const int m)         { return x>=0?x%m:(x%m?m+x%m:0); }

    //! Return minmod(\p a,\p b).
    /**
       The operator minmod(\p a,\p b) is defined to be :
       - minmod(\p a,\p b) = min(\p a,\p b), if (\p a * \p b)>0.
       - minmod(\p a,\p b) = 0,              if (\p a * \p b)<=0
    **/
    template<typename T> inline T minmod(const T& a,const T& b) { return a*b<=0?0:(a>0?(a<b?a:b):(a<b?b:a)); }
    //! Return a random variable between [0,1] (uniform distribution).
    inline double rand() { return (double)std::rand()/RAND_MAX; }
    //! Return a random variable between [-1,1] (uniform distribution).
    inline double crand() { return 1-2*cimg::rand(); }
    //! Return a random variable using a gaussian distribution and a standart deviation of 1.
    inline double grand() {
      return std::sqrt(-2*std::log((double)(1e-10 + (1-2e-10)*cimg::rand())))*std::cos((double)(2*PI*cimg::rand())); 
    }

    // Return (a^2+b^2)^0.5
    inline double pythagore(double a, double b) {
      const double absa = cimg::abs(a), absb = cimg::abs(b);
      if (absa>absb) { const double tmp = absb/absa; return absa*std::sqrt(1.0+tmp*tmp); }
      else { const double tmp = absa/absb; return (absb==0?0:absb*std::sqrt(1.0+tmp*tmp)); }
    }
  }

  /*-------------------------------------------------------
    
  
  
  
    Definition of the CImgStats structure
  
  
  
    
    ------------------------------------------------------*/
  //! This class is used to compute basics statistics of a <tt>CImg<T></tt> image.
  /** 
      Constructing a CImgStats instance by passing an image CImg<T> or an image list CImgl<T> as a parameter,
      will compute the minimum, the maximum and the average pixel values of the given object, and optionally
      the variance of the pixel values. Use it to retrieve basic statistics of an image, or an image list,
      like this :
      \code 
      const CImg<float> img("my_image.jpg");
      const CImgStats stats(img);
      stats.print("My statistics");
      std::printf("Max-Min = %lf",stats.max-stats.min);
      \endcode
      Note that statistics are computed for all scalar values of a CImg<T> or a CImgl<T>. No vector-valued
      statistics are performed.
  **/
  struct CImgStats {
    double min;                 //!< Minimum of the pixel values after statistics computation.
    double max;                 //!< Maximum of the pixel values after statistics computation.
    double mean;                //!< Mean of the pixel values after statistics computation.
    double variance;            //!< Variance of the pixel values after statistics computation.
    int xmin;                   //!< X-coordinate of the minimum pixel value.
    int ymin;                   //!< Y-coordinate of the minimum pixel value.
    int zmin;                   //!< Z-coordinate of the minimum pixel value.
    int vmin;                   //!< V-coordinate of the minimum pixel value.
    int lmin;                   //!< Image number of the list where the minimum is reached.
    int xmax;                   //!< X-coordinate of the maximum pixel value.
    int ymax;                   //!< Y-coordinate of the maximum pixel value.
    int zmax;                   //!< Z-coordinate of the maximum pixel value.
    int vmax;                   //!< V-coordinate of the maximum pixel value.
    int lmax;                   //!< Image number of the list where the maximum is reached.

    //! Empty constructor.
    CImgStats():min(0),max(0),mean(0),variance(0),xmin(-1),ymin(-1),zmin(-1),vmin(-1),lmin(-1),
        xmax(-1),ymax(-1),zmax(-1),vmax(-1),lmax(-1) {}
    //! Copy constructor.
    CImgStats(const CImgStats& stats):min(stats.min),max(stats.max),mean(stats.mean),variance(stats.variance),
                      xmin(stats.xmin),ymin(stats.ymin),zmin(stats.zmin),vmin(stats.vmin),lmin(stats.lmin),
                      xmax(stats.xmax),ymax(stats.ymax),zmax(stats.zmax),vmax(stats.vmax),lmax(stats.lmax) {};

    //! Constructor that compute statistics of an image \p img.
    /** 
        If \p compute_variance = true, the variance field of the CImgStats structure is computed, else it is set to 0.
    **/
    template<typename T> CImgStats(const CImg<T>& img,const bool compute_variance=true):mean(0),variance(0),lmin(-1),lmax(-1) {
      cimg_test(img,"CImgStats::CImgStats");
      T pmin=img[0], pmax=pmin, *ptrmin=img.data, *ptrmax=ptrmin;
      cimg_map(img,ptr,T) {
    const T& a=*ptr;
    mean+=(double)a;
    if (a<pmin) { pmin=a; ptrmin = ptr; }
    if (a>pmax) { pmax=a; ptrmax = ptr; }
      }
      mean/=img.size();
      min=(double)pmin;
      max=(double)pmax;
      unsigned long offmin = (unsigned long)(ptrmin-img.data), offmax = (unsigned long)(ptrmax-img.data);
      const unsigned long whz = img.width*img.height*img.depth, wh = img.width*img.height;      
      vmin = offmin/whz; offmin%=whz; zmin = offmin/wh; offmin%=wh; ymin = offmin/img.width; xmin = offmin%img.width;
      vmax = offmax/whz; offmax%=whz; zmax = offmax/wh; offmax%=wh; ymax = offmax/img.width; xmax = offmax%img.width;
      if (compute_variance) {
        cimg_map(img,ptr,T) { const double tmpf=(*ptr)-mean; variance+=tmpf*tmpf; }
        variance/=img.size();
      }
    }

    //! Constructor that compute statistics of an image list \p list.
    /**
       Statistics are computed for all pixels of all images of the list.
       If \p compute_variance = true, the variance field of the CImgStats structure is computed, else it is undefined.
    **/
    template<typename T> CImgStats(const CImgl<T>& list,const bool compute_variance=true):mean(0),variance(0) {
      cimgl_test(list,"CImgStats::CImgStats");
      T pmin=list[0][0], pmax=pmin, *ptrmin=list[0].data, *ptrmax=ptrmin;
      int psize=0;
      cimgl_map(list,l) {
        cimg_map(list[l],ptr,T) {
          const T& a=*ptr;
          mean+=(double)a;
          if (a<pmin) { pmin=a; ptrmin = ptr; lmin = l; }
          if (a>pmax) { pmax=a; ptrmax = ptr; lmax = l; }
        }
        psize+=list[l].size();
      }
      mean/=psize;
      min=(double)pmin;
      max=(double)pmax;
      const CImg<T> &imin = list[lmin], &imax = list[lmax];
      unsigned long offmin = (ptrmin-imin.data), offmax = (ptrmax-imax.data);
      const unsigned long whz1 = imin.width*imin.height*imin.depth, wh1 = imin.width*imin.height;
      vmin = offmin/whz1; offmin%=whz1; zmin = offmin/wh1; offmin%=wh1; ymin = offmin/imin.width; xmin = offmin%imin.width;
      const unsigned long whz2 = imax.width*imax.height*imax.depth, wh2 = imax.width*imax.height;
      vmax = offmax/whz2; offmax%=whz2; zmax = offmax/wh2; offmax%=wh2; ymax = offmax/imax.width; xmax = offmax%imax.width;
      if (compute_variance) {
        cimgl_map(list,l) cimg_map(list[l],ptr,T) { const double tmpf=(*ptr)-mean; variance+=tmpf*tmpf; }
        variance/=psize;
      }
    }

    //! Assignement operator.
    CImgStats& operator=(const CImgStats stats) {
      min = stats.min;
      max = stats.max;
      mean = stats.mean;
      variance = stats.variance;
      xmin = stats.xmin; ymin = stats.ymin; zmin = stats.zmin; vmin = stats.vmin; lmin = stats.lmin;
      xmax = stats.xmax; ymax = stats.ymax; zmax = stats.zmax; vmax = stats.vmax; lmax = stats.lmax;
      return *this;
    }
    //! Print the current statistics on the standart error output.
    const CImgStats& print(const char* title=NULL) const {
      if (lmin>=0 && lmax>=0)
    std::fprintf(stderr,"%-8s(this=%p) : { min=%g, mean=%g [var=%g], max=%g, "
             "pmin=[%d](%d,%d,%d,%d), pmax=[%d](%d,%d,%d,%d) }\n",
             title?title:"CImgStats",(void*)this,min,mean,variance,max,
             lmin,xmin,ymin,zmin,vmin,lmax,xmax,ymax,zmax,vmax);
      else
    std::fprintf(stderr,"%-8s(this=%p) : { min=%g, mean=%g [var=%g], max=%g, "
             "pmin=(%d,%d,%d,%d), pmax=(%d,%d,%d,%d) }\n",
             title?title:"CImgStats",(void*)this,min,mean,variance,max,
             xmin,ymin,zmin,vmin,xmax,ymax,zmax,vmax);
      return *this;
    }
    
#ifdef cimgstats_plugin
#include cimgstats_plugin
#endif

  };

  /*-------------------------------------------------------
  



    Definition of the CImgDisplay structure




  ------------------------------------------------------*/

  //! A CImgDisplay instance opens a display window where images can be drawn into it, and mouse and keyboard events can be handled.
  /**
     Creating a \c CImgDisplay instance opens a window that can be used to display a \c CImg<T> image
     of a \c CImgl<T> image list inside. When a display is created, associated window events
     (such as mouse motion, keyboard and window size changes) are handled and can be easily
     detected by testing specific \c CImgDisplay data fields.
     See \ref cimg_displays for a complete tutorial on using the \c CImgDisplay class.
  **/

  struct CImgDisplay {

    //------------------------
    //
    // CImgDisplay variables
    //
    //------------------------

    //! Width of the display.
    /**
       Prefer using CImgDisplay::dimx() to get the width of the display.
       
       \note Using CImgDisplay::dimx() instead of \p width is more safe when doing arithmetics
       involving the value of \p width, since it returns a \e signed int. Arithmetics with
       \e unsigned types needs a lot of attention.

       \note The variable \c width should be considered as read-only.
       Setting a new value for \p CImgDisplay::width is done through CImgDisplay::resize().
       Modifying directly \p width would probably result in a crash.

       \see CImgDisplay::height, CImgDisplay::resize()
    **/
    unsigned int width;

    //! Height of the display.
    /**
       Prefer using CImgDisplay::dimy() to get the height of the display.
 
       \note Using CImgDisplay::dimy() instead of \p height is more safe when doing arithmetics
       involving the value of \p height, since it returns a \e signed int. Artihmetics with
       \e unsigned types needs a lot of attention.

       \note The variable \c height should be considered as read-only.
       Setting a new value for \p CImgDisplay::height is done through CImgDisplay::resize().
       Modifying directly \p height would probably result in a crash.
       
       \see CImgDisplay::width, CImgDisplay::resize()
    **/
    unsigned int height;

    //! Width of the window containing the display.
    /**
       \note This is not the width of the display, but the width of the underlying system window.
       This variable is updated when an user resized the window associated to the display.
       When it occurs, \c width and \c window_width will be probably different.       
       \see CImgDisplay::window_height, CImgDisplay::resized, CImgDisplay::resize().
    **/
    volatile unsigned int window_width;

    //! Height of the window containing the display.
    /**
       \note This is not the height of the display, but the height of the underlying system window.
       This variable is updated when an user resized the window associated to the display.
       When it occurs, \c height and \c window_height will be probably different.
       \see CImgDisplay::window_width, CImgDisplay::resized, CImgDisplay::resize().
    **/
    volatile unsigned int window_height;

    //! X-coordinate of the display, relative to screen coordinates.
    volatile int window_x;

    //! Y-coordinate of the display, relative to screen coordinates.
    volatile int window_y;

    //! Type of pixel normalization done by the display.
    /**
       It represents the way the pixel values are normalized for display purposes.
       Its value can be set to :
       - \c 0 : No pixel value normalization are performed (fastest). Be sure your image data are bounded in [0,255].
       - \c 1 : Pixel value renormalization between [0,255] is done at each display request (default).
       - \c 2 : Pixel value renormalization between [0,255] is done at the first display request. Then the
       normalization parameters are kept and used for the next image display requests.
       \note \c normalization is preferably set by invoking constructors CImgDisplay::CImgDisplay().
       \see CImgDisplay::CImgDisplay(), CImgDisplay::display().
    **/
    unsigned int normalization;

    //! Type of events handled by the display.
    /**
       It represents what events are handled by the display. Its value can be set to :
       - \c 0 : No events are handled by the display.
       - \c 1 : Display closing and resizing are handled by the display.
       - \c 2 : Display closing, resizing, mouse motion and buttons press, as well as key press are handled by the display.
       - \c 3 : Display closing, resizing, mouse motion and buttons press/release, as well as key press/release
       are handled by the display.
       \note \c events if preferably set by invoking constructors CImgDisplay::CImgDisplay().
       \see CImgDisplay::CImgDisplay(), CImgDisplay::mouse_x, CImgDisplay::mouse_y, CImgDisplay::key,
       CImgDisplay::button, CImgDisplay::resized, CImgDisplay::closed.
    **/
    unsigned int events;

    //! Flag indicating fullscreen mode.
    /**
       If the display has been specified to be fullscreen at the construction, this variable is set to \c true.
       \note This is only useful for Windows-based OS. Fullscreen is not yet supported on X11-based systems
       and \c fullscreen will always be equal to \e false in this case.
    **/
    const bool fullscreen;

    //!  X-coordinate of the mouse pointer over the display.
    /**
       If CImgDisplay::events>=2, \p mouse_x represents the current x-coordinate of the mouse pointer.
       - If the mouse pointer is outside the display window, \p mouse_x is equal to \p -1.
       - If the mouse pointer is over the display window, \p mouse_x falls in the range [0,CImgDisplay::width-1],
       where \p 0 corresponds to the far left coordinate and \p CImgDisplay::width-1 to the far right coordinate.
       \note \p mouse_x is updated every 25 milliseconds, through an internal thread.
       \see CImgDisplay::mouse_y, CImgDisplay::button
    **/
    volatile int mouse_x;

    //! Y-coordinate of the mouse pointer over the display.
    /**
       If CImgDisplay::events>=2, \p mouse_y represents the current y-coordinate of the mouse pointer.
       - If the mouse pointer is outside the display window, \p mouse_y is equal to \p -1.
       - If the mouse pointer is over the display window, \p mouse_y falls in the range [0,CImgDisplay::height-1],
       where \p 0 corresponds to the far top coordinate and \p CImgDisplay::height-1 to the far bottom coordinate.
       \note \p mouse_y is updated every 25 milliseconds, through an internal thread.
       \see CImgDisplay::mouse_x, CImgDisplay::button
    **/
    volatile int mouse_y;

    //! Variable representing the state of the mouse buttons when the mouse pointer is over the display window.
    //! (should be considered as read only)
    /**
       If CImgDisplay::events>=2, \c button represents the current state of the mouse buttons.
       - If the mouse pointer is outside the display window, \c button is equal to \c 0.
       - If the mouse pointer is over the display window, \c button is a combination of the following bits :
       - bit 0 : State of the left mouse button.
       - bit 1 : State of the right mouse button.
       - bit 2 : State of the middle mouse button.
       - Other bits are unused.
       \note
       - \c button is updated every 25 milliseconds, through an internal thread.
       - If CImgDisplay::events==2, you should re-init \p button to \p 0 after catching the
       mouse button events, since it will NOT be done automatically (\p Mouse \p button \p Release event is
       not handled in this case).
       \see CImgDisplay::mouse_x, CImgDisplay::mouse_y
    **/
    volatile unsigned int button;

    //! Variable representing the key pressed when mouse pointer is over the display window.
    /**
       If CImgDisplay::events>=2, \c key represents a raw integer value corresponding 
       to the current pressed key.
       - If no keys are pressed, \c key is equal to \p 0.
       - If a key is pressed, \p key is a value representing the key. This raw value is \e OS-dependent.
       Testing the \p key value directly with a raw integer will mostly result in incompabilities
       between different plateforms. 
       To bypass this problem, \b OS-independent \b keycodes are defined in the \p cimg:: namespace.
       They are named as \p cimg::key*, where * stands for the key name :
       \p cimg::keyESC, \p cimg::keyF1, \p cimg::key0, \p cimg::keyA, \p cimg::keySPACE, \p cimg::keySHIFTLEFT, etc...
       \code
       CImgDisplay disp(320,200,"Display");        // Create a display window with full events handling
       ...
       if (disp.key==cimg::keyESC) std::exit(0);        // Exit when pressing the ESC key.
       ...
       \endcode

       \note 
       - \p key is updated every 25 milliseconds, through an internal thread.
       - If CImgDisplay::events==2, You should re-init the \c key variable to \c 0 after catching
       the \p Key \p Pressed event, since it will NOT be done automatically (Key Release event is handled
       only when \c CImgDisplay::events>=3).
 
       \see CImgDisplay::button, CImgDisplay::mouse_x, CImgDisplay::mouse_y
    **/
    volatile unsigned int key;

    //! Variable representing the visibility state of the display window (should be read only).
    /**
       \p closed can be either true or false :
       - \p false : The window is visible.
       - \p true  : The window is hidden.
     
       If CImgDisplay::events>=1, \p closed is set to \p true when the user try to close the display window.
       The way to set a value for \p closed is to use the functions :
       - CImgDisplay::show(), to set \p closed to \p false.
       - CImgDisplay::close(), to set \p closed to \p true.

       Closing a display window DO NOT destroy the instance object. It simply \e hides the display window
       and set the variable \p closed to true. You are then free to decide what to do
       when this event occurs. For instance, the following code will re-open the window indefinitely 
       when the user tries to close it :
       \code
       CImgDisplay disp(320,200,"Try to close me !");
       for (;; disp.wait()) if (disp.closed) disp.show();
       \endcode

       \note - \p closed is updated every 25 milliseconds, through an internal thread.

       \see CImgDisplay::show(), CImgDisplay::close().
    **/
    volatile bool closed;

    //! Event-variable
    volatile bool resized;
    volatile bool moved;

    // Not documented, internal use only.
    double min,max;

    //------------------------
    //
    // CImgDisplay functions
    //
    //------------------------

    //! Return the width of the display window, as a signed integer.
    /** \note When working with resizing window, \p dimx() does not necessarily return the width of the resized window,
        but the width of the internal data structure that can be used to display image.
        Resizing a display window can be done with the function CImgDisplay::resize().
      
        \see CImgDisplay::width, CImgDisplay::dimy(), CImgDisplay::resize()     
    **/
    const int dimx() const { return (int)width; }

    //! Return the height of the display window, as a signed integer.
    /** \note When working with resizing window, \p dimy() does not necessarily return the height of the resized window,
        but the height of the internal data structure that can be used to display image.
        Resizing a display window can be done with the function CImgDisplay::resize().
      
        \see CImgDisplay::height, CImgDisplay::dimx(), CImgDisplay::resize()     
    **/
    const int dimy() const { return (int)height; }

    const int window_dimx() const { return (int)window_width; }
    const int window_dimy() const { return (int)window_height; }    

    // operator=(). It is actually defined to avoid its use, and throw a CImgDisplay exception.
    CImgDisplay& operator=(const CImgDisplay&) {
      throw CImgDisplayException("CImgDisplay()::operator=() : Assignement of CImgDisplay is not allowed. Use pointers instead !");
      return *this;
    }
    
    //! Synchronized waiting function. Same as cimg::wait().
    /** \see cimg::wait()
     **/
      const CImgDisplay& wait(const unsigned int milliseconds) const { cimg::wait(milliseconds); return *this; }

    //! Display an image list CImgl<T> into a display window.
    /** First, all images of the list are appended into a single image used for visualization,
        then this image is displayed in the current display window.
        \param list     : The list of images to display.
        \param axe      : The axe used to append the image for visualization. Can be 'x' (default),'y','z' or 'v'.
        \param align : Defines the relative alignment of images when displaying images of different sizes.
        Can be '\p c' (centered, which is the default), '\p p' (top alignment) and '\p n' (bottom aligment).

        \see CImg::append()
    **/
    template<typename T> CImgDisplay& display(const CImgl<T>& list,const char axe='x',const char align='c') { 
      return display(list.get_append(axe,align)); 
    } 

    //! Resize a display window with the size of an image.
    /** \param img    : Input image. \p image.width and \p image.height give the new dimensions of the display window.
        \param redraw : If \p true (default), the current displayed image in the display window will
        be bloc-interpolated to fit the new dimensions. If \p false, a black image will be drawn in the resized window.
        \param force  : If \p true, the window size is effectively set to the specified dimensions (default).
        If \p false, only internal data buffer to display images is resized, not the window itself.
      
        \see CImgDisplay::resized, CImgDisplay::resizedimx(), CImgDisplay::resizedimy()
    **/
    template<typename T> CImgDisplay& resize(const CImg<T>& img,const bool redraw=false,const bool force=true) { 
      return resize(img.width,img.height,redraw,force); 
    }

    CImgDisplay& resize(const CImgDisplay& disp,const bool redraw=false,const bool force=true) {
      return resize(disp.width,disp.height,redraw,force);
    }

    CImgDisplay& resize(const bool redraw=false,const bool force=false) {
      resize(window_width,window_height,redraw,force);
      return *this;
    }

    // When no display available
    //---------------------------
#if cimg_display_type==0
    void nodisplay_available() {
      static bool first = true;
      if (first) {
        cimg::warn(true,"CImgDisplay() : Display has been required but is not available (cimg_display_type=0)");
        first = false;
      }    
    }  
    //! Create a display window with a specified size \p pwidth x \p height.
    /** \param dimw       : Width of the display window.
        \param dimh       : Height of the display window.
        \param title      : Title of the display window.
        \param normalization_type  : Normalization type of the display window (see CImgDisplay::normalize).
    \param events_type : Type of events handled by the display window.
    \param fullscreen_flag : Fullscreen mode.
    \param closed_flag : Initially visible mode.      
        A black image will be initially displayed in the display window.
    **/
    CImgDisplay(const unsigned int dimw,const unsigned int dimh,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):fullscreen(false) {
      nodisplay_available(); 
    }

    //! Create a display window from an image.
    /** \param img : Image that will be used to create the display window.
        \param title : Title of the display window
        \param normalization_type : Normalization type of the display window.
        \param events_type : Type of events handled by the display window.
    \param fullscreen_flag : Fullscreen mode.
    \param closed_flag : Initially visible mode.      
    **/
    template<typename T> 
    CImgDisplay(const CImg<T>& img,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):fullscreen(false) {
      nodisplay_available(); 
    }
    
    //! Create a display window from an image list.
    /** \param list : The list of images to display.
        \param title : Title of the display window
        \param normalization_type : Normalization type of the display window.
    \param events_type : Type of events handled by the display window.
    \param fullscreen_flag : Fullscreen mode.
    \param closed_flag : Initially visible mode.      
    **/
    template<typename T> 
    CImgDisplay(const CImgl<T>& list,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):fullscreen(false) {
      nodisplay_available(); 
    }
  
    //! Create a display window by copying another one.
    /** \param win   : Display window to copy.
        \param title : Title of the new display window.
    **/
    CImgDisplay(const CImgDisplay& win, char *title=NULL):fullscreen(false) { nodisplay_available(); }

    //! Resize a display window with new dimensions \p width and \p height.
    CImgDisplay& resize(const int width, const int height,const bool redraw=false,const bool force=true) {
      return *this; 
    }
    //! Move a display window at a specific location \p posx, \p posy.
    CImgDisplay& move(const int posx,const int posy) { return *this; }

    //! Destructor. Close and destroy a display.
    ~CImgDisplay() {}
    //! Fill the pixel data of the window buffer according to the image \p pimg.
    template<typename T> void render(const CImg<T>& img,const unsigned int ymin=0,const unsigned int ymax=~0) {}
    //! Display an image in a window.
    template<typename T> CImgDisplay& display(const CImg<T>& img,const unsigned int ymin=0,const unsigned int ymax=-1) { return *this; }
    //! Wait for a window event
    CImgDisplay& wait()  { return *this; }
    //! Show a closed display
    CImgDisplay& show()  { return *this; }
    //! Close a visible display
    CImgDisplay& close() { return *this; }
  
    //! Return the width of the screen resolution.
    static const int screen_dimx() { return 0; }

    //! Return the height of the screen resolution.
    static const int screen_dimy() { return 0; }

    // X11-based display
    //------------------
#elif cimg_display_type==1
    void *data;
    Window window;
    XImage *image;
 
    CImgDisplay(const unsigned int dimw,const unsigned int dimh,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):
      width(dimw),height(dimh),normalization(normalization_type&3),events(events_type&3),
      fullscreen(fullscreen_flag),closed(closed_flag),min(0),max(0) {
      if (!dimw || !dimh) throw CImgArgumentException("CImgDisplay::CImgDisplay() : null size specified");
      new_lowlevel(title);
      std::memset(data,0,(cimg::X11attr().nb_bits>16?sizeof(unsigned int):sizeof(unsigned short))*width*height);
      pthread_mutex_lock(cimg::X11attr().mutex);
      XPutImage(cimg::X11attr().display,window,*cimg::X11attr().gc,image,0,0,0,0,width,height);
      XFlush(cimg::X11attr().display);
      pthread_mutex_unlock(cimg::X11attr().mutex);
    }

    template<typename T> 
    CImgDisplay(const CImg<T>& img,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):
      normalization(normalization_type&3),events(events_type&3),
      fullscreen(fullscreen_flag),closed(closed_flag),min(0),max(0) {
      cimg_test(img,"CImgDisplay::CImgDisplay");
      CImg<T> tmp;
      const CImg<T>& nimg = (img.depth==1)?img:(tmp=img.get_3dplanes(img.width/2,img.height/2,img.depth/2));
      width  = nimg.width;
      height = nimg.height;
      if (normalization==2) { CImgStats st(img,false); min=st.min; max=st.max; }
      new_lowlevel(title);
      display(nimg);
    }

    template<typename T> 
    CImgDisplay(const CImgl<T>& list,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):
      normalization(normalization_type&3),events(events_type&3),fullscreen(fullscreen_flag),
      closed(closed_flag),min(0),max(0) {
      cimgl_test(list,"CImgDisplay::CImgDisplay");
      CImg<T> tmp;
      const CImg<T> 
    img0 = list.get_append('x'), 
        &img = (img0.depth==1)?img0:(tmp=img0.get_3dplanes(img0.width/2,img0.height/2,img0.depth/2));
      width  = img.width; 
      height = img.height;
      if (normalization==2) { CImgStats st(img,false); min=st.min; max=st.max; }
      new_lowlevel(title);
      display(img);
    }

    CImgDisplay(const CImgDisplay& win, char *title="[Copy]"):
      width(win.width),height(win.height),normalization(win.normalization&3),events(win.events&3),
      fullscreen(win.fullscreen),closed(win.closed),min(win.min),max(win.max) {
      new_lowlevel(title);
      std::memcpy(data,win.data,(cimg::X11attr().nb_bits>16?sizeof(unsigned int):sizeof(short))*width*height);
      pthread_mutex_lock(cimg::X11attr().mutex);
      XPutImage(cimg::X11attr().display,window,*cimg::X11attr().gc,image,0,0,0,0,width,height);
      XFlush(cimg::X11attr().display);
      pthread_mutex_unlock(cimg::X11attr().mutex);
    }

    CImgDisplay& resize(const int nwidth, const int nheight,const bool redraw=false,const bool force=true) {
      const unsigned int
        dimx=(nwidth>0)?nwidth:(-width*nwidth/100),
        dimy=(nheight>0)?nheight:(-height*nheight/100);
      if (!dimx || !dimy) return *this;
      pthread_mutex_lock(cimg::X11attr().mutex);
      if (dimx!=width || dimy!=height) {
    if (cimg::X11attr().nb_bits>16) {
      unsigned int *ndata = new unsigned int[dimx*dimy];
      if (redraw) for (unsigned int y=0; y<dimy; y++) for (unsigned int x=0; x<dimx; x++)
        ndata[x+y*dimx] = ((unsigned int*)data)[x*width/dimx + width*(y*height/dimy)];
      else std::memset(ndata,0,sizeof(unsigned int)*dimx*dimy);
      data = (void*)ndata;
    } else {
      unsigned short *ndata = new unsigned short[dimx*dimy];
      if (redraw) for (unsigned int y=0; y<dimy; y++) for (unsigned int x=0; x<dimx; x++)
        ndata[x+y*dimx] = ((unsigned short*)data)[x*width/dimx + width*(y*height/dimy)];
      else std::memset(ndata,0,sizeof(unsigned short)*dimx*dimy);
      data = (void*)ndata;
    }
        XDestroyImage(image);
        image = XCreateImage(cimg::X11attr().display,DefaultVisual(cimg::X11attr().display,DefaultScreen(cimg::X11attr().display)),
                 cimg::X11attr().nb_bits,ZPixmap,0,(char*)data,dimx,dimy,8,0);
      }
      width  = dimx;
      height = dimy;
      if (force && (window_width!=width || window_height!=height)) {
        XResizeWindow(cimg::X11attr().display,window,width,height);
    window_width = width;
    window_height = height;
      }
      XPutImage(cimg::X11attr().display,window,*cimg::X11attr().gc,image,0,0,0,0,width,height);
      XFlush(cimg::X11attr().display);
      resized = false;
      pthread_mutex_unlock(cimg::X11attr().mutex);
      return *this;
    }
  
    CImgDisplay& move(const int posx,const int posy) {
      pthread_mutex_lock(cimg::X11attr().mutex);
      window_x = posx;
      window_y = posy;
      XMoveWindow(cimg::X11attr().display,window,posx,posy);
      XFlush(cimg::X11attr().display);
      moved = false;
      pthread_mutex_unlock(cimg::X11attr().mutex);
      return *this;
    }
    
    ~CImgDisplay() {
      unsigned int i;
      pthread_mutex_lock(cimg::X11attr().mutex);
      for (i=0; i<cimg::X11attr().nb_wins && cimg::X11attr().wins[i]!=this; i++) i++;
      for (; i<cimg::X11attr().nb_wins-1; i++) cimg::X11attr().wins[i]=cimg::X11attr().wins[i+1];
      cimg::X11attr().nb_wins--;
      XDestroyWindow(cimg::X11attr().display,window);
      XDestroyImage(image);
      if (!cimg::X11attr().nb_wins) {
        pthread_cancel(*cimg::X11attr().event_thread);
        pthread_join(*cimg::X11attr().event_thread,NULL);
        XCloseDisplay(cimg::X11attr().display);
        cimg::X11attr().display=NULL;
        pthread_mutex_unlock(cimg::X11attr().mutex);
        pthread_mutex_destroy(cimg::X11attr().mutex);
        delete cimg::X11attr().event_thread;
        delete cimg::X11attr().mutex;
        delete cimg::X11attr().gc;
      } else pthread_mutex_unlock(cimg::X11attr().mutex);
    }
  
    void new_lowlevel(const char *title=NULL) {
      cimg::warn(fullscreen,"CImgDisplay::new_lowlevel() : Fullscreen mode requested, "
         "but not supported on X11 Displays");
      if (!cimg::X11attr().display) { // Open X11 Display if not already done.
        cimg::X11attr().nb_wins = 0;
        cimg::X11attr().thread_finished = false;
        cimg::X11attr().mutex = new pthread_mutex_t;
        pthread_mutex_init(cimg::X11attr().mutex,NULL);
        pthread_mutex_lock(cimg::X11attr().mutex);
        cimg::X11attr().display = XOpenDisplay((std::getenv("DISPLAY") ? std::getenv("DISPLAY") : ":0.0"));
        if (!cimg::X11attr().display) throw CImgDisplayException("CImgDisplay::new_lowlevel() : Can't open X11 display");
        cimg::X11attr().nb_bits = DefaultDepth(cimg::X11attr().display, DefaultScreen(cimg::X11attr().display));
        if (cimg::X11attr().nb_bits!=16 && cimg::X11attr().nb_bits!=24)
          throw CImgDisplayException("CImgDisplay::new_lowlevel() : %u bits mode is not supported "
                     "(only 16 and 24 bits are supported)",cimg::X11attr().nb_bits);
        cimg::X11attr().gc = new GC;
        *cimg::X11attr().gc = DefaultGC(cimg::X11attr().display,DefaultScreen(cimg::X11attr().display));
        Visual *visual = DefaultVisual(cimg::X11attr().display,0);
    XVisualInfo vtemplate;
    vtemplate.visualid = XVisualIDFromVisual(visual);
    int nb_visuals;
    XVisualInfo *vinfo = XGetVisualInfo(cimg::X11attr().display,VisualIDMask,&vtemplate,&nb_visuals);
    if (vinfo && vinfo->red_mask<vinfo->blue_mask) cimg::X11attr().endian = true;
        cimg::X11attr().event_thread = new pthread_t;
        pthread_create(cimg::X11attr().event_thread,NULL,thread_lowlevel,NULL);
      } else pthread_mutex_lock(cimg::X11attr().mutex);
      
      // Create display window and image data.
      window = XCreateSimpleWindow(cimg::X11attr().display,RootWindow(cimg::X11attr().display,DefaultScreen(cimg::X11attr().display)),
                     0,0,width,height,2,0,0x0L);
      if (cimg::X11attr().nb_bits>16) data = new unsigned int[width*height]; else data = new unsigned short[width*height];
      image = XCreateImage(cimg::X11attr().display,DefaultVisual(cimg::X11attr().display,DefaultScreen(cimg::X11attr().display)),
                 cimg::X11attr().nb_bits,ZPixmap,0,(char*)data,width,height,8,0);      
      XStoreName(cimg::X11attr().display,window,title?title:"");
      if (!closed) {
        XEvent event;
        XSelectInput(cimg::X11attr().display,window,StructureNotifyMask);
        XMapWindow(cimg::X11attr().display,window);
        do XWindowEvent(cimg::X11attr().display,window,StructureNotifyMask,&event); while (event.type!=MapNotify);
    XWindowAttributes attr;
    XGetWindowAttributes(cimg::X11attr().display,window,&attr);
    window_x = attr.x;
    window_y = attr.y;
      } else window_x = window_y = 0;
      if (events) { 
        Atom atom = XInternAtom(cimg::X11attr().display, "WM_DELETE_WINDOW", False); 
        XSetWMProtocols(cimg::X11attr().display, window, &atom, 1); 
      }
      window_width = width;
      window_height = height;
      mouse_x = mouse_y = -1;
      button = key = 0;
      resized = moved = false;
      cimg::X11attr().wins[cimg::X11attr().nb_wins++]=this;
      pthread_mutex_unlock(cimg::X11attr().mutex);
    }
  
    void proc_lowlevel(XEvent *pevent) {
      const unsigned int buttoncode[3] = { 1,4,2 };
      XEvent event=*pevent;
      switch (event.type) {
      case ClientMessage:
        XUnmapWindow(cimg::X11attr().display,window);
        mouse_x=mouse_y=-1; 
    button=key=0;
    closed=true; 
        break;
     case ConfigureNotify: {
        while (::XCheckWindowEvent(cimg::X11attr().display,window,StructureNotifyMask,&event));
        const unsigned int
      nw = event.xconfigure.width,
      nh = event.xconfigure.height;
    const int
      nx = event.xconfigure.x,
      ny = event.xconfigure.y;
        if (nw && nh && (nw!=window_width || nh!=window_height)) { 
          window_width = nw; 
          window_height = nh; 
      mouse_x = mouse_y = -1;
          XResizeWindow(cimg::X11attr().display,window,window_width,window_height);
          resized = true;
        }
    if (nx!=window_x || ny!=window_y) {
      window_x = nx;
      window_y = ny;
      moved = true;
    }
      } break;
      case Expose:
        while (XCheckWindowEvent(cimg::X11attr().display,window,ExposureMask,&event));
        XPutImage(cimg::X11attr().display,window,*cimg::X11attr().gc,image,0,0,0,0,width,height);
        break;
      case ButtonPress:
        while (XCheckWindowEvent(cimg::X11attr().display,window,ButtonPressMask,&event));
        button |= buttoncode[event.xbutton.button-1];
        break;
      case ButtonRelease:
        while (XCheckWindowEvent(cimg::X11attr().display,window,ButtonReleaseMask,&event));
        button &= ~buttoncode[event.xbutton.button-1];
        break;
      case KeyPress: {
        while (XCheckWindowEvent(cimg::X11attr().display,window,KeyPressMask,&event));
    char tmp;
    KeySym ksym;
    XLookupString(&event.xkey,&tmp,1,&ksym,NULL);
    key = (unsigned int)ksym;
      }
        break;
      case KeyRelease:
        while (XCheckWindowEvent(cimg::X11attr().display,window,KeyReleaseMask,&event));
        key = 0;
        break;
      case LeaveNotify:
        while (XCheckWindowEvent(cimg::X11attr().display,window,LeaveWindowMask,&event));
        mouse_x = mouse_y =-1; 
        break;
      case MotionNotify:
        while (XCheckWindowEvent(cimg::X11attr().display,window,PointerMotionMask,&event));
        mouse_x = event.xmotion.x; 
        mouse_y = event.xmotion.y;
        if (mouse_x<0 || mouse_y<0 || mouse_x>=dimx() || mouse_y>=dimy()) mouse_x=mouse_y=-1; 
        break;
      }
    }
  
    static void* thread_lowlevel(void *arg) {
      XEvent event;
      pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
      for (;;) {
        pthread_mutex_lock(cimg::X11attr().mutex);
        for (unsigned int i=0; i<cimg::X11attr().nb_wins; i++) {
          const unsigned int xevent_type = (cimg::X11attr().wins[i]->events)&3;
          const unsigned int emask =
            ((xevent_type>=1)?ExposureMask|StructureNotifyMask:0)|
            ((xevent_type>=2)?ButtonPressMask|KeyPressMask|PointerMotionMask|LeaveWindowMask:0)|
            ((xevent_type>=3)?ButtonReleaseMask|KeyReleaseMask:0);
          XSelectInput(cimg::X11attr().display,cimg::X11attr().wins[i]->window,emask);
        }
        bool event_flag = XCheckTypedEvent(cimg::X11attr().display, ClientMessage, &event);
        if (!event_flag) event_flag = XCheckMaskEvent(cimg::X11attr().display,
                                                      ExposureMask|StructureNotifyMask|ButtonPressMask|
                                                      KeyPressMask|PointerMotionMask|LeaveWindowMask|ButtonReleaseMask|
                                                      KeyReleaseMask,&event);
        if (event_flag) {
          for (unsigned int i=0; i<cimg::X11attr().nb_wins; i++)
            if (!cimg::X11attr().wins[i]->closed && event.xany.window==cimg::X11attr().wins[i]->window)
          cimg::X11attr().wins[i]->proc_lowlevel(&event);
          cimg::X11attr().thread_finished = true;
        }
        pthread_mutex_unlock(cimg::X11attr().mutex);
        cimg::wait(25);
      }
      return NULL;
    }

    template<typename T> XImage* render(const CImg<T>& img,const unsigned int ymin=0,const unsigned int ymax=~0) {
      cimg_test(img,"CImgDisplay::render");
      if (img.depth!=1) return render(img.get_3dplanes(img.width/2,img.height/2,img.depth/2),0,~0);
      if (img.width!=width || img.height!=height) return render(img.get_resize(width,height,1,-100,1),0,~0);
      const bool by = (ymin<=ymax);
      const unsigned int 
    nymin = (by?ymin:ymax),
    nymax = (by?(ymax<height?ymax:height-1):(ymin<height?ymin:height-1)),
    xymax = (nymax+1)*width;
      const T 
        *data1 = img.ptr(0,nymin,0,0),
        *data2 = (img.dim>=2)?img.ptr(0,nymin,0,1):data1,
        *data3 = (img.dim>=3)?img.ptr(0,nymin,0,2):data1;
      if (cimg::X11attr().endian) cimg::swap(data1,data3);
      pthread_mutex_lock(cimg::X11attr().mutex);
      if (!normalization) {
        switch (cimg::X11attr().nb_bits) {
        case 16: {
      for (unsigned int xy=nymin*width; xy<xymax; xy++) {
        const unsigned char R = (unsigned char)*(data1++), G = (unsigned char)*(data2++), B = (unsigned char)*(data3++);
        XPutPixel(image,xy,0,((R>>3)<<11) | ((G>>2)<<5) | (B>>3));
      }
    } break;
        case 24: {
      for (unsigned int xy=nymin*width; xy<xymax; xy++) {        
        const unsigned char R = (unsigned char)*(data1++), G = (unsigned char)*(data2++), B = (unsigned char)*(data3++);
        XPutPixel(image,xy,0,(R<<16) | (G<<8) | B);
      }
    } break;
        };
      } else {
        if (normalization==1) { CImgStats st(img,false); min=st.min; max=st.max; }
        const T nmin = (T)min, delta = (T)max-nmin, mm=delta?delta:(T)1;
        switch (cimg::X11attr().nb_bits) {
        case 16: {
      for (unsigned int xy=nymin*width; xy<xymax; xy++) {
        const unsigned char
          R = (unsigned char)(255*(*(data1++)-nmin)/mm),
          G = (unsigned char)(255*(*(data2++)-nmin)/mm),
          B = (unsigned char)(255*(*(data3++)-nmin)/mm);
        XPutPixel(image,xy,0,((R>>3)<<11) | ((G>>2)<<5) | (B>>3));
      }
    } break;
        case 24: {
      for (unsigned int xy=nymin*width; xy<xymax; xy++) {
        const unsigned char
          R = (unsigned char)(255*(*(data1++)-nmin)/mm),
          G = (unsigned char)(255*(*(data2++)-nmin)/mm),
          B = (unsigned char)(255*(*(data3++)-nmin)/mm);
        XPutPixel(image,xy,0,(R<<16) | (G<<8) | B);
      } 
    } break;
        } 
      }
      pthread_mutex_unlock(cimg::X11attr().mutex);
      return image;
    }
    
    template<typename T> CImgDisplay& display(const CImg<T>& pimg,const unsigned int pymin=0,const unsigned int pymax=~0) {
      const unsigned int
        ymin = pymin<pymax?pymin:pymax,
        ymax = pymin<pymax?(pymax>=height?height-1:pymax):(pymin>=height?height-1:pymin);
      render(pimg,ymin,ymax);
      if (!closed) {      
        pthread_mutex_lock(cimg::X11attr().mutex);
        XPutImage(cimg::X11attr().display,window,*cimg::X11attr().gc,image,0,ymin,0,ymin,width,ymax-ymin+1);
        XFlush(cimg::X11attr().display);
        pthread_mutex_unlock(cimg::X11attr().mutex);
      }
      return *this;
    }
  
    CImgDisplay& wait() {
      if (!closed && events) {
        XEvent event;
        do {
          pthread_mutex_lock(cimg::X11attr().mutex);
          const unsigned int 
            emask = ExposureMask|StructureNotifyMask|
            ((events>=2)?ButtonPressMask|KeyPressMask|PointerMotionMask|LeaveWindowMask:0)|
            ((events>=3)?ButtonReleaseMask|KeyReleaseMask:0);
          XSelectInput(cimg::X11attr().display,window,emask);
          XPeekEvent(cimg::X11attr().display,&event);
          cimg::X11attr().thread_finished = false;
          pthread_mutex_unlock(cimg::X11attr().mutex);
        } while (event.xany.window!=window);
        while (!cimg::X11attr().thread_finished) cimg::wait(25);
      }
      return *this;
    }

    CImgDisplay& show() {
      if (closed) {
        pthread_mutex_lock(cimg::X11attr().mutex);
        XEvent event;
        XSelectInput(cimg::X11attr().display,window,StructureNotifyMask);
        XMapWindow(cimg::X11attr().display,window);
        do XWindowEvent(cimg::X11attr().display,window,StructureNotifyMask,&event);
        while (event.type!=MapNotify);
    XWindowAttributes attr;
    XGetWindowAttributes(cimg::X11attr().display,window,&attr);
    window_x = attr.x;
    window_y = attr.y;
        XPutImage(cimg::X11attr().display,window,*cimg::X11attr().gc,image,0,0,0,0,width,height);
        XFlush(cimg::X11attr().display);
        closed = false;
        pthread_mutex_unlock(cimg::X11attr().mutex);
      }
      return *this;
    }
    CImgDisplay& close() {
      if (!closed) {
        pthread_mutex_lock(cimg::X11attr().mutex);
        XUnmapWindow(cimg::X11attr().display,window);
        XFlush(cimg::X11attr().display);
        closed = true;
    window_x = window_y = 0;    
        pthread_mutex_unlock(cimg::X11attr().mutex);
      }
      return *this;
    }

    static const int screen_dimx() { 
      int res = 0;
      if (!cimg::X11attr().display) {
    Display *disp = XOpenDisplay((std::getenv("DISPLAY") ? std::getenv("DISPLAY") : ":0.0"));
    if (!disp) throw CImgDisplayException("CImgDisplay::screen_dimx() : Can't open X11 display");
    res = DisplayWidth(disp,DefaultScreen(disp));
    XCloseDisplay(disp);
      } else res = DisplayWidth(cimg::X11attr().display,DefaultScreen(cimg::X11attr().display));
      return res;
    }

    static const int screen_dimy() { 
      int res = 0;
      if (!cimg::X11attr().display) {
    Display *disp = XOpenDisplay((std::getenv("DISPLAY") ? std::getenv("DISPLAY") : ":0.0"));
    if (!disp) throw CImgDisplayException("CImgDisplay::screen_dimy() : Can't open X11 display");
    res = DisplayHeight(disp,DefaultScreen(disp));
    XCloseDisplay(disp);
      } else res = DisplayHeight(cimg::X11attr().display,DefaultScreen(cimg::X11attr().display));
      return res;
    }

    // Windows-based display
    //-----------------------
#elif cimg_display_type==2
    CLIENTCREATESTRUCT ccs;
    BITMAPINFO bmi;
    unsigned int *data;
    DEVMODE curr_mode;
    HWND window;
    HDC hdc;
    HANDLE thread;
    HANDLE wait_disp;
    HANDLE created;
    HANDLE mutex;

    CImgDisplay(const unsigned int dimw,const unsigned int dimh,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):
      width(dimw),height(dimh),normalization(normalization_type&3),events(events_type&3),
      fullscreen(fullscreen_flag),closed(closed_flag),min(0),max(0) {
      if (!dimw || !dimh) throw CImgArgumentException("CImgDisplay::CImgDisplay() : null size specified. ");
      new_lowlevel(title);
      std::memset(data,0,sizeof(unsigned int)*width*height);
      SetDIBitsToDevice(hdc,0,0,width,height,0,0,0,height,data,&bmi,DIB_RGB_COLORS);
    }

    template<typename T> 
    CImgDisplay(const CImg<T>& img,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):
      normalization(normalization_type&3),events(events_type&3),
      fullscreen(fullscreen_flag),closed(closed_flag),min(0),max(0) {
      cimg_test(img,"CImgDisplay::CImgDisplay");
      CImg<T> tmp;
      const CImg<T>& nimg = (img.depth==1)?img:(tmp=img.get_3dplanes(img.width/2,img.height/2,img.depth/2));
      width  = nimg.width;
      height = nimg.height;
      if (normalization==2) { CImgStats st(img,false); min=st.min; max=st.max; }
      new_lowlevel(title);
      display(nimg);
    }

    template<typename T>
    CImgDisplay(const CImgl<T>& list,const char *title=NULL,
                const unsigned int normalization_type=1,const unsigned int events_type=3,
                const bool fullscreen_flag=false,const bool closed_flag=false):
      normalization(normalization_type&3),events(events_type&3),fullscreen(fullscreen_flag),
      closed(closed_flag),min(0),max(0) {
      cimgl_test(list,"CImgDisplay::CImgDisplay");
      CImg<T> tmp;
      const CImg<T>
    img0 = list.get_append('x'),
        &img = (img0.depth==1)?img0:(tmp=img0.get_3dplanes(img0.width/2,img0.height/2,img0.depth/2));
      width  = img.width;
      height = img.height;
      if (normalization==2) { CImgStats st(img,false); min=st.min; max=st.max; }
      new_lowlevel(title);
      display(img);
    }

    CImgDisplay(const CImgDisplay& win, char *title="[Copy]"):
      width(win.width),height(win.height),normalization(win.normalization&3),events(win.events&3),
      fullscreen(win.fullscreen),closed(win.closed),min(win.min),max(win.max) {
      new_lowlevel(title);
      std::memcpy(data,win.data,sizeof(unsigned int)*width*height);
      SetDIBitsToDevice(hdc,0,0,width,height,0,0,0,height,data,&bmi,DIB_RGB_COLORS);
    }

    CImgDisplay& resize(const int nwidth, const int nheight,const bool redraw=false,const bool force=true) {
      const unsigned int
        dimx=(nwidth>0)?nwidth:(-nwidth*width/100),
        dimy=(nheight>0)?nheight:(-nheight*height/100);
      if (!dimx || !dimy) return *this;
      if (dimx!=width || dimy!=height) {
        unsigned int *ndata = new unsigned int[dimx*dimy];
        if (redraw) 
          for (unsigned int y=0; y<dimy; y++) for (unsigned int x=0; x<dimx; x++)
        ndata[x+y*dimx] = data[x*width/dimx + width*(y*height/dimy)];
        else std::memset(ndata,0x80,sizeof(unsigned int)*dimx*dimy);
        delete[] data;
        data = ndata;
        bmi.bmiHeader.biWidth=dimx;
        bmi.bmiHeader.biHeight=-(int)dimy;
      }
      width  = dimx;
      height = dimy;
      if (force && (window_width!=width || window_height!=height)) {
        RECT rect; rect.left=rect.top=0; rect.right=width-1; rect.bottom=height-1;
        AdjustWindowRect(&rect,WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,false);
    const int cwidth = rect.right-rect.left+1, cheight = rect.bottom-rect.top+1;
        SetWindowPos(window,0,0,0,cwidth,cheight,SWP_NOMOVE | SWP_NOZORDER | SWP_NOCOPYBITS);
        window_width  = width;
        window_height = height;
      }
      SetDIBitsToDevice(hdc,0,0,width,height,0,0,0,height,data,&bmi,DIB_RGB_COLORS);
      resized = false;
      return *this;
    }
    
    CImgDisplay& move(const int posx,const int posy) {
      RECT rect; rect.left=rect.top=0; rect.right=width-1; rect.bottom=height-1;
      AdjustWindowRect(&rect,WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,false);
      const int border1 = (rect.right-rect.left+1-width)/2, border2 = rect.bottom-rect.top+1-height-border1;
      window_x = posx;
      window_y = posy;
      SetWindowPos(window,0,posx-border1,posy-border2,0,0,SWP_NOSIZE | SWP_NOZORDER);
      moved = false;
      return *this;
    }

    ~CImgDisplay() {
      DestroyWindow(window);
      if (events) TerminateThread(thread,0);
      delete[] data;
      if (curr_mode.dmSize) ChangeDisplaySettings(&curr_mode,0);
    }
  
    void new_lowlevel(const char *title=NULL) {
      unsigned long ThreadID;
      DEVMODE mode;
      unsigned int imode=0,ibest=0,bestbpp=0;
      void *arg = (void*)(new void*[2]);
      ((void**)arg)[0]=(void*)this;
      ((void**)arg)[1]=(void*)title;
      if (fullscreen) {
        for (mode.dmSize = sizeof(DEVMODE), mode.dmDriverExtra = 0; EnumDisplaySettings(NULL,imode,&mode); imode++)
          if (mode.dmPelsWidth==width && mode.dmPelsHeight==height && mode.dmBitsPerPel>bestbpp) {
            bestbpp = mode.dmBitsPerPel;
            ibest=imode; 
          }
        cimg::warn(!bestbpp,"CImgDisplay::new_lowlevel() : Could not initialize fullscreen mode %ux%u\n",width,height);
        if (bestbpp) {
          curr_mode.dmSize = sizeof(DEVMODE); curr_mode.dmDriverExtra = 0;
          EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&curr_mode);
          EnumDisplaySettings(NULL,ibest,&mode);
          ChangeDisplaySettings(&mode,0);
        }
        else curr_mode.dmSize = 0;
      }
      else curr_mode.dmSize = 0;
      if (events) {
        mutex     = CreateMutex(NULL,FALSE,NULL);
        created   = CreateEvent(NULL,FALSE,FALSE,NULL);
        wait_disp = CreateEvent(NULL,FALSE,FALSE,NULL);
        thread    = CreateThread(NULL,0,thread_lowlevel,arg,0,&ThreadID);
        WaitForSingleObject(created,INFINITE);
      } else thread_lowlevel(arg);
    }
  
    static LRESULT APIENTRY proc_lowlevel(HWND window,UINT msg,WPARAM wParam,LPARAM lParam) {
      CImgDisplay* disp = (CImgDisplay*)GetWindowLong(window,GWL_USERDATA);
      MSG st_msg;

      switch(msg) {
      case WM_CLOSE:
    disp->mouse_x=disp->mouse_y=-1;
    disp->key=disp->button=disp->window_x=disp->window_y=0;
        disp->closed=true;
        ReleaseMutex(disp->mutex);
        ShowWindow(disp->window,SW_HIDE);
        return 0;
      case WM_SIZE: {
        while (PeekMessage(&st_msg,window,WM_SIZE,WM_SIZE,PM_REMOVE));
        WaitForSingleObject(disp->mutex,INFINITE);
        const unsigned int nw = LOWORD(lParam),nh = HIWORD(lParam);
        if (nw && nh && (nw!=disp->width || nh!=disp->height)) { 
          disp->window_width  = nw; 
          disp->window_height = nh;
      disp->mouse_x = disp->mouse_y = -1;
          disp->resized = true;
        }
        ReleaseMutex(disp->mutex);
      } break;
      case WM_MOVE: {
        while (PeekMessage(&st_msg,window,WM_SIZE,WM_SIZE,PM_REMOVE));
    WaitForSingleObject(disp->mutex,INFINITE);
    const int nx = (int)(short)(LOWORD(lParam)), ny = (int)(short)(HIWORD(lParam));
    if (nx!=disp->window_x || ny!=disp->window_y) {
      disp->window_x = nx;
      disp->window_y = ny;
      disp->moved = true;
    }
        ReleaseMutex(disp->mutex);
      } break;
      case WM_PAINT:
        WaitForSingleObject(disp->mutex,INFINITE);
        SetDIBitsToDevice(disp->hdc,0,0,disp->width,disp->height,0,0,0,disp->height,disp->data,&(disp->bmi),DIB_RGB_COLORS);
        ReleaseMutex(disp->mutex);
        break;
      }
      if (disp->events>=2) switch(msg) {
      case WM_KEYDOWN:
        while (PeekMessage(&st_msg,window,WM_KEYDOWN,WM_KEYDOWN,PM_REMOVE)); 
        disp->key=(int)wParam;
        break;
      case WM_MOUSEMOVE: {
        while (PeekMessage(&st_msg,window,WM_MOUSEMOVE,WM_MOUSEMOVE,PM_REMOVE));
        disp->mouse_x = LOWORD(lParam);
        disp->mouse_y = HIWORD(lParam);
        if (disp->mouse_x<0 || disp->mouse_y<0 ||    disp->mouse_x>=disp->dimx() || disp->mouse_y>=disp->dimy())
      disp->mouse_x=disp->mouse_y=-1;
      }
        break;
      case WM_LBUTTONDOWN: 
        while (PeekMessage(&st_msg,window,WM_LBUTTONDOWN,WM_LBUTTONDOWN,PM_REMOVE));
        disp->button |= 1; 
        break;
      case WM_RBUTTONDOWN: 
        while (PeekMessage(&st_msg,window,WM_RBUTTONDOWN,WM_RBUTTONDOWN,PM_REMOVE));
        disp->button |= 2; 
        break;
      case WM_MBUTTONDOWN: 
        while (PeekMessage(&st_msg,window,WM_MBUTTONDOWN,WM_MBUTTONDOWN,PM_REMOVE));
        disp->button |= 4; 
        break;
      }
      if (disp->events>=3) switch(msg) {
      case WM_KEYUP:
        while (PeekMessage(&st_msg,window,WM_KEYUP,WM_KEYUP,PM_REMOVE));
        disp->key=0;
        break;
      case WM_LBUTTONUP:
        while (PeekMessage(&st_msg,window,WM_LBUTTONUP,WM_LBUTTONUP,PM_REMOVE));
        disp->button &= ~1; 
        break;
      case WM_RBUTTONUP:
        while (PeekMessage(&st_msg,window,WM_RBUTTONUP,WM_RBUTTONUP,PM_REMOVE)); 
        disp->button &= ~2;
        break;
      case WM_MBUTTONUP:
        while (PeekMessage(&st_msg,window,WM_MBUTTONUP,WM_MBUTTONUP,PM_REMOVE)); 
        disp->button &= ~4;
        break;
      }
      return DefWindowProc(window,msg,wParam,lParam);
    }
  
    static DWORD WINAPI thread_lowlevel(void* arg) {
      CImgDisplay *disp  = (CImgDisplay*)(((void**)arg)[0]);
      const char *title = (const char*)(((void**)arg)[1]);
      MSG msg;
      delete[] (void**)arg;
      disp->bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      disp->bmi.bmiHeader.biWidth=disp->width;
      disp->bmi.bmiHeader.biHeight=-(int)disp->height;
      disp->bmi.bmiHeader.biPlanes=1;
      disp->bmi.bmiHeader.biBitCount=32;
      disp->bmi.bmiHeader.biCompression=BI_RGB;
      disp->bmi.bmiHeader.biSizeImage=0;
      disp->bmi.bmiHeader.biXPelsPerMeter=1;
      disp->bmi.bmiHeader.biYPelsPerMeter=1;
      disp->bmi.bmiHeader.biClrUsed=0;
      disp->bmi.bmiHeader.biClrImportant=0;
      disp->data = new unsigned int[disp->width*disp->height];
      if (!disp->curr_mode.dmSize) { // Normal window
        RECT rect;
        rect.left=rect.top=0; rect.right=disp->width-1; rect.bottom=disp->height-1;
    AdjustWindowRect(&rect,WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,false);
    const int border1 = (rect.right-rect.left+1-disp->width)/2, border2 = rect.bottom-rect.top+1-disp->height-border1;
        disp->window = CreateWindow("MDICLIENT",title?title:"",
                                    WS_OVERLAPPEDWINDOW | (disp->closed?0:WS_VISIBLE), CW_USEDEFAULT,CW_USEDEFAULT,
                                    disp->width + 2*border1, disp->height + border1 + border2,
                    NULL,NULL,NULL,&(disp->ccs));
    if (!disp->closed) {
      GetWindowRect(disp->window,&rect);    
      disp->window_x = rect.left + border1;
      disp->window_y = rect.top + border2;
    } else disp->window_x = disp->window_y = 0;
      } else { // Fullscreen window
    disp->window = CreateWindow("MDICLIENT",title?title:"",
                    WS_POPUP | (disp->closed?0:WS_VISIBLE), CW_USEDEFAULT,CW_USEDEFAULT,
                    disp->width,disp->height,NULL,NULL,NULL,&(disp->ccs));
    disp->window_x = disp->window_y = 0;
      }
      SetForegroundWindow(disp->window);
      disp->hdc = GetDC(disp->window);
      disp->window_width = disp->width;
      disp->window_height = disp->height;
      disp->mouse_x = disp->mouse_y = -1;
      disp->button = disp->key = 0;
      disp->resized = disp->moved = false;      
      if (disp->events) {
        SetWindowLong(disp->window,GWL_USERDATA,(LONG)disp);
        SetWindowLong(disp->window,GWL_WNDPROC,(LONG)proc_lowlevel);
        SetEvent(disp->created);
        while( GetMessage( &msg, NULL, 0, 0 ) ) { DispatchMessage( &msg ); SetEvent(disp->wait_disp); }
      }
      return 0;
    }

    template<typename T> BITMAPINFO* render(const CImg<T>& img,const unsigned int ymin=0,const unsigned int ymax=~0) {
      cimg_test(img,"CImgDisplay::render");
      if (img.depth!=1) return render(img.get_3dplanes(img.width/2,img.height/2,img.depth/2),
                      (unsigned int)0,~(unsigned int)0);
      if (img.width!=width || img.height!=height) return render(img.get_resize(width,height,1,-100,1),
                                (unsigned int)0,~(unsigned int)0);
      const bool by=(ymin<=ymax);
      const unsigned int nymin = by?ymin:ymax, nymax = by?(ymax>=height?height-1:ymax):(ymin>=height?height-1:ymin);
      const T 
        *data1 = img.ptr(0,nymin,0,0),
        *data2 = (img.dim>=2)?img.ptr(0,nymin,0,1):data1,
        *data3 = (img.dim>=3)?img.ptr(0,nymin,0,2):data1;
      unsigned int *ximg = data + nymin*width;
      WaitForSingleObject(mutex,INFINITE);
      if (!normalization) for (unsigned int xy = (nymax-nymin+1)*width; xy>0; xy--)
    *(ximg++) = ((unsigned char)*(data1++)<<16) | ((unsigned char)*(data2++)<<8) | (unsigned char)*(data3++);
      else {
        if (normalization==1) { CImgStats st(img,false); min=st.min; max=st.max; }
        const T nmin = (T)min, delta = (T)(max-nmin), mm = delta?delta:(T)1;
    for (unsigned int xy = (nymax-nymin+1)*width; xy>0; xy--) {
          const unsigned char
            R = (unsigned char)(255*(*(data1++)-nmin)/mm),
            G = (unsigned char)(255*(*(data2++)-nmin)/mm),
            B = (unsigned char)(255*(*(data3++)-nmin)/mm);
          *(ximg++) = (R<<16) | (G<<8) | (B);
        }
      }
      ReleaseMutex(mutex);
      return &bmi;
    }

    template<typename T> CImgDisplay& display(const CImg<T>& img,const unsigned int pymin=0,const unsigned int pymax=~0) {
      cimg_test(img,"CImgDisplay::display");
      const unsigned int 
        ymin = pymin<pymax?pymin:pymax,
        ymax = pymin<pymax?(pymax>=height?height-1:pymax):(pymin>=height?height-1:pymin);
      render(img,ymin,ymax);
      if (!closed) {
        WaitForSingleObject(mutex,INFINITE);
        SetDIBitsToDevice(hdc,0,ymin,width,ymax-ymin+1,0,0,0,ymax-ymin+1,data+ymin*width,&bmi,DIB_RGB_COLORS);
        ReleaseMutex(mutex);
      }
      return *this;
    }
  
    CImgDisplay& wait() {
      if (!closed && events) WaitForSingleObject(wait_disp,INFINITE);
      return *this;
    }

    CImgDisplay& show() {
      if (closed) {
        ShowWindow(window,SW_SHOW);    
        SetDIBitsToDevice(hdc,0,0,width,height,0,0,0,height,data,&bmi,DIB_RGB_COLORS);
    RECT rect; 
    rect.left=rect.top=0; rect.right=width-1; rect.bottom=height-1;
    AdjustWindowRect(&rect,WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,false);
    const int border1 = (rect.right-rect.left+1-width)/2, border2 = rect.bottom-rect.top+1-height-border1;
    GetWindowRect(window,&rect);
    window_x = rect.left + border1;
    window_y = rect.top + border2;
        closed = false;
      }
      return *this;
    }

    CImgDisplay& close() {
      if (!closed) {
        ShowWindow(window,SW_HIDE);
        closed = true;
    window_x = window_y = 0;
      }
      return *this;
    }

    static const int screen_dimx() { 
      DEVMODE mode;
      mode.dmSize = sizeof(DEVMODE); mode.dmDriverExtra = 0;
      EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&mode);
      return mode.dmPelsWidth;      
    }

    static const int screen_dimy() {
      DEVMODE mode;
      mode.dmSize = sizeof(DEVMODE); mode.dmDriverExtra = 0;
      EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&mode);
      return mode.dmPelsHeight;      
    }

#endif

#ifdef cimgdisplay_plugin
#include cimgdisplay_plugin
#endif
    
  };

  /*-------------------------------------------------------



  
    Definition of the CImg<T> structure
    
    
    
    
  ------------------------------------------------------*/

  //! This structure represents an image (up to 4 dimensions wide), with pixels of type \c T.
  /**
     This is the main structure of the CImg Library. It allows to define an image,
     access its pixel values, and perform various operations on it.

     <b>* Image structure</b>

     A \ref CImg<\c T> structure contains only five fields :
     - \ref width defines the number of columns of the image.
     - \ref height defines the number of rows of the image.
     - \ref depth defines the number of slices of the image.
     - \ref dim defines the number of channels of the image.
     - \ref data defines a pointer to the pixel data (of type \c T).
     
     You can access these fields publicly although it is recommended to use dedicated functions
     dimx(), dimy(), dimz(), dimv() and ptr() to do so.     
     Image dimensions are not limited to a specific range (as long as you got enough RAM).
     A value of \e 1 usually means that the corresponding dimension is 'flat'.
     If one dimension is \e 0, the image is considered as an \e empty image.
     Empty images do not contain pixel data and thus, are not processed by most of CImg member functions.
     Most of the CImg<T> member functions are designed to work on images with general dimensions.     

     <b>* Image declaration</b>

     Declaration of an image uses one of the several available constructors. Below is a list of
     the most used :
     - Construct images from dimensions :
         - <tt>CImg<char> img;</tt> constructs an empty image.
         - <tt>CImg<unsigned char> img(128,128);</tt> constructs a 128x128 greyscale image with \c unsigned \c char pixel values.
         - <tt>CImg<double> img(3,3);</tt> constructs  a 3x3 matrix with \c double coefficients.
         - <tt>CImg<unsigned char> img(256,256,1,3);</tt> for a 256x256x1x3 (color) image (colors are stored as three channels).
         - <tt>CImg<double> img(128,128,128);</tt> for a 128x128x128 volumetric (greyscale) image (with \c double pixel values).
         - <tT>CImg<> img(128,128,128,3);</tt> for a 128x128x128 volumetric color image (with \c float pixels, which is
     the default value of the template parameter \c T).
     - \b Note : images pixels are <b>not automatically initialized to 0</b>. You may use the function \ref fill() to
     do it.
     - Construct images from filenames :
         - <tt>CImg<unsigned char> img("image.jpg");</tt> reads a color image from the disk.
     - <tt>CImg<float> img("analyze.hdr");</tt> reads a volumetric image with float pixel (ANALYZE7.5 format).
     - \b Note : You will need to install ImageMagick to be able to read compressed image formats (JPG,PNG,...)
     - Construct images from C-arrays :
         - <tt>CImg<int> img(data_buffer,256,256);</tt> convert a \c int buffer \c data_buffer to a 256x256 greyscale image.     

     More constructors are available (copy constructor,...). Please look at the constructor list for more
     informations.

  
  A CImg image is defined as a template class CImg<T> containing a pixel data field with a maximum of 4 dimensions :
  the 3 first dimensions are usually used to describe the spatial coordinates <tt>(x,y,z)</tt> in the image, while the last one
  is often used as a vector-valued pixel dimension (color channel for instance). Then, this class can handle the case of 3D volumes
  of vector-valued pixels, and all images that requires less dimensions (gray-valued or color 2D images for instance). 
  It also means that (almost) all member functions of the class CImg<T> are designed to handle the maximum case of these (3+1) dimensions.

  Moreover, the pixel type is given by the template parameter T, which means that you can define images with different pixel types T.
  Fully supported template types are the basic C++ types : <tt>unsigned char, char, short, unsigned int, int, float, double, ... </tt>
  Using your own template types is possible however, but you will have to redefine the complete set of arithmetic and logical operators.
  Typically, fast image display would be done using <tt>CImg<unsigned char></tt> images, while complex image processing algorithms would be coded
  using <tt>CImg<float></tt> or <tt>CImg<double></tt> images which have floating-point pixel values. Only two others classes are defined
  in the <tt>CImg.h</tt> file : <tt>CImgStats</tt> that is used to represent image statistics, and <tt>CImgDisplay</tt> used to open 
  windows where images are displayed, and handle keyboard and mouse events.

  CImg has been designed to be 'user-friendly', meaning that the underlying structure of the classes is always quite basic to understand.
  For instance, the <tt>CImg<T></tt> class members (which are defined as public) are only the dimensions
  <tt>width, height, depth, dim</tt>, and the pixel data <tt>data</tt>. Very useful when you want to access the raw pixel buffer
  for calling with functions of other libraries, or when you want to convert your raw data buffer into a <tt>CImg</tt>,
  in order to visualize it for instance. Moreover, most of the classical arithmetic and logical operators have been redefined
  in the <tt>CImg<T> class</tt>, which means that accessing a pixel is as simple as <tt>image(x,y)</tt> or <tt>image(x,y,z,v)</tt> (in 4D),
  and beautiful things are possible such as <tt>(img1+3*img2).display();</tt>


     \see \ref cimg_structure
  **/
  template<typename T> struct CImg {
    
    //! Number of columns in the instance image (size along the X-axis).
    /**
       \note
       - Prefer using CImg<T>::dimx() to get the width of the instance image.
       - Should be considered as \e read-only. Modifying directly \c CImg<T>::width would probably
       result in a crash.
       - This value can be modified through the CImg<T>::resize() function.
       - If CImg<T>::width==0, the image is empty and contains no pixel data.
    **/
    unsigned int width;       
    
    //! Number of rows in the instance image (size along the Y-axis).
    /**
       \note 
       - Prefer using CImg<T>::dimy() to get the height of the instance image.
       - Should be considered as \e read-only. Modifying directly \c CImg<T>::height would probably
       result in a crash.
       - This value can be modified through the CImg<T>::resize() function.
       - If CImg<T>::height==0, the image is empty and contains no pixel data.
    **/
    unsigned int height;
    
    //! Number of slices in the instance image (size along the Z-axis).
    /**
       \note 
       - Prefer using CImg<T>::dimz() to get the depth of the instance image.
       - Should be considered as \e read-only. Modifying directly \c CImg<T>::depth would probably
       result in a crash.
       - This value can be modified through the CImg<T>::resize() function.
       - If CImg<T>::depth==0, the image is empty and contains no pixel data.
    **/
    unsigned int depth;
    
    //! Number of vector channels in the instance image (size along the V-axis).
    /**
       \note 
       - Prefer using CImg<T>::dimv() to get the depth of the instance image.
       - Should be considered as \e read-only. Modifying directly \c CImg<T>::dim would probably
       result in a crash.
       - This value can be modified through the CImg<T>::resize() function.
       - If CImg<T>::dim==0, the image is empty and contains no pixel data.
    **/
    unsigned int dim;
    
    //! Pointer to pixel values (array of elements \c T).
    /**
       \note
       - Prefer using CImg<T>::ptr() to get a pointer to the pixel buffer.
       - Should be considered as \e read-only. Modifying directly \c CImg<T>::data would probably
       result in a crash.
       - If CImg<T>::data==NULL, the image is empty and contains no pixel data.
       \see \ref cimg_storage
    **/
    T *data;

    //------------------------------------------
    //------------------------------------------
    //
    //! \name Constructors - Destructor - Copy
    //@{
    //------------------------------------------
    //------------------------------------------
  
    //! Create an image of size (\c dx,\c dy,\c dz,\c dv) with pixels of type \c T.
    /**
       \param dx = number of columns of the created image (size along the X-axis).
       \param dy = number of rows of the created image (size along the Y-axis).
       \param dz = number of slices of the created image (size along the Z-axis).
       \param dv = number of vector channels of the created image (size along the V-axis).
       \note 
       - Pixel values are \e not \e initialized by this constructor (this can be done by another constructors).
       - If invoked without parameters, this constructor creates an \e empty image (default constructor).
       - The construction of an empty image does not allocate extra-memory.
    **/
    explicit CImg(const unsigned int dx=0,const unsigned int dy=1,const unsigned int dz=1,const unsigned int dv=1):
      width(dx),height(dy),depth(dz),dim(dv) {
      const unsigned int siz = size();
      if (siz) data = new T[siz]; else { data=NULL; width=height=depth=dim=0; }
    }

    //! Create an image of size (\c dx,\c dy,\c dz,\c dv) with pixels of type \c T,
    //! and set the image pixels to the value \c val.
    /**
       \param dx = number of columns of the created image (size along the X-axis).
       \param dy = number of rows of the created image (size along the Y-axis).
       \param dz = number of slices of the created image (size along the Z-axis).
       \param dv = number of vector channels of the created image (size along the V-axis).
       \param val = default value of the pixels.
    **/    
    explicit CImg(const unsigned int dx,const unsigned int dy,const unsigned int dz,const unsigned int dv,const T& val):
      width(dx),height(dy),depth(dz),dim(dv) {
      const unsigned int siz = size();
      if (siz) { data = new T[siz]; fill(val); } else { data=NULL; width=height=depth=dim=0; }
    }

    //! Copy constructor.
    /**
       \param img = the image to copy.
       \note
       - An optimized version of the copy constructor is used if the input and output image have same pixel types.
    **/
    template<typename t> CImg(const CImg<t>& img):width(img.width),height(img.height),depth(img.depth),dim(img.dim) {
      const unsigned int siz = size();
      if (siz) {
        data = new T[siz];
        const t *ptrs = img.data + siz;
        cimg_map(*this,ptrd,T) (*ptrd)=(T)*(--ptrs);
      } else data = NULL;
    }

    CImg(const CImg<T>& img):width(img.width),height(img.height),depth(img.depth),dim(img.dim) {
      const unsigned siz = size();
      if (siz) {
    data = new T[width*height*depth*dim];
    std::memcpy(data,img.data,siz*sizeof(T));
      } else data = NULL;
    }


    //! Copy constructor.
    /**
       \param img = the image to copy.
       \param pixel_copy = tells the constructor if the pixel data of the original image are copied into the created image.
       \note
       This may be useful when one wants to create an image with same size than other image, but without copying pixel data :
       \code
       CImg<float> source("image.jpg");
       CImg<unsigned char> destination(source,false);
       \endcode
       is equivalent to
       \code
       CImg<float> source("image.jpg");
       CImg<unsigned char> destination(source.dimx(),source.dimy(),source.dimz(),source.dimv());
       \endcode
    **/
    template<typename t> CImg(const CImg<t>& img,const bool pixel_copy):width(0),height(0),depth(0),dim(0),data(NULL) {
      if (pixel_copy) CImg<T>(img).swap(*this);
      CImg<T>(img.width,img.height,img.depth,img.dim).swap(*this);
    }

    //! Create an image by loading a file.
    /**
       \param filename = the filename of the image file.
       \note
       The filename extension implicitely informs CImg about the image format.
       \see cimg_files_io.
    **/
    CImg(const char *const filename):width(0),height(0),depth(0),dim(0),data(NULL) { load(filename).swap(*this); }

    //! Create an image from a data buffer.
    /**
       \param data_buffer = pointer \c t* to a buffer of pixel values t.
       \param dx = number of columns of the created image (size along the X-axis).
       \param dy = number of rows of the created image (size along the Y-axis).
       \param dz = number of slices of the created image (size along the Z-axis).
       \param dv = number of vector channels of the created image (size along the V-axis).
       \param multiplexed = If true, it indicates that the source buffer is multiplexed, i.e.
       data values are interlaced with respect to the v axis (useful for vector-valued/color images only).
       \see \ref cimg_storage
    **/
    template<typename t> CImg(const t *const data_buffer,
                  const unsigned int dx,const unsigned int dy=1,
                  const unsigned int dz=1,const unsigned int dv=1,
                  const bool multiplexed=false):width(dx),height(dy),depth(dz),dim(dv) {
      const unsigned int siz = size();
      if (data_buffer && siz) {
        data = new T[siz];
    if (multiplexed) {
      const t *ptrs = data_buffer;
      T *ptrd = data;
      cimg_mapV(*this,k) { cimg_mapXYZ(*this,x,y,z) { *(ptrd++) = (T)(*(ptrs)); ptrs+=dim; } ptrs-=siz-1; }
    } else { const t *ptrs = data_buffer+siz; cimg_map(*this,ptrd,T) *ptrd = (T)(*(--ptrs)); }
      } else { width=height=depth=dim=0; data = NULL; }
    }
    // Add template overloading if VC>7.1 (optimized version)
#if !defined(_MSC_VER) && _MSC_VER>1300
    CImg(const T *const data_buffer,
     const unsigned int dx,const unsigned int dy=1,
     const unsigned int dz=1,const unsigned int dv=1,
     const bool multiplexed=false):width(dx),height(dy),depth(dz),dim(dv) {
      const unsigned int siz = size();
      if (data_buffer && siz) {
        data = new T[siz];
    if (multiplexed) {
      const T *ptrs = data_buffer;
      T *ptrd = data;
      cimg_mapV(*this,k) { cimg_mapXYZ(*this,x,y,z) { *(ptrd++) = (T)(*(ptrs)); ptrs+=dim; } ptrs-=siz-1; }
    } else std::memcpy(data,data_buffer,siz*sizeof(T));
      } else { width=height=depth=dim=0; data = NULL; }
    }
#endif
    
    //! Destructor.
    /**
       \note The destructor frees the memory eventually used by the image pixels.
    **/
    ~CImg() { if (data) delete[] data; }
    
    //! Empty the instance image
    /**
       \note Equivalent to
       \code
       (*this) = CImg<T>();
       \endcode
    **/
    CImg& empty() { return CImg<T>().swap(*this); }

    //@}
    //-----------------------------------------------------
    //-----------------------------------------------------
    //
    //! \name Access to image dimensions and pixel values
    //@{
    //-----------------------------------------------------
    //-----------------------------------------------------
  
    //! Return the type of the pixel values
    /**
       \return a string describing the type of the image pixels (template parameter \p T).
       \note
       - The string returned may contains spaces (for instance "unsigned char").
       - If the template parameter T does not correspond to a registered type, the string "unknown" is returned.
    **/
    static const char* pixel_type() { T val; return cimg::get_type(val); }

    //! Return the number of pixel values of an image.
    /**
       \return dimx()*dimy()*dimz()*dimv()
       \see dimx(), dimy(), dimz(), dimv()
    **/
    const unsigned int size() const { return width*height*depth*dim; }  

    //! Return the number of columns of the instance image (size along the X-axis).
    /**
       \return this->width       
       \see dimy(),dimz(),dimv(),size()
       \note
       - It is preferable to use img.dimx() instead of img.width since the latter is unsigned and
       may perturb arithmetics with signed numbers.
    **/
    const int dimx() const { return (int)width; }  

    //! Return the number of rows of the instance image (size along the Y-axis).
    /**
       \return this->height
       \see dimx(),dimz(),dimv(),size().
       \note
       - It is preferable to use img.dimy() instead of img.height since the latter is unsigned and
       may perturb arithmetics with signed numbers.
    **/
    const int dimy() const { return (int)height; }
  
    //! Return the number of slices of the instance image (size along the Z-axis).
    /**
       \return this->depth
       \see dimx(),dimy(),dimv(),size().
       - It is preferable to use img.dimz() instead of img.depth since the latter is unsigned and
       may perturb arithmetics with signed numbers.
    **/
    const int dimz() const { return (int)depth; }
  
    //! Return the number of vector channels of the instance image (size along the V-axis).
    /**
       \return this->dim
       \see dimx(),dimy(),dimz(),size().
       - It is preferable to use img.dimv() instead of img.dim since the latter is unsigned and
       may perturb arithmetics with signed numbers.
    **/
    const int dimv() const { return (int)dim; }
  
    //! Return the offset corresponding to the location of the pixel value located at (\p x,\p y,\p z,\p v)
    // with respect to the pixel data pointer \ref data.
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
       \note No tests are made on the validity of the given coordinates.
       \see cimg_storage.
    **/
    const long offset(const int x=0, const int y=0, const int z=0, const int v=0) const { 
      return x+width*(y+height*(z+depth*v)); }
  
    //! Return a pointer to the pixel value located at (\p x,\p y,\p z,\p v).
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
       \note If the macro \p cimg_debug == 2, an extra test is made and a warning message appears if 
       input coordinates are outside the image range.
       \see cimg_storage, cimg_environment.
    **/
    T* ptr(const unsigned int x=0, const unsigned int y=0, const unsigned int z=0, const unsigned int v=0) const {
      const long off = offset(x,y,z,v);
#if cimg_debug>1
      if (off<0 || off>=size()) {
        cimg::warn(true,"CImg<%s>::ptr() : Trying to get a pointer at (%u,%u,%u,%u) (offset=%d) which is"
           "outside the data of the image (%u,%u,%u,%u) (size=%u)",
           pixel_type(),x,y,z,v,off,width,height,depth,dim,size());
        return data;
      }
#endif
      return data+off;
    }

    //! Access to pixel value for reading or writing, without boundary checking.
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel       
       \note If the macro \p cimg_debug == 2, a boundary checking is performed (but it slows down the code).
    **/
    T& operator()(const unsigned int x,const unsigned int y=0,const unsigned int z=0,const unsigned int v=0) const {
      const int off = offset(x,y,z,v);
#if cimg_debug>1
      if (!data || off>=(int)size()) {
        cimg::warn(true,"CImg<%s>::operator() : Pixel access requested at (%u,%u,%u,%u) (offset=%d) "
           "outside the image range (%u,%u,%u,%u) (size=%u)",
                   pixel_type(),x,y,z,v,off,width,height,depth,dim,size());            
        return *data;
      }
#endif
      return data[off];
    }
    
    //! Access to pixel buffer value for reading or writing.
    /**
       \param off = offset in the pixel buffer
       \note if \c cimg_debug==2, a out-of-buffer test is performed (but it slows down the code).
    **/    
    T& operator[](const unsigned long off) const {
#if cimg_debug>1
      if (!data || off>=size()) {
        cimg::warn(true,
                   "CImg<%s>::operator[] : Trying to get a pixel at offset=%d, outside the range of the image (%u,%u,%u,%u) (size=%u)",
                   pixel_type(),off,width,height,depth,dim,size());            
        return *data;
      }
#endif
      return data[off];
    }

    //! Pixel access with Dirichlet boundary conditions for all coordinates (x,y,z,v).
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
       \param out_val = returned value if pixel coordinates is out of the image range.
    **/
    T dirichlet_pix4d(const int x,const int y=0,const int z=0,const int v=0,const T out_val=(T)0) const {
      return (x<0 || y<0 || z<0 || v<0 || x>=dimx() || y>=dimy() || z>=dimz() || v>=dimv())?out_val:(*this)(x,y,z,v);
    }

    //! Pixel access with Dirichlet boundary conditions for the three first coordinates (x,y,z).
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
       \param out_val = returned value if pixel coordinates is out of the image range.
    **/
    T dirichlet_pix3d(const int x,const int y=0,const int z=0,const int v=0,const T out_val=(T)0) const {
      return (x<0 || y<0 || z<0 || x>=dimx() || y>=dimy() || z>=dimz())?out_val:(*this)(x,y,z,v);
    }
    //! Pixel access with Dirichlet boundary conditions for the two first coordinates (x,y).
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
       \param out_val = returned value if pixel coordinates is out of the image range.
    **/
    T dirichlet_pix2d(const int x,const int y=0,const int z=0,const int v=0,const T out_val=(T)0) const {
      return (x<0 || y<0 || x>=dimx() || y>=dimy())?out_val:(*this)(x,y,z,v);
    }

    //! Pixel access with Dirichlet boundary conditions for the first coordinate x.
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
       \param out_val = returned value if pixel coordinates is out of the image range.
    **/
    T dirichlet_pix1d(const int x,const int y=0,const int z=0,const int v=0,const T out_val=(T)0) const {
      return (x<0 || x>=dimx())?out_val:(*this)(x,y,z,v);
    }

    //! Pixel access with Neumann boundary conditions for all coordinates (x,y,z,v).
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
    **/
    const T& neumann_pix4d(const int x,const int y=0,const int z=0,const int v=0) const {
      return (*this)(x<0?0:(x>=dimx()?dimx()-1:x),
                     y<0?0:(y>=dimy()?dimy()-1:y),
                     z<0?0:(z>=dimz()?dimz()-1:z),
                     v<0?0:(v>=dimv()?dimv()-1:v));
    }
    //! Pixel access with Neumann boundary conditions for the three first coordinates (x,y,z).
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
    **/
    const T& neumann_pix3d(const int x,const int y=0,const int z=0,const int v=0) const {
      return (*this)(x<0?0:(x>=dimx()?dimx()-1:x),
                     y<0?0:(y>=dimy()?dimy()-1:y),
                     z<0?0:(z>=dimz()?dimz()-1:z),v);
    }

    //! Pixel access with Neumann boundary conditions for the two first coordinates (x,y).
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
    **/
    const T& neumann_pix2d(const int x,const int y=0,const int z=0,const int v=0) const {
      return (*this)(x<0?0:(x>=dimx()?dimx()-1:x),
                     y<0?0:(y>=dimy()?dimy()-1:y),z,v);
    }
    //! Pixel access with Neumann boundary conditions for the first coordinate x.
    /**
       \param x = x-coordinate of the pixel
       \param y = y-coordinate of the pixel
       \param z = z-coordinate of the pixel
       \param v = v-coordinate of the pixel
    **/
    const T& neumann_pix1d(const int x,const int y=0,const int z=0,const int v=0) const {
      return (*this)(x<0?0:(x>=dimx()?dimx()-1:x),y,z,v);
    }
    
    //! Pixel access with Neumann boundary conditions and linear interpolation for all coordinates (x,y,z,v).
    /**
       \param ffx = x-coordinate of the pixel (float value)
       \param ffy = y-coordinate of the pixel (float value)
       \param ffz = z-coordinate of the pixel (float value)
       \param ffv = v-coordinate of the pixel (float value)
    **/
    double linear_pix4d(const float ffx,const float ffy=0,const float ffz=0,const float ffv=0) const {
      double valx0,valx1,valy0,valy1,valz0,valz1;
      const float fx = ffx<0?0:(ffx>width-1?width-1:ffx), fy = ffy<0?0:(ffy>height-1?height-1:ffy),
        fz = ffz<0?0:(ffz>depth-1?depth-1:ffz), fv = ffv<0?0:(ffv>dim-1?dim-1:ffv);
      const unsigned int x = (unsigned int)fx, y = (unsigned int)fy,  z = (unsigned int)fz, v = (unsigned int)fv;
      const float dx = fx-x, dy = fy-y, dz = fz-z, dv = fv-v;
      const unsigned int nx = dx>0?x+1:x, ny = dy>0?y+1:y,  nz = dz>0?z+1:z, nv = dv>0?v+1:v;
      valx0 = (1-dx)*(*this)(x,y,z,v)  + (dx)*(*this)(nx,y,z,v);
      valx1 = (1-dx)*(*this)(x,ny,z,v) + (dx)*(*this)(nx,ny,z,v);
      valy0 = (1-dy)*valx0 + (dy)*valx1;
      valx0 = (1-dx)*(*this)(x,y,nz,v)  + (dx)*(*this)(nx,y,nz,v);
      valx1 = (1-dx)*(*this)(x,ny,nz,v) + (dx)*(*this)(nx,ny,nz,v);
      valy1 = (1-dy)*valx0 + (dy)*valx1;
      valz0 = (1-dz)*valy0 + (dz)*valy1;
      valx0 = (1-dx)*(*this)(x,y,z,nv)  + (dx)*(*this)(nx,y,z,nv);
      valx1 = (1-dx)*(*this)(x,ny,z,nv) + (dx)*(*this)(nx,ny,z,nv);
      valy0 = (1-dy)*valx0 + (dy)*valx1;
      valx0 = (1-dx)*(*this)(x,y,nz,nv)  + (dx)*(*this)(nx,y,nz,nv);
      valx1 = (1-dx)*(*this)(x,ny,nz,nv) + (dx)*(*this)(nx,ny,nz,nv);
      valy1 = (1-dy)*valx0 + (dy)*valx1;
      valz1 = (1-dz)*valy0 + (dz)*valy1;
      return (1-dv)*valz0 + (dv)*valz1;
    }

    //! Pixel access with Neumann boundary conditions and linear interpolation for the three first coordinates (x,y,z).
    /**
       \param ffx = x-coordinate of the pixel (float value)
       \param ffy = y-coordinate of the pixel (float value)
       \param ffz = z-coordinate of the pixel (float value)
       \param v = v-coordinate of the pixel (integer value)
    **/
    double linear_pix3d(const float ffx,const float ffy=0,const float ffz=0,const int v=0) const {
      double valx0,valx1,valy0,valy1;
      const float fx = ffx<0?0:(ffx>width-1?width-1:ffx), fy = ffy<0?0:(ffy>height-1?height-1:ffy), fz = ffz<0?0:(ffz>depth-1?depth-1:ffz);
      const unsigned int x = (unsigned int)fx, y = (unsigned int)fy, z = (unsigned int)fz;
      const float dx = fx-x, dy = fy-y, dz = fz-z;
      const unsigned int nx = dx>0?x+1:x, ny = dy>0?y+1:y, nz = dz>0?z+1:z;
      valx0 = (1-dx)*(*this)(x,y,z,v)  + (dx)*(*this)(nx,y,z,v);
      valx1 = (1-dx)*(*this)(x,ny,z,v) + (dx)*(*this)(nx,ny,z,v);
      valy0 = (1-dy)*valx0 + (dy)*valx1;
      valx0 = (1-dx)*(*this)(x,y,nz,v)  + (dx)*(*this)(nx,y,nz,v);
      valx1 = (1-dx)*(*this)(x,ny,nz,v) + (dx)*(*this)(nx,ny,nz,v);
      valy1 = (1-dy)*valx0 + (dy)*valx1;
      return (1-dz)*valy0 + (dz)*valy1;
    }

    //! Pixel access with Neumann boundary conditions and linear interpolation for the two first coordinates (x,y).
    /**
       \param ffx = x-coordinate of the pixel (float value)
       \param ffy = y-coordinate of the pixel (float value)
       \param z = z-coordinate of the pixel (integer value)
       \param v = v-coordinate of the pixel (integer value)
    **/
    double linear_pix2d(const float ffx,const float ffy=0,const int z=0,int v=0) const {
      double valx0,valx1;
      const float fx = ffx<0?0:(ffx>width-1?width-1:ffx), fy = ffy<0?0:(ffy>height-1?height-1:ffy);
      const unsigned int x = (unsigned int)fx, y = (unsigned int)fy;
      const float dx = fx-x, dy = fy-y;
      const unsigned int nx = dx>0?x+1:x, ny = dy>0?y+1:y;
      valx0 = (1-dx)*(*this)(x,y,z,v)  + (dx)*(*this)(nx,y,z,v);
      valx1 = (1-dx)*(*this)(x,ny,z,v) + (dx)*(*this)(nx,ny,z,v);
      return (1-dy)*valx0 + (dy)*valx1;
    }

    //! Pixel access with Neumann boundary conditions and linear interpolation for the first coordinate x.
    /**
       \param ffx = x-coordinate of the pixel (float value)
       \param y = y-coordinate of the pixel (integer value)
       \param z = z-coordinate of the pixel (integer value)
       \param v = v-coordinate of the pixel (integer value)
    **/
    double linear_pix1d(const float ffx,const int y=0,const int z=0,int v=0) const {
      const float fx = ffx<0?0:(ffx>width-1?width-1:ffx);
      const unsigned int x = (unsigned int)fx;
      const float dx = fx-x;
      const unsigned int nx = dx>0?x+1:x;
      return (1-dx)*(*this)(x,y,z,v)  + (dx)*(*this)(nx,y,z,v);
    }

    //! Pixel access with Neumann boundary conditions and cubic interpolation for the two first coordinates (x,y).
    /**
       \param pfx = x-coordinate of the pixel (float value)
       \param pfy = y-coordinate of the pixel (float value)
       \param z = z-coordinate of the pixel (integer value)
       \param v = v-coordinate of the pixel (integer value)
    **/
    double cubic_pix2d(const float pfx,const float pfy=0,const int z=0,int v=0) const {
      const float fx = pfx<0?0:(pfx>width-1?width-1:pfx), fy = pfy<0?0:(pfy>height-1?height-1:pfy);
      const unsigned int 
        x = (unsigned int)fx,  px = (int)x-1>=0?x-1:0, nx = x+1<width?x+1:width-1, ax = nx+1<width?nx+1:width-1,
        y = (unsigned int)fy,  py = (int)y-1>=0?y-1:0, ny = y+1<height?y+1:height-1, ay = ny+1<height?ny+1:height-1;
      const float dx = fx-x, dy = fy-y;
      const T& 
        a = (*this)(px,py,z,v), b = (*this)(x,py,z,v), c = (*this)(nx,py,z,v), d = (*this)(ax,py,z,v),
        e = (*this)(px, y,z,v), f = (*this)(x, y,z,v), g = (*this)(nx, y,z,v), h = (*this)(ax, y,z,v),
        i = (*this)(px,ny,z,v), j = (*this)(x,ny,z,v), k = (*this)(nx,ny,z,v), l = (*this)(ax,ny,z,v),
        m = (*this)(px,ay,z,v), n = (*this)(x,ay,z,v), o = (*this)(nx,ay,z,v), p = (*this)(ax,ay,z,v);
      const double 
        A = dx*dx*dx*(2*(b-c)+0.5*(c-a+d-b)) + dx*dx*(2*c-2.5*b+a-0.5*d) + dx*0.5*(c-a) + b,
        B = dx*dx*dx*(2*(f-g)+0.5*(g-e+h-f)) + dx*dx*(2*g-2.5*f+e-0.5*h) + dx*0.5*(g-e) + f,
        C = dx*dx*dx*(2*(j-k)+0.5*(k-i+l-j)) + dx*dx*(2*k-2.5*j+i-0.5*l) + dx*0.5*(k-i) + j,
        D = dx*dx*dx*(2*(n-o)+0.5*(o-m+p-n)) + dx*dx*(2*o-2.5*n+m-0.5*p) + dx*0.5*(o-m) + n;
      return dy*dy*dy*(2*(B-C)+0.5*(C-A+D-B)) + dy*dy*(2*C-2.5*B+A-0.5*D) + dy*0.5*(C-A) + B;
    }

    //! Pixel access with Neumann boundary conditions and cubic interpolation for all coordinates x.
    /**
       \param pfx = x-coordinate of the pixel (float value)
       \param y = y-coordinate of the pixel (integer value)
       \param z = z-coordinate of the pixel (integer value)
       \param v = v-coordinate of the pixel (integer value)
    **/
    double cubic_pix1d(const float pfx,const int y=0,const int z=0,int v=0) const {
      const float fx = pfx<0?0:(pfx>width-1?width-1:pfx);
      const unsigned int x = (unsigned int)fx, px = (int)x-1>=0?x-1:0, nx = x+1<width?x+1:width-1, ax = nx+1<width?nx+1:width-1;
      const float dx = fx-x;
      const T& a = (*this)(px,y,z,v), b = (*this)(x,y,z,v), c = (*this)(nx,y,z,v), d = (*this)(ax,y,z,v);
      return dx*dx*dx*(2*(b-c)+0.5*(c-a+d-b)) + dx*dx*(2*c-2.5*b+a-0.5*d) + dx*0.5*(c-a) + b;
    }
    
    //! Print image information on the standart error output
    /**
       \param title = name of your printed variable.
       \param print_flag = - 0 : print only informations about image size and pixel buffer.
                           - 1 : print also statistics on the image pixels.
               - 2 : print also the pixel buffer content, in a matlab-style.
    **/
    const CImg& print(const char *title=NULL,const unsigned int print_flag=1) const {
      std::fprintf(stderr,"%-8s(this=%p): { size=(%u,%u,%u,%u), data=(%s*)%p",
           title?title:"CImg",(void*)this,
           width,height,depth,dim,pixel_type(),(void*)data);
      if (size()==0 || !data) { std::fprintf(stderr,", [Undefined pixel data] }\n"); return *this; }
      if (print_flag>=1) { 
        CImgStats st(*this);
        std::fprintf(stderr,", min=%g, mean=%g [var=%g], max=%g, pmin=(%d,%d,%d,%d), pmax=(%d,%d,%d,%d)",
             st.min,st.mean,st.variance,st.max,st.xmin,st.ymin,st.zmin,st.vmin,st.xmax,st.ymax,st.zmax,st.vmax); 
      }     
      if (print_flag>=2 || size()<=16) {
    std::fprintf(stderr," }\n%s = [ ",title?title:"data");
    cimg_mapXYZV(*this,x,y,z,k) 
      std::fprintf(stderr,"%g%s",(double)(*this)(x,y,z,k),
               ((x+1)*(y+1)*(z+1)*(k+1)==(int)size()?" ]\n":(((x+1)%width==0)?" ; ":" ")));
      } else std::fprintf(stderr," }\n");
      return *this;
    }

    //! Print image information on the standart error output
    /**
      \param print_flag = - 0 : print only informations about image size and pixel buffer.
                           - 1 : print also statistics on the image pixels.
               - 2 : print also the pixel buffer content, in a matlab-style.       
    **/
    const CImg& print(const unsigned int print_flag) const { return print(NULL,print_flag); }
  
    //@}
    //--------------------------------------------------
    //--------------------------------------------------
    //
    //! \name Common arithmetics and boolean operators
    //@{
    //--------------------------------------------------
    //--------------------------------------------------
  
    //! Copy an image to the instance image.
    /**
       \param img = image to copy.
       \note If pixel types are different, a type cast is performed.
    **/
    template<typename t> CImg<T>& operator=(const CImg<t>& img) { 
      const unsigned int sizes = img.size(), sized = size();
      if (sizes!=sized) return CImg<T>(img).swap(*this); 
      const t* ptrs = img.data + sized;
      for (T *ptrd = data+sized; ptrd>data; ) *(--ptrd) = (T)*(--ptrs);
      width = img.width; height = img.height; depth = img.depth; dim = img.dim;
      return *this;
    }
    
    CImg& operator=(const CImg& img) {
      if (&img==this) return *this;
      const unsigned int sizes = img.size(), sized = size();
      if (sizes!=sized) return CImg<T>(img).swap(*this); 
      std::memcpy(data,img.data,sizeof(T)*sized);
      width = img.width; height = img.height; depth = img.depth; dim = img.dim;
      return *this;
    }
      
    //! Assign a scalar value to all pixels of the instance image
    /**
       \param val = the value to assign.
    **/
    CImg& operator=(const T& val) { return fill(val); }

    //! Copy the content of a C-array to the pixel buffer of the instance image
    /**
       \param buf = pointer to a C-array, with a size of (at least) this->size().
    **/
    CImg& operator=(const T *buf) {
      if (buf) std::memcpy(data,buf,size()*sizeof(T));
      else throw CImgArgumentException("CImg<T>::operator=() : Given array pointer 'ptr' is NULL");
      return *this; 
    }
       
    //! Operator+=
    /**
       \param val = value to add to each pixel value of the instance image.
    **/
    CImg& operator+=(const T& val) { cimg_map(*this,ptr,T) (*ptr)+=val; return *this; }

    //! Operator++
    CImg& operator++() { return (*this)+=1; }

    //! Operator-=
    /**
       \param val = value to sub to each pixel value of the instance image.
    **/
    CImg& operator-=(const T& val) { cimg_map(*this,ptr,T) (*ptr)-=val; return *this; }

    //! Operator--
    CImg& operator--() { return (*this)-=1; }

    //! Operator%=
    /**
       \param val = value of the modulo applied to each pixel value of the instance image.
    **/
    CImg& operator%=(const T& val) { cimg_map(*this,ptr,T) (*ptr)%=val; return *this; }

    //! Operator&=
    /**
       \param val = value of the bitwise AND applied to each pixel value of the instance image.
     **/
    CImg& operator&=(const T& val) { cimg_map(*this,ptr,T) (*ptr)&=val; return *this; }

    //! Operator|=
    CImg& operator|=(const T& val) { cimg_map(*this,ptr,T) (*ptr)|=val; return *this; }

    //! Operator^=
    CImg& operator^=(const T& val) { cimg_map(*this,ptr,T) (*ptr)^=val; return *this; }

    //! Operator+
    CImg operator+(const T& val) const { return CImg<T>(*this)+=val; }

    //! Operator-
    CImg operator-(const T& val) const { return CImg<T>(*this)-=val; }

    //! Operator%
    CImg operator%(const T& val) const { return CImg<T>(*this)%=val; }  

    //! Operator&
    CImg operator&(const T& val) const { return CImg<T>(*this)&=val; }

    //! Operator|
    CImg operator|(const T& val) const { return CImg<T>(*this)|=val; }

    //! Operator^
    CImg operator^(const T& val) const { return CImg<T>(*this)^=val; }

    //! Operator!
    CImg operator!() const {
      CImg<T> res(*this,false);
      const T *ptrs = ptr() + size();
      cimg_map(res,ptrd,T) *ptrd=!(*(--ptrs));
      return res;
    }

    //! Operator~
    CImg operator~() const {
      CImg<T> res(*this,false);
      const T *ptrs = ptr() + size();
      cimg_map(res,ptrd,T) *ptrd=~(*(--ptrs));
      return res;
    }
    
    //! Operator+=
    template<typename t> CImg& operator+=(const CImg<t>& img) {
      const unsigned int smin = cimg::min(size(),img.size());
      t *ptrs = img.data+smin;
      for (T *ptrd = data+smin; ptrd>data; --ptrd, (*ptrd)=(T)((*ptrd)+(*(--ptrs))));
      return *this;
    }
    //! Operator-=
    template<typename t> CImg& operator-=(const CImg<t>& img) {
      const unsigned int smin = cimg::min(size(),img.size());
      t *ptrs = img.data+smin;
      for (T *ptrd = data+smin; ptrd>data; --ptrd, (*ptrd)=(T)((*ptrd)-(*(--ptrs))));
      return *this;
    }
    //! Operator%=
    CImg& operator%=(const CImg& img) {
      const unsigned int smin = cimg::min(size(),img.size());
      for (T *ptrs=img.data+smin, *ptrd=data+smin; ptrd>data; *(--ptrd)%=*(--ptrs));
      return *this;
    }
    //! Operator&=
    CImg& operator&=(const CImg& img) {
      const unsigned int smin = cimg::min(size(),img.size());
      for (T *ptrs=img.data+smin, *ptrd=data+smin; ptrd>data; *(--ptrd)&=*(--ptrs));
      return *this;
    }
    //! Operator|=
    CImg& operator|=(const CImg& img) {
      const unsigned int smin = cimg::min(size(),img.size());
      for (T *ptrs=img.data+smin, *ptrd=data+smin; ptrd>data; *(--ptrd)|=*(--ptrs));
      return *this;
    }
    //! Operator^=
    CImg& operator^=(const CImg& img) {
      const unsigned int smin = cimg::min(size(),img.size());
      for (T *ptrs=img.data+smin, *ptrd=data+smin; ptrd>data; *(--ptrd)^=*(--ptrs));
      return *this;
    }

    //! Operator+
    template<typename t> CImg operator+(const CImg<t>& img)  const { return CImg<T>(*this)+=img; }

    //! Operator-
    template<typename t> CImg operator-(const CImg<t>& img)  const { return CImg<T>(*this)-=img; }

    //! Operator%
    CImg operator%(const CImg& img) const { return CImg<T>(*this)%=img; }

    //! Operator&
    CImg operator&(const CImg& img) const { return CImg<T>(*this)&=img; } 

    //! Operator|
    CImg operator|(const CImg& img) const { return CImg<T>(*this)|=img; }

    //! Operator^
    CImg operator^(const CImg& img) const { return CImg<T>(*this)^=img; }  

    //! Operator*=
    CImg& operator*=(const double val) { cimg_map(*this,ptr,T) (*ptr)=(T)((*ptr)*val); return *this; }

    //! Operator/=
    CImg& operator/=(const double val) { cimg_map(*this,ptr,T) (*ptr)=(T)((*ptr)/val); return *this; }

    //! Operator*
    CImg operator*(const double val) const { return CImg<T>(*this)*=val; }

    //! Operator/
    CImg operator/(const double val) const { return CImg<T>(*this)/=val; }

    //! Operator+
    friend CImg operator+(const T& val, const CImg& img) { return CImg<T>(img)+=val; }

    //! Operator*
    friend CImg operator*(const double val,const CImg &img) { return CImg<T>(img)*=val; }

    //! Operator-
    friend CImg operator-(const T& val, const CImg& img) { return CImg<T>(img.width,img.height,img.depth,img.dim,val)-=img; }

    //! Operator==
    template<typename t> const bool operator==(const CImg<t>& img) const {
      const unsigned int siz = size();
      bool vequal = true;
      if (siz!=img.size()) return false;
      t *ptrs=img.data+siz;
      for (T *ptrd=data+siz; vequal && ptrd>data; vequal=vequal&&((*(--ptrd))==(*(--ptrs))));
      return vequal;
    }
    //! Operator!=
    template<typename t> const bool operator!=(const CImg<t>& img) const { return !((*this)==img); }

    //@}
    //--------------------------------------------------
    //--------------------------------------------------
    //
    //! \name Usual mathematical operations
    //@{
    //--------------------------------------------------
    //--------------------------------------------------
     
    //! Replace the image by the pointwise multiplication between \p *this and \p img.
    /**
       This is the in-place version of get_mul().
       \see get_mul().
    **/
    template<typename t> CImg& mul(const CImg<t>& img) {
      t *ptrs = img.data;
      T *ptrf = data + cimg::min(size(),img.size());
      for (T* ptrd = data; ptrd<ptrf; ptrd++) (*ptrd)=(T)(*ptrd*(*(ptrs++)));
      return *this;
    }

    //! Return the image corresponding to the pointwise multiplication between \p *this and \p img.
    /**
       \param img = argument of the multiplication.
       \note if \c *this and \c img have different size, the multiplication is applied
       only on possible values.
       \see get_div(),mul(),div()
    **/
    template<typename t> CImg get_mul(const CImg<t>& img) const { return CImg<T>(*this).mul(img); }
  
    //! Replace the image by the pointwise division between \p *this and \p img.
    /**
       This is the in-place version of get_div().
       \see get_div().
    **/
    template<typename t> CImg& div(const CImg<t>& img) {
      t *ptrs = img.data;
      T *ptrf = data + cimg::min(size(),img.size());
      for (T* ptrd = data; ptrd<ptrf; ptrd++) (*ptrd)=(T)(*ptrd/(*(ptrs++)));
      return *this;
    }

    //! Return an image from a pointwise division between \p *this and \p img.
    /**
       \param img = argument of the division.
       \note if \c *this and \c img have different size, the division is applied
       only on possible values.
       \see get_mul(),mul(),div()
    **/
    template<typename t> CImg get_div(const CImg<t>& img) const { return CImg<T>(*this).div(img); }
  
    //! Replace the image by the pointwise max operator between \p *this and \p img
    /**
       This is the in-place version of get_max().       
       \see get_max().
    **/
    template<typename t> CImg& max(const CImg<t>& img) {
      t *ptrs = img.data;
      T *ptrf = data + cimg::min(size(),img.size());
      for (T* ptrd = data; ptrd<ptrf; ptrd++) (*ptrd)=cimg::max((T)(*(ptrs++)),*ptrd);
      return *this;
    }
    //! Return the image corresponding to the max value for each pixel.
    /**
       \param img = second argument of the max operator (the first one is *this).
       \see max(), min(), get_min()
    **/
    template<typename t> CImg get_max(const CImg<t>& img) const { return CImg<T>(*this).max(img); }
  
    //! Replace the image by the pointwise min operator between \p *this and \p img
    /**
       This is the in-place version of get_min().
       \see get_min().
    **/
    template<typename t> CImg& min(const CImg<t>& img) {
      t *ptrs = img.data;
      T *ptrf = data + cimg::min(size(),img.size());
      for (T* ptrd = data; ptrd<ptrf; ptrd++) (*ptrd)=cimg::min((T)(*(ptrs++)),*ptrd);
      return *this;
    }
    //! Return the image corresponding to the min value for each pixel.
    /**
       \param img = second argument of the min operator (the first one is *this).
       \see min(), max(), get_max()
    **/
    template<typename t> CImg get_min(const CImg<t>& img) const { return CImg<T>(*this).min(img); }

    //! Replace each image pixel by its square root.
    /**
       \see get_sqrt()
    **/
    CImg& sqrt() {
      cimg_map(*this,ptr,T) (*ptr)=(T)std::sqrt((double)(*ptr));
      return *this;
    }

    //! Return the image of the square root of the pixel values.
    /**
       \see sqrt()
    **/
    CImg get_sqrt() const { return CImg<T>(*this).sqrt(); }
  
    //! Replace each image pixel by its log.
    /**
       \see get_log(), log10(), get_log10()
    **/
    CImg& log() {
      cimg_map(*this,ptr,T) (*ptr)=(T)std::log((double)(*ptr));
      return *this;
    }

    //! Return the image of the log of the pixel values.
    /**
       \see log(), log10(), get_log10()
    **/
    CImg get_log() const { return CImg<T>(*this).log(); }

    //! Replace each image pixel by its log10.
    /**
       \see get_log10(), log(), get_log()
    **/
    CImg& log10() {
      cimg_map(*this,ptr,T) (*ptr)=(T)std::log10((double)(*ptr));
      return *this;
    }

    //! Return the image of the log10 of the pixel values.
    /**
       \see log10(), log(), get_log()
    **/
    CImg get_log10() const { return CImg<T>(*this).log10(); }

    //! Replace each image pixel by its power by \p p.
    /**
       \param p = power
       \see get_pow(), sqrt(), get_sqrt()
    **/
    CImg& pow(const double p) {
      if (p==0) return fill(1);
      if (p==1) return *this;
      if (p==2) { cimg_map(*this,ptr,T) { const T& val = *ptr; *ptr=val*val; } return *this; }
      if (p==3) { cimg_map(*this,ptr,T) { const T& val = *ptr; *ptr=val*val*val; } return *this; }
      if (p==4) { cimg_map(*this,ptr,T) { const T& val = *ptr; *ptr=val*val*val*val; } return *this; }
      cimg_map(*this,ptr,T) (*ptr)=(T)std::pow((double)(*ptr),p);
      return *this;
    }

    //! Return the image of the square root of the pixel values.
    /**
       \param p = power
       \see pow(), sqrt(), get_sqrt()
    **/
    CImg get_pow(const double p) const { return CImg<T>(*this).pow(p); }
  
    //! Replace each pixel value by its absolute value.
    /**
       \see get_abs()
    **/
    CImg& abs() {
      cimg_map(*this,ptr,T) (*ptr)=cimg::abs(*ptr);
      return *this;
    }

    //! Return the image of the absolute value of the pixel values.
    /**
       \see abs()
    **/
    CImg get_abs() const { return CImg<T>(*this).abs(); }
  
    //! Replace each image pixel by its cosinus.
    /**
       \see get_cos(), sin(), get_sin(), tan(), get_tan()
    **/
    CImg& cos() {
      cimg_map(*this,ptr,T) (*ptr)=(T)std::cos((double)(*ptr));
      return *this;
    }

    //! Return the image of the cosinus of the pixel values.
    /**
       \see cos(), sin(), get_sin(), tan(), get_tan()
    **/
    CImg get_cos() const { return CImg<T>(*this).cos(); }
 
    //! Replace each image pixel by its sinus.
    /**
       \see get_sin(), cos(), get_cos(), tan(), get_tan()
    **/
    CImg& sin() {
      cimg_map(*this,ptr,T) (*ptr)=(T)std::sin((double)(*ptr));
      return *this;
    }

    //! Return the image of the sinus of the pixel values.
    /**
       \see sin(), cos(), get_cos(), tan(), get_tan()
    **/
    CImg get_sin() const { return CImg<T>(*this).sin(); }
  
    //! Replace each image pixel by its tangent.
    /**
       \see get_tan(), cos(), get_cos(), sin(), get_sin()
    **/
    CImg& tan() {
      cimg_map(*this,ptr,T) (*ptr)=(T)std::tan((double)(*ptr));
      return *this;
    }

    //! Return the image of the tangent of the pixel values.
    /**
       \see tan(), cos(), get_cos(), sin(), get_sin()
    **/
    CImg get_tan() const { return CImg<T>(*this).tan(); }
  

    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name Usual image transformation
    //@{
    //------------------------------------------
    //------------------------------------------
    
    //! Fill an image by a value \p val.
    /**
       \param val = fill value
       \note All pixel values of the instance image will be initialized by \p val.
       \see operator=().
    **/
    CImg& fill(const T& val) {
      cimg_test(*this,"CImg<T>::fill");      
      if (val!=0 && sizeof(T)!=1) cimg_map(*this,ptr,T) *ptr=val; 
      else std::memset(data,(int)val,size()*sizeof(T));
      return *this;
    }

    //! Fill sequentially all pixel values with values \a val0 and \a val1 respectively.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
    **/
    CImg& fill(const T& val0,const T& val1) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-1; ) { *(ptr++)=val0; *(ptr++)=val1; }
      if (ptr!=ptr_end) *(ptr++)=val0;
      return *this;
    }
    
    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-2; ) { *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; }     
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1;
      return *this;
    }
    
    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,const T& val3) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-3; ) { *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; *(ptr++)=val3; }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      return *this;
    }

    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3 and \a val4.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
       \param val4 = fill value 5
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,const T& val3,const T& val4) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-4; ) { *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; *(ptr++)=val3; *(ptr++)=val4; }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      if (ptr!=ptr_end) *(ptr++)=val3;
      return *this;
    }
    
    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3 and \a val4 and \a val5
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
       \param val4 = fill value 5
       \param val5 = fill value 6
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,const T& val3,const T& val4,const T& val5) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size(); 
      for (ptr=data; ptr<ptr_end-5; ) {
    *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; *(ptr++)=val3; *(ptr++)=val4; *(ptr++)=val5; 
      }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      if (ptr!=ptr_end) *(ptr++)=val3; if (ptr!=ptr_end) *(ptr++)=val4;
      return *this;
    }

    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3 and \a val4 and \a val5
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
       \param val4 = fill value 5
       \param val5 = fill value 6
       \param val6 = fill value 7
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,const T& val3,
           const T& val4,const T& val5,const T& val6) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size(); 
      for (ptr=data; ptr<ptr_end-6; ) {
    *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; *(ptr++)=val3; *(ptr++)=val4; *(ptr++)=val5; *(ptr++)=val6;
      }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      if (ptr!=ptr_end) *(ptr++)=val3; if (ptr!=ptr_end) *(ptr++)=val4; if (ptr!=ptr_end) *(ptr++)=val5;
      return *this;
    }

    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3 and \a ... and \a val7.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
       \param val4 = fill value 5
       \param val5 = fill value 6
       \param val6 = fill value 7
       \param val7 = fill value 8
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,const T& val3,
           const T& val4,const T& val5,const T& val6,const T& val7) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-7; ) {
    *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; *(ptr++)=val3;
    *(ptr++)=val4; *(ptr++)=val5; *(ptr++)=val6; *(ptr++)=val7;
      }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      if (ptr!=ptr_end) *(ptr++)=val3; if (ptr!=ptr_end) *(ptr++)=val4; if (ptr!=ptr_end) *(ptr++)=val5;
      if (ptr!=ptr_end) *(ptr++)=val6;
      return *this;
    }

    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3 and \a ... and \a val8.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
       \param val4 = fill value 5
       \param val5 = fill value 6
       \param val6 = fill value 7
       \param val7 = fill value 8
       \param val8 = fill value 9
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,
           const T& val3,const T& val4,const T& val5,
           const T& val6,const T& val7,const T& val8) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-8; ) {
    *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; 
    *(ptr++)=val3; *(ptr++)=val4; *(ptr++)=val5; 
    *(ptr++)=val6; *(ptr++)=val7; *(ptr++)=val8;
      }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      if (ptr!=ptr_end) *(ptr++)=val3; if (ptr!=ptr_end) *(ptr++)=val4; if (ptr!=ptr_end) *(ptr++)=val5;
      if (ptr!=ptr_end) *(ptr++)=val6; if (ptr!=ptr_end) *(ptr++)=val7;
      return *this;
    }

    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3 and \a ... and \a val9.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
       \param val4 = fill value 5
       \param val5 = fill value 6
       \param val6 = fill value 7
       \param val7 = fill value 8
       \param val8 = fill value 9
       \param val9 = fill value 10
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,const T& val3,const T& val4,
           const T& val5,const T& val6,const T& val7,const T& val8,const T& val9) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-9; ) {
    *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; *(ptr++)=val3; *(ptr++)=val4;
    *(ptr++)=val5; *(ptr++)=val6; *(ptr++)=val7; *(ptr++)=val8; *(ptr++)=val9;
      }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      if (ptr!=ptr_end) *(ptr++)=val3; if (ptr!=ptr_end) *(ptr++)=val4; if (ptr!=ptr_end) *(ptr++)=val5;
      if (ptr!=ptr_end) *(ptr++)=val6; if (ptr!=ptr_end) *(ptr++)=val7; if (ptr!=ptr_end) *(ptr++)=val8;
      return *this;
    }

    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3 and \a ... and \a val11.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
       \param val4 = fill value 5
       \param val5 = fill value 6
       \param val6 = fill value 7
       \param val7 = fill value 8
       \param val8 = fill value 9
       \param val9 = fill value 10
       \param val10 = fill value 11
       \param val11 = fill value 12
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,const T& val3,
           const T& val4,const T& val5,const T& val6,const T& val7,
           const T& val8,const T& val9,const T& val10,const T& val11) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-11; ) {
    *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; *(ptr++)=val3; *(ptr++)=val4; *(ptr++)=val5; 
    *(ptr++)=val6; *(ptr++)=val7; *(ptr++)=val8; *(ptr++)=val9; *(ptr++)=val10; *(ptr++)=val11;
      }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      if (ptr!=ptr_end) *(ptr++)=val3; if (ptr!=ptr_end) *(ptr++)=val4; if (ptr!=ptr_end) *(ptr++)=val5;
      if (ptr!=ptr_end) *(ptr++)=val6; if (ptr!=ptr_end) *(ptr++)=val7; if (ptr!=ptr_end) *(ptr++)=val8;
      if (ptr!=ptr_end) *(ptr++)=val9; if (ptr!=ptr_end) *(ptr++)=val10;
      return *this;
    }
    //! Fill sequentially all pixel values with values \a val0 and \a val1 and \a val2 and \a val3 and \a ... and \a val15.
    /**
       \param val0 = fill value 1
       \param val1 = fill value 2
       \param val2 = fill value 3
       \param val3 = fill value 4
       \param val4 = fill value 5
       \param val5 = fill value 6
       \param val6 = fill value 7
       \param val7 = fill value 8
       \param val8 = fill value 9
       \param val9 = fill value 10
       \param val10 = fill value 11
       \param val11 = fill value 12
       \param val12 = fill value 13
       \param val13 = fill value 14
       \param val14 = fill value 15
       \param val15 = fill value 16
    **/
    CImg& fill(const T& val0,const T& val1,const T& val2,const T& val3,
           const T& val4,const T& val5,const T& val6,const T& val7,
               const T& val8,const T& val9,const T& val10,const T& val11,
           const T& val12,const T& val13,const T& val14,const T& val15) {
      cimg_test(*this,"CImg<T>::fill");
      T *ptr, *ptr_end = data+size();
      for (ptr=data; ptr<ptr_end-15; ) {
    *(ptr++)=val0; *(ptr++)=val1; *(ptr++)=val2; *(ptr++)=val3; *(ptr++)=val4; *(ptr++)=val5; 
    *(ptr++)=val6; *(ptr++)=val7; *(ptr++)=val8; *(ptr++)=val9; *(ptr++)=val10; *(ptr++)=val11;
    *(ptr++)=val12; *(ptr++)=val13; *(ptr++)=val14; *(ptr++)=val15;
      }
      if (ptr!=ptr_end) *(ptr++)=val0; if (ptr!=ptr_end) *(ptr++)=val1; if (ptr!=ptr_end) *(ptr++)=val2;
      if (ptr!=ptr_end) *(ptr++)=val3; if (ptr!=ptr_end) *(ptr++)=val4; if (ptr!=ptr_end) *(ptr++)=val5;
      if (ptr!=ptr_end) *(ptr++)=val6; if (ptr!=ptr_end) *(ptr++)=val7; if (ptr!=ptr_end) *(ptr++)=val8;
      if (ptr!=ptr_end) *(ptr++)=val9; if (ptr!=ptr_end) *(ptr++)=val10; if (ptr!=ptr_end) *(ptr++)=val11;
      if (ptr!=ptr_end) *(ptr++)=val12; if (ptr!=ptr_end) *(ptr++)=val13; if (ptr!=ptr_end) *(ptr++)=val14;
      return *this;
    }
  
    //! Linear normalization of the pixel values between \a a and \a b.
    /**
       \param a = minimum pixel value after normalization.
       \param b = maximum pixel value after normalization.
       \see get_normalize(), cut(), get_cut().
    **/
    CImg& normalize(const T& a,const T& b) {
      cimg_test(*this,"CImg<T>::normalize");
      const CImgStats st(*this,false);
      if (st.min==st.max) fill(0);
      else cimg_map(*this,ptr,T) *ptr=(T)((*ptr-st.min)/(st.max-st.min)*(b-a)+a);
      return *this;
    }

    //! Return the image of normalized values.
    /**
       \param a = minimum pixel value after normalization.
       \param b = maximum pixel value after normalization.
       \see normalize(), cut(), get_cut().
    **/
    CImg get_normalize(const T& a,const T& b) const { return CImg<T>(*this).normalize(a,b); }
  
    //! Cut pixel values between \a a and \a b.
    /**
       \param a = minimum pixel value after cut.
       \param b = maximum pixel value after cut.
       \see get_cut(), normalize(), get_normalize().
    **/
    CImg& cut(const T& a, const T& b) {
      cimg_test(*this,"CImg<T>::cut");
      cimg_map(*this,ptr,T) *ptr = (*ptr<a)?a:((*ptr>b)?b:*ptr);
      return *this;
    }

    //! Return the image of cutted values.
    /**
       \param a = minimum pixel value after cut.
       \param b = maximum pixel value after cut.
       \see cut(), normalize(), get_normalize().
    **/
    CImg get_cut(const T& a, const T& b) const { return CImg<T>(*this).cut(a,b); }

    //! Quantify pixel values into \n levels.
    /**
       \param n = number of quantification levels
       \see get_quantify().
    **/
    CImg& quantify(const unsigned int n=256) {
      cimg_test(*this,"CImg<T>::quantify");
      const CImgStats st(*this,false);
      const double range = st.max-st.min;
      cimg_map(*this,ptr,T) *ptr = (T)(st.min + range*(int)((*ptr-st.min)*(int)n/range)/n);
      return *this;
    }

    //! Return a quantified image, with \n levels.
    /**
       \param n = number of quantification levels
       \see quantify().
    **/
    CImg get_quantify(const unsigned int n=256) const { return CImg<T>(*this).quantify(n); }

    //! Threshold the image.
    /**
       \param thres = threshold
       \see get_threshold().
    **/    
    CImg& threshold(const T& thres) {
      cimg_test(*this,"CImg<T>::threshold");
      cimg_map(*this,ptr,T) *ptr = *ptr<=thres?(T)0:(T)1;
      return *this;
    }

    //! Return a thresholded image.
    /**
       \param thres = threshold.
       \see threshold().
    **/
    CImg get_threshold(const T& thres) const { return CImg<T>(*this).threshold(thres); }
  
    //! Return a rotated image.
    /**
       \param angle = rotation angle (in degrees).
       \param cond = rotation type. can be :
       - 0 = zero-value at borders
       - 1 = repeat image at borders
       - 2 = zero-value at borders and linear interpolation
       \note Returned image will probably have a different size than the instance image *this.
       \see rotate()
    **/
    CImg get_rotate(const float angle,const unsigned int cond=2) const {
      cimg_test(*this,"CImg<T>::get_rotate");
      CImg dest;
      const float nangle = cimg::mod(angle,360.0f), rad = (float)((nangle*cimg::PI)/180.0),
        ca=(float)std::cos(rad), sa=(float)std::sin(rad);
    
      if (cond!=1 && cimg::mod(nangle,90.0f)==0) { // optimized version for orthogonal angles
        const int iangle = (int)nangle/90;
        switch (iangle) {
        case 1: {
          dest = CImg<T>(height,width,depth,dim); 
          cimg_mapXYZV(dest,x,y,z,v) dest(x,y,z,v) = (*this)(y,height-1-x,z,v); 
        } break; 
        case 2: {
          dest = CImg<T>(width,height,depth,dim);
          cimg_mapXYZV(dest,x,y,z,v) dest(x,y,z,v) = (*this)(width-1-x,height-1-y,z,v); 
        } break;
        case 3: {
          dest = CImg<T>(height,width,depth,dim); 
          cimg_mapXYZV(dest,x,y,z,v) dest(x,y,z,v) = (*this)(width-1-y,x,z,v); 
        } break;
        default: 
          return *this;        
        }
      } else { // generic version
        const float 
          ux  = (float)(cimg::abs(width*ca)),  uy  = (float)(cimg::abs(width*sa)),
          vx  = (float)(cimg::abs(height*sa)), vy  = (float)(cimg::abs(height*ca)),
          w2  = 0.5f*width,           h2  = 0.5f*height,
          dw2 = 0.5f*(ux+vx),         dh2 = 0.5f*(uy+vy);
        dest = CImg<T>((int)(ux+vx), (int)(uy+vy),depth,dim);

        switch (cond) {
        case 0: { 
          cimg_mapXY(dest,x,y)
            cimg_mapZV(*this,z,v) 
            dest(x,y,z,v) = dirichlet_pix2d((int)(w2 + (x-dw2)*ca + (y-dh2)*sa),(int)(h2 - (x-dw2)*sa + (y-dh2)*ca),z,v);
        } break;
        case 1: {
          cimg_mapXY(dest,x,y)
            cimg_mapZV(*this,z,v) 
            dest(x,y,z,v) = (*this)(cimg::mod((int)(w2 + (x-dw2)*ca + (y-dh2)*sa),width),
                                    cimg::mod((int)(h2 - (x-dw2)*sa + (y-dh2)*ca),height),z,v);
        } break;
        default: {
          cimg_mapXY(dest,x,y) {
            const float X = w2 + (x-dw2)*ca + (y-dh2)*sa, Y = h2 - (x-dw2)*sa + (y-dh2)*ca;
            const int ix = (int)X, iy = (int)Y;
            if (ix<0 || ix>=dimx() || iy<0 || iy>=dimy()) cimg_mapZV(*this,z,v) dest(x,y,z,v) = 0;
            else cimg_mapZV(*this,z,v) dest(x,y,z,v) = (T)linear_pix2d(X,Y,z,v);
          }
        } break; 
        }
      }
      return dest;
    }
  
    //! Rotate the image 
    /**
       \param angle = rotation angle (in degrees).
       \param cond = rotation type. can be :
       - 0 = zero-value at borders
       - 1 = repeat image at borders
       - 2 = zero-value at borders and linear interpolation
       \see get_rotate()
    **/
    CImg& rotate(const float angle,const unsigned int cond=2) { return get_rotate(angle,cond).swap(*this); }
  
    //! Return a rotated image around the point (\c cx,\c cy).
    /**
       \param angle = rotation angle (in degrees).
       \param cx = X-coordinate of the rotation center.
       \param cy = Y-coordinate of the rotation center.
       \param zoom = zoom.
       \param cond = rotation type. can be :
       - 0 = zero-value at borders
       - 1 = repeat image at borders
       - 2 = zero-value at borders and linear interpolation
       \see rotate()
    **/
    CImg get_rotate(const float angle,const float cx,const float cy,const float zoom=1,const unsigned int cond=2) const {
      cimg_test(*this,"CImg<T>::get_rotate");
      CImg dest(*this,false);
      const float nangle = cimg::mod(angle,360.0f), rad = (float)((nangle*cimg::PI)/180.0),
        ca=(float)std::cos(rad)/zoom, sa=(float)std::sin(rad)/zoom;
    
      if (cond!=1 && zoom==1 && cimg::mod(nangle,90.0f)==0) { // optimized version for orthogonal angles
        const int iangle = (int)nangle/90;
        switch (iangle) {
        case 1: {
          dest.fill(0);
          const unsigned int
            xmin = cimg::max(0,(dimx()-dimy())/2), xmax = cimg::min(width,xmin+height),
            ymin = cimg::max(0,(dimy()-dimx())/2), ymax = cimg::min(height,ymin+width),
            xoff = xmin + cimg::min(0,(dimx()-dimy())/2),
            yoff = ymin + cimg::min(0,(dimy()-dimx())/2);
          cimg_mapZV(dest,z,v) for (unsigned int y=ymin; y<ymax; y++) for (unsigned int x=xmin; x<xmax; x++)
            dest(x,y,z,v) = (*this)(y-yoff,height-1-x+xoff,z,v);
        } break;
        case 2: {
          cimg_mapXYZV(dest,x,y,z,v) dest(x,y,z,v) = (*this)(width-1-x,height-1-y,z,v); 
        } break;
        case 3: {
          dest.fill(0);
          const unsigned int
            xmin = cimg::max(0,(dimx()-dimy())/2), xmax = cimg::min(width,xmin+height),
            ymin = cimg::max(0,(dimy()-dimx())/2), ymax = cimg::min(height,ymin+width),
            xoff = xmin + cimg::min(0,(dimx()-dimy())/2),
            yoff = ymin + cimg::min(0,(dimy()-dimx())/2);
          cimg_mapZV(dest,z,v) for (unsigned int y=ymin; y<ymax; y++) for (unsigned int x=xmin; x<xmax; x++)
            dest(x,y,z,v) = (*this)(width-1-y+yoff,x-xoff,z,v);
        } break;
        default: 
          return *this;        
        }
      } else 
        switch (cond) { // generic version
        case 0: { 
          cimg_mapXY(dest,x,y)
            cimg_mapZV(*this,z,v) 
            dest(x,y,z,v) = dirichlet_pix2d((int)(cx + (x-cx)*ca + (y-cy)*sa),(int)(cy - (x-cx)*sa + (y-cy)*ca),z,v);
        } break;
        case 1: {
          cimg_mapXY(dest,x,y)
            cimg_mapZV(*this,z,v) 
            dest(x,y,z,v) = (*this)(cimg::mod((int)(cx + (x-cx)*ca + (y-cy)*sa),width),
                                    cimg::mod((int)(cy - (x-cx)*sa + (y-cy)*ca),height),z,v);
        } break;
        default: {
          cimg_mapXY(dest,x,y) {
            const float X = cx + (x-cx)*ca + (y-cy)*sa, Y = cy - (x-cx)*sa + (y-cy)*ca;
            const int ix = (int)X, iy = (int)Y;
            if (ix<0 || ix>=dimx() || iy<0 || iy>=dimy()) cimg_mapZV(*this,z,v) dest(x,y,z,v) = 0;
            else cimg_mapZV(*this,z,v) dest(x,y,z,v) = (T)linear_pix2d(X,Y,z,v);
          }
        } break; 
        }
      return dest;
    }
  
    //! Rotate the image around the point (\c cx,\c cy).
    /**
       \param angle = rotation angle (in degrees).
       \param cx = X-coordinate of the rotation center.
       \param cy = Y-coordinate of the rotation center.
       \param zoom = zoom.
       \param cond = rotation type. can be :
       - 0 = zero-value at borders
       - 1 = repeat image at borders
       - 2 = zero-value at borders and linear interpolation
       \note Rotation does not change the image size. If you want to get an image with a new size, use get_rotate() instead.
       \see get_rotate()
    **/
    CImg& rotate(const float angle,const float cx,const float cy,const float zoom=1,const unsigned int cond=2) {
      return get_rotate(angle,cx,cy,zoom,cond).swap(*this);
    }
 
    //! Return a resized image.
    /**
       \param pdx = Number of columns (new size along the X-axis).
       \param pdy = Number of rows (new size along the Y-axis).
       \param pdz = Number of slices (new size along the Z-axis).
       \param pdv = Number of vector-channels (new size along the V-axis).
       \param interp = Resizing type :
       - 0 = no interpolation : additionnal space is filled with 0.
       - 1 = bloc interpolation (nearest point).
       - 2 = mosaic : image is repeated if necessary.
       - 3 = linear interpolation.
       - 4 = grid interpolation.
       - 5 = bi-cubic interpolation.
       \note If pd[x,y,z,v]<0, it corresponds to a percentage of the original size (the default value is -100).
    **/
    CImg get_resize(const int pdx=-100,const int pdy=-100,const int pdz=-100,const int pdv=-100,const unsigned int interp=1) const {
      cimg_test(*this,"CImg<T>::get_resize");
      const unsigned int 
    dx = pdx<0?-pdx*width/100:pdx,
    dy = pdy<0?-pdy*height/100:pdy,
    dz = pdz<0?-pdz*depth/100:pdz, 
    dv = pdv<0?-pdv*dim/100:pdv;
      CImg res(dx?dx:1,dy?dy:1,dz?dz:1,dv?dv:1);
      if (width==res.width && height==res.height && depth==res.depth && dim==res.dim) return *this;
      switch (interp) {
      case 0: { // Zero filling
    res.fill(0).draw_image(*this,0,0,0,0);
      } break;
      case 1: { // Bloc interpolation
    const float sx = (float)width/res.width, sy = (float)height/res.height, sz = (float)depth/res.depth, sk = (float)dim/res.dim;
    float cx,cy,cz,ck=0;
    cimg_mapV(res,k) { cz = 0; 
    cimg_mapZ(res,z) { cy = 0; 
    cimg_mapY(res,y) { cx = 0; 
    cimg_mapX(res,x) { res(x,y,z,k) = (*this)((unsigned int)cx,(unsigned int)cy,(unsigned int)cz,(unsigned int)ck); cx+=sx;
    } cy+=sy;
    } cz+=sz;
    } ck+=sk;
    }
      } break;
      case 2: { // Mosaic filling
    for (unsigned int k=0; k<res.dim; k+=dim)
      for (unsigned int z=0; z<res.depth; z+=depth)
        for (unsigned int y=0; y<res.height; y+=height)
          for (unsigned int x=0; x<res.width; x+=width) res.draw_image(*this,x,y,z,k);
      } break;
      case 3: { // Linear interpolation
    const float
      sx = res.width>1?(float)(width-1)/(res.width-1):0,
      sy = res.height>1?(float)(height-1)/(res.height-1):0,
      sz = res.depth>1?(float)(depth-1)/(res.depth-1):0,
      sk = res.dim>1?(float)(dim-1)/(res.dim-1):0;
    float cx,cy,cz,ck = 0;
    cimg_mapV(res,k) { cz = 0; 
    cimg_mapZ(res,z) { cy = 0;
    cimg_mapY(res,y) { cx = 0; 
    cimg_mapX(res,x) { res(x,y,z,k) = (T)linear_pix4d(cx,cy,cz,ck); cx+=sx;
    } cy+=sy;
    } cz+=sz;
    } ck+=sk;
    }
      } break;
      case 4: { // Grid filling
    const float sx = (float)width/res.width, sy = (float)height/res.height, sz = (float)depth/res.depth, sk = (float)dim/res.dim;
    res.fill(0);
    cimg_mapXYZV(*this,x,y,z,k) res((int)(x/sx),(int)(y/sy),(int)(z/sz),(int)(k/sk)) = (*this)(x,y,z,k);
      } break;
      case 5: { // Cubic interpolation
    const float
      sx = res.width>1?(float)(width-1)/(res.width-1):0,
      sy = res.height>1?(float)(height-1)/(res.height-1):0,
      sz = res.depth>1?(float)(depth-1)/(res.depth-1):0,
      sk = res.dim>1?(float)(dim-1)/(res.dim-1):0;
    float cx,cy,cz,ck = 0;
    cimg_mapV(res,k) { cz = 0;
    cimg_mapZ(res,z) { cy = 0;
    cimg_mapY(res,y) { cx = 0;
    cimg_mapX(res,x) { res(x,y,z,k) = (T)cubic_pix2d(cx,cy,(int)cz,(int)ck); cx+=sx;
    } cy+=sy;
    } cz+=sz;
    } ck+=sk;
    }
      } break;      
      }
      return res;
    }

    //! Return a resized image.
    /**
       \param src = Image giving the geometry of the resize.
       \param interp = Resizing type :
       - 0 = no interpolation : additionnal space is filled with 0.
       - 1 = bloc interpolation (nearest point).
       - 2 = mosaic : image is repeated if necessary.
       - 3 = linear interpolation.
       - 4 = grid interpolation.
       - 5 = bi-cubic interpolation.
       \note If pd[x,y,z,v]<0, it corresponds to a percentage of the original size (the default value is -100).
    **/
    template<typename t> CImg get_resize(const CImg<t>& src,const unsigned int interp=1) const {
      return get_resize(src.width,src.height,src.depth,src.dim,interp); 
    }  

    //! Return a resized image.
    /**
       \param disp = Display giving the geometry of the resize.
       \param interp = Resizing type :
       - 0 = no interpolation : additionnal space is filled with 0.
       - 1 = bloc interpolation (nearest point).
       - 2 = mosaic : image is repeated if necessary.
       - 3 = linear interpolation.
       - 4 = grid interpolation.
       - 5 = bi-cubic interpolation.
       \note If pd[x,y,z,v]<0, it corresponds to a percentage of the original size (the default value is -100).
    **/
    CImg get_resize(const CImgDisplay& disp,const unsigned int interp=1) const {
      return get_resize(disp.width,disp.height,depth,dim,interp);
    }

    //! Resize the image.
    /**
       \param pdx = Number of columns (new size along the X-axis).
       \param pdy = Number of rows (new size along the Y-axis).
       \param pdz = Number of slices (new size along the Z-axis).
       \param pdv = Number of vector-channels (new size along the V-axis).
       \param interp = Resizing type :
       - 0 = no interpolation : additionnal space is filled with 0.
       - 1 = bloc interpolation (nearest point).
       - 2 = mosaic : image is repeated if necessary.
       - 3 = linear interpolation.
       - 4 = grid interpolation.
       - 5 = bi-cubic interpolation.
       \note If pd[x,y,z,v]<0, it corresponds to a percentage of the original size (the default value is -100).       
    **/
    CImg& resize(const int pdx=-100,const int pdy=-100,const int pdz=-100,const int pdv=-100,const unsigned int interp=1) {
      const unsigned int
        dx = pdx<0?-pdx*width/100 :(pdx==0?1:pdx),
        dy = pdy<0?-pdy*height/100:(pdy==0?1:pdy),
        dz = pdz<0?-pdz*depth/100 :(pdz==0?1:pdz),
        dv = pdv<0?-pdv*dim/100   :(pdv==0?1:pdv);
      if (width==dx && height==dy && depth==dz && dim==dv) return *this;
      else return get_resize(dx,dy,dz,dv,interp).swap(*this);
    }

    //! Resize the image.
    /**
       \param src = Image giving the geometry of the resize.
       \param interp = Resizing type :
       - 0 = no interpolation : additionnal space is filled with 0.
       - 1 = bloc interpolation (nearest point).
       - 2 = mosaic : image is repeated if necessary.
       - 3 = linear interpolation.
       - 4 = grid interpolation.
       - 5 = bi-cubic interpolation.
       \note If pd[x,y,z,v]<0, it corresponds to a percentage of the original size (the default value is -100).
    **/
    template<typename t> CImg& resize(const CImg<t>& src,const unsigned int interp=1) { 
      return resize(src.width,src.height,src.depth,src.dim,interp); 
    }

    //! Resize the image
    /**
       \param disp = Display giving the geometry of the resize.
       \param interp = Resizing type :
       - 0 = no interpolation : additionnal space is filled with 0.
       - 1 = bloc interpolation (nearest point).
       - 2 = mosaic : image is repeated if necessary.
       - 3 = linear interpolation.
       - 4 = grid interpolation.
       - 5 = bi-cubic interpolation.
       \note If pd[x,y,z,v]<0, it corresponds to a percentage of the original size (the default value is -100).
    **/
    CImg& resize(const CImgDisplay& disp,const unsigned int interp=1) {
      return resize(disp.width,disp.height,depth,dim,interp);
    }

    //! Return an half-resized image, using a special filter.
    /**
       \see resize_halfXY(), resize(), get_resize().
    **/
    CImg get_resize_halfXY() const {
      cimg_test(*this,"CImg<T>::get_resize_halfXY");
      CImg<float> mask = CImg<float>::matrix(0.07842776544f, 0.1231940459f, 0.07842776544f,
                         0.1231940459f,  0.1935127547f, 0.1231940459f,
                         0.07842776544f, 0.1231940459f, 0.07842776544f);
      CImg_3x3(I,float);
      CImg dest(width/2,height/2,depth,dim);
      cimg_mapZV(*this,z,k) cimg_map3x3(*this,x,y,z,k,I) dest(x/2,y/2,z,k) = (T)cimg_conv3x3(I,mask);
      return dest;
    }

    //! Half-resize the image, using a special filter
    /**
       \see get_resize_halfXY(), resize(), get_resize().
    **/
    CImg& resize_halfXY() {    return get_resize_halfXY().swap(*this); }

    //! Return a square region of the image, as a new image
    /**
       \param x0 = X-coordinate of the upper-left crop rectangle corner.
       \param y0 = Y-coordinate of the upper-left crop rectangle corner.
       \param z0 = Z-coordinate of the upper-left crop rectangle corner.
       \param v0 = V-coordinate of the upper-left crop rectangle corner.
       \param x1 = X-coordinate of the lower-right crop rectangle corner.
       \param y1 = Y-coordinate of the lower-right crop rectangle corner.
       \param z1 = Z-coordinate of the lower-right crop rectangle corner.
       \param v1 = V-coordinate of the lower-right crop rectangle corner.
       \param border_condition = Dirichlet (false) or Neumann border conditions.
       \see crop()
    **/
    CImg get_crop(const unsigned int x0,const unsigned int y0,const unsigned int z0,const unsigned int v0,
          const unsigned int x1,const unsigned int y1,const unsigned int z1,const unsigned int v1,
          const bool border_condition = false) const {
      cimg_test(*this,"CImg<T>::get_crop");
      const unsigned int dx=x1-x0+1, dy=y1-y0+1, dz=z1-z0+1, dv=v1-v0+1;
      CImg dest(dx,dy,dz,dv);
      if (x0>=width || x1>=width || y0>=height || y1>=height || z0>=depth || z1>=depth ||
      v0>=dim || v1>=dim || x1<x0 || y1<y0 || z1<z0 || v1<v0)
    switch (border_condition) {
    case false: { cimg_mapXYZV(dest,x,y,z,v) dest(x,y,z,v) = dirichlet_pix4d(x0+x,y0+y,z0+z,v0+v,0); } break;
    default: { cimg_mapXYZV(dest,x,y,z,v) dest(x,y,z,v) = neumann_pix4d(x0+x,y0+y,z0+z,v0+v); } break;
    } else {
      T *psrc = ptr(x0,y0,z0,v0), *pdest = dest.ptr(0,0,0,0);
      if (dx!=width)
        for (unsigned int k=0; k<dv; k++) {
          for (unsigned int z=0; z<dz; z++) {
        for (unsigned int y=0; y<dy; y++) {
          std::memcpy(pdest,psrc,dx*sizeof(T));
          pdest+=dx;
          psrc+=width;
        }
        psrc+=width*(height-dy);
          }
          psrc+=width*height*(depth-dz);
        }
      else {
        if (dy!=height)         
          for (unsigned int k=0; k<dv; k++) {
        for (unsigned int z=0; z<dz; z++) {
          std::memcpy(pdest,psrc,dx*dy*sizeof(T));
          pdest+=dx*dy;
          psrc+=width*height;
        }
        psrc+=width*height*(depth-dz);
          }
        else {
          if (dz!=depth)
        for (unsigned int k=0; k<dv; k++) {
          std::memcpy(pdest,psrc,dx*dy*dz*sizeof(T));
          pdest+=dx*dy*dz;
          psrc+=width*height*depth;
        }
          else std::memcpy(pdest,psrc,dx*dy*dz*dv*sizeof(T));
        }
      }
    }
      return dest;
    }
    
    //! Return a square region of the image, as a new image
    /**
       \param x0 = X-coordinate of the upper-left crop rectangle corner.
       \param y0 = Y-coordinate of the upper-left crop rectangle corner.
       \param z0 = Z-coordinate of the upper-left crop rectangle corner.
       \param x1 = X-coordinate of the lower-right crop rectangle corner.
       \param y1 = Y-coordinate of the lower-right crop rectangle corner.
       \param z1 = Z-coordinate of the lower-right crop rectangle corner.
       \see crop()   
    **/
    CImg get_crop(const unsigned int x0,const unsigned int y0,const unsigned int z0,
          const unsigned int x1,const unsigned int y1,const unsigned int z1,
          const bool border_condition=false) const {
      return get_crop(x0,y0,z0,0,x1,y1,z1,dim-1,border_condition);
    }

    //! Return a square region of the image, as a new image
    /**
       \param x0 = X-coordinate of the upper-left crop rectangle corner.
       \param y0 = Y-coordinate of the upper-left crop rectangle corner.
       \param x1 = X-coordinate of the lower-right crop rectangle corner.
       \param y1 = Y-coordinate of the lower-right crop rectangle corner.
       \see crop()   
    **/
    CImg get_crop(const unsigned int x0,const unsigned int y0,
          const unsigned int x1,const unsigned int y1,
          const bool border_condition=false) const {
      return get_crop(x0,y0,0,0,x1,y1,depth-1,dim-1,border_condition);
    }

    //! Return a square region of the image, as a new image
    /**
       \param x0 = X-coordinate of the upper-left crop rectangle corner.
       \param x1 = X-coordinate of the lower-right crop rectangle corner.
       \see crop()   
    **/
    CImg get_crop(const unsigned int x0,const unsigned int x1,const bool border_condition=false) const {
      return get_crop(x0,0,0,0,x1,height-1,depth-1,dim-1,border_condition); 
    }

    //! Replace the image by a square region of the image
    /**
       \param x0 = X-coordinate of the upper-left crop rectangle corner.
       \param y0 = Y-coordinate of the upper-left crop rectangle corner.
       \param z0 = Z-coordinate of the upper-left crop rectangle corner.
       \param v0 = V-coordinate of the upper-left crop rectangle corner.
       \param x1 = X-coordinate of the lower-right crop rectangle corner.
       \param y1 = Y-coordinate of the lower-right crop rectangle corner.
       \param z1 = Z-coordinate of the lower-right crop rectangle corner.
       \param v1 = V-coordinate of the lower-right crop rectangle corner.
       \see get_crop()
    **/
    CImg& crop(const unsigned int x0,const unsigned int y0,const unsigned int z0,const unsigned int v0,
           const unsigned int x1,const unsigned int y1,const unsigned int z1,const unsigned int v1,
           const bool border_condition=false) {
      return get_crop(x0,y0,z0,v0,x1,y1,z1,v1,border_condition).swap(*this);
    }

    //! Replace the image by a square region of the image
    /**
       \param x0 = X-coordinate of the upper-left crop rectangle corner.
       \param y0 = Y-coordinate of the upper-left crop rectangle corner.
       \param z0 = Z-coordinate of the upper-left crop rectangle corner.
       \param x1 = X-coordinate of the lower-right crop rectangle corner.
       \param y1 = Y-coordinate of the lower-right crop rectangle corner.
       \param z1 = Z-coordinate of the lower-right crop rectangle corner.
       \see get_crop()
    **/
    CImg& crop(const unsigned int x0,const unsigned int y0,const unsigned int z0,
           const unsigned int x1,const unsigned int y1,const unsigned int z1,
           const bool border_condition=false) {
      return crop(x0,y0,z0,0,x1,y1,z1,dim-1,border_condition);
    }

    //! Replace the image by a square region of the image
    /**
       \param x0 = X-coordinate of the upper-left crop rectangle corner.
       \param y0 = Y-coordinate of the upper-left crop rectangle corner.
       \param x1 = X-coordinate of the lower-right crop rectangle corner.
       \param y1 = Y-coordinate of the lower-right crop rectangle corner.
       \see get_crop()
    **/
    CImg& crop(const unsigned int x0,const unsigned int y0,
           const unsigned int x1,const unsigned int y1,
           const bool border_condition=false) { 
      return crop(x0,y0,0,0,x1,y1,depth-1,dim-1,border_condition); 
    }

    //! Replace the image by a square region of the image
    /**
       \param x0 = X-coordinate of the upper-left crop rectangle corner.
       \param x1 = X-coordinate of the lower-right crop rectangle corner.
       \see get_crop()
    **/
    CImg& crop(const unsigned int x0,const unsigned int x1,const bool border_condition=false) { 
      return crop(x0,0,0,0,x1,height-1,depth-1,dim-1,border_condition);
    }

    //! Get the channel \a v of the current image, as a new image.
    /**
       \param v0 = vector-channel to return.
       \see channel(), get_slice(), slice(), get_plane(), plane().
    **/
    CImg get_channel(const unsigned int v0=0) const { return get_crop(0,0,0,v0,width-1,height-1,depth-1,v0); }

    //! Get the z-slice \a z of *this, as a new image.
    /**
       \param z0 = Z-slice to return.
       \see slice(), get_channel(), channel(), get_plane(), plane().
    **/
    CImg get_slice(const unsigned int z0=0) const { return get_crop(0,0,z0,0,width-1,height-1,z0,dim-1); }

    //! Get the z-slice \a z of the channel \a v of the current image, as a new image.
    /**
       \param z0 = Z-slice of the plane to return.
       \param v0 = V-channel of the plane to return.
       \see plane(), get_channel(), channel(), get_slice(), slice().
    **/
    CImg get_plane(const unsigned int z0=0,const unsigned int v0=0) const { return get_crop(0,0,z0,v0,width-1,height-1,z0,v0); }

    //! Return a reference to a set of points (x0->x1,y0,z0,v0) of the image. Use it carefully !
    CImgROI<T> ref_pointset(const unsigned int xmin,const unsigned int xmax,const unsigned int y0=0,const unsigned int z0=0,const unsigned int v0=0) const {
      cimg_test(*this,"CImg<T>::ref_pointset");
      if (xmax<xmin || xmax>=width || y0>=height || z0>=depth || v0>=dim)
        throw CImgArgumentException("CImg<%s>::ref_pointset() : Cannot return a reference (%u->%u,%u,%u,%u) from a (%u,%u,%u,%u) image",
                                    pixel_type(),xmin,xmax,y0,z0,v0,width,height,depth,dim);
      return CImgROI<T>(1+xmax-xmin,1,1,1,ptr(xmin,y0,z0,v0));
    }

    //! Return a reference to a set of lines (y0->y1,z0,v0) of the image. Use it carefully !
    CImgROI<T> ref_lineset(const unsigned int ymin,const unsigned int ymax,const unsigned int z0=0,const unsigned int v0=0) const {
      cimg_test(*this,"CImg<T>::ref_lineset");
      if (ymax<ymin || ymax>=height || z0>=depth || v0>=dim)
        throw CImgArgumentException("CImg<%s>::ref_lineset() : Cannot return a reference (0->%u,%u->%u,%u,%u) from a (%u,%u,%u,%u) image",
                                    pixel_type(),width-1,ymin,ymax,z0,v0,width,height,depth,dim);
      return CImgROI<T>(width,1+ymax-ymin,1,1,ptr(0,ymin,z0,v0));
    }
  
    //! Return a reference to a set of planes (z0->z1,v0) of the image. Use it carefully !
    CImgROI<T> ref_planeset(const unsigned int zmin,const unsigned int zmax,const unsigned int v0=0) const {
      cimg_test(*this,"CImg<T>::ref_planeset");
      if(zmax<zmin || zmax>=depth || v0>=dim)
        throw CImgArgumentException("CImg<%s>::ref_planeset() : Cannot return a reference (0->%u,0->%u,%u->%u,%u) from a (%u,%u,%u,%u) image",
                                    pixel_type(),width-1,height-1,zmin,zmax,v0,width,height,depth,dim);
      return CImgROI<T>(width,height,1+zmax-zmin,1,ptr(0,0,zmin,v0));
    }

    //! Return a reference to a set of channels (v0->v1) of the image. Use it carefully !
    CImgROI<T> ref_channelset(const unsigned int vmin,const unsigned int vmax) const {
      cimg_test(*this,"CImg<T>::ref_channelset");
      if (vmax<vmin || vmax>=dim)
        throw CImgArgumentException("CImg<%s>::ref_channelset() : Cannot return a reference (0->%u,0->%u,0->%u,%u->%u) from a (%u,%u,%u,%u) image",
                                    pixel_type(),width-1,height-1,depth-1,vmin,vmax,width,height,depth,dim);
      return CImgROI<T>(width,height,depth,1+vmax-vmin,ptr(0,0,0,vmin));
    }
  
    //! Return a reference to a line (y0,z0,v0) of the image. Use it carefully !
    CImgROI<T> ref_line(const unsigned int y0,const unsigned int z0=0,const unsigned int v0=0) const { return ref_pointset(0,width-1,y0,z0,v0); }

    //! Return a reference to a plane (z0,v0) of the image. Use it carefully !
    CImgROI<T> ref_plane(const unsigned int z0,const unsigned int v0=0) const { return ref_lineset(0,height-1,z0,v0); }

    //! Return a reference to a channel (v0) of the image. Use it carefully !
    CImgROI<T> ref_channel(const unsigned int v0) const { return ref_planeset(0,depth-1,v0); }

    //! Replace the image by one of its channel
    CImg& channel(const unsigned int v0=0) { return get_channel(v0).swap(*this); }

    //! Replace the image by one of its z-slice
    CImg& slice(const unsigned int z0=0) { return get_slice(z0).swap(*this); }

    //! Replace the image by one of its plane
    CImg& plane(const unsigned int z0=0, const unsigned int v0=0) { return get_plane(z0,v0).swap(*this); }
  
    //! Flip an image along the specified axis
    CImg& flip(const char axe='x') {
      cimg_test(*this,"CImg<T>::flip");
      T *pf,*pb,*buf=NULL;
      switch (cimg::uncase(axe)) {
      case 'x': {
    pf = ptr(); pb = ptr(width-1);
    for (unsigned int yzv=0; yzv<height*depth*dim; yzv++) { 
      for (unsigned int x=0; x<width/2; x++) { const T val = *pf; *(pf++)=*pb; *(pb--)=val; }
      pf+=width-width/2;
      pb+=width+width/2;
    }
      } break;
      case 'y': {
    buf = new T[width];
    pf = ptr(); pb = ptr(0,height-1);
    for (unsigned int zv=0; zv<depth*dim; zv++) {
      for (unsigned int y=0; y<height/2; y++) {
        std::memcpy(buf,pf,width*sizeof(T));
        std::memcpy(pf,pb,width*sizeof(T));
        std::memcpy(pb,buf,width*sizeof(T));
        pf+=width;
        pb-=width;
      }
      pf+=width*(height-height/2);
      pb+=width*(height+height/2);
    }
      } break;
      case 'z': {
    buf = new T[width*height];
    pf = ptr(); pb = ptr(0,0,depth-1);
    cimg_mapV(*this,v) {
      for (unsigned int z=0; z<depth/2; z++) {
        std::memcpy(buf,pf,width*height*sizeof(T));
        std::memcpy(pf,pb,width*height*sizeof(T));
        std::memcpy(pb,buf,width*height*sizeof(T));
        pf+=width*height;
        pb-=width*height;
      }
      pf+=width*height*(depth-depth/2);
      pb+=width*height*(depth+depth/2);
    }
      } break;
      case 'v': {
    buf = new T[width*height*depth];
    pf = ptr(); pb = ptr(0,0,0,dim-1);
    for (unsigned int v=0; v<dim/2; v++) {
      std::memcpy(buf,pf,width*height*depth*sizeof(T));
      std::memcpy(pf,pb,width*height*depth*sizeof(T));
      std::memcpy(pb,buf,width*height*depth*sizeof(T));
      pf+=width*height*depth;
      pb-=width*height*depth;
    }
      } break;
      default:
    throw CImgArgumentException("CImg<%s>::flip() : unknow axe '%c', must be 'x','y','z' or 'v'",pixel_type(),axe);
      }
      if (buf) delete[] buf;
      return *this;
    }

    //! Get a flipped version of the image, along the specified axis
    CImg get_flip(const char axe='x') { return CImg<T>(*this).flip(axe); }
    
    //! Scroll the image
    CImg& scroll(const int scrollx,const int scrolly=0,const int scrollz=0,const int scrollv=0,const int border_condition=0) {
      cimg_test(*this,"CImg<T>::scroll");
      
      if (scrollx) // Scroll along X-axis
    switch (border_condition) {
    case 0:
      if (cimg::abs(scrollx)>=(int)width) return fill(0);
      if (scrollx>0) cimg_mapYZV(*this,y,z,k) {
        std::memmove(ptr(0,y,z,k),ptr(scrollx,y,z,k),(width-scrollx)*sizeof(T));
        std::memset(ptr(width-scrollx,y,z,k),0,scrollx*sizeof(T));
      } else cimg_mapYZV(*this,y,z,k) {
        std::memmove(ptr(-scrollx,y,z,k),ptr(0,y,z,k),(width+scrollx)*sizeof(T));
        std::memset(ptr(0,y,z,k),0,-scrollx*sizeof(T));
      }
      break;   
    case 1:
      if (scrollx>0) {
        const int nscrollx = (scrollx>=(int)width)?width-1:scrollx;
        if (!nscrollx) return *this;
        cimg_mapYZV(*this,y,z,k) {
          std::memmove(ptr(0,y,z,k),ptr(nscrollx,y,z,k),(width-nscrollx)*sizeof(T));
          T *ptrd = ptr(width-1,y,z,k);
          const T &val = *ptrd;
          for (int l=0; l<nscrollx-1; l++) *(--ptrd) = val;
        }
      } else {
        const int nscrollx = (-scrollx>=(int)width)?width-1:-scrollx;
        if (!nscrollx) return *this;
        cimg_mapYZV(*this,y,z,k) {
          std::memmove(ptr(nscrollx,y,z,k),ptr(0,y,z,k),(width-nscrollx)*sizeof(T));
          T *ptrd = ptr(0,y,z,k);
          const T &val = *ptrd;
          for (int l=0; l<nscrollx-1; l++) *(++ptrd) = val;
        }
      }    
      break; 
    case 2: {
      const int ml = cimg::mod(scrollx,width), nscrollx = (ml<=(int)width/2)?ml:(ml-(int)width);
      if (!nscrollx) return *this;
      T* buf = new T[cimg::abs(nscrollx)];
      if (nscrollx>0) cimg_mapYZV(*this,y,z,k) {
        std::memcpy(buf,ptr(0,y,z,k),nscrollx*sizeof(T));
        std::memmove(ptr(0,y,z,k),ptr(nscrollx,y,z,k),(width-nscrollx)*sizeof(T));
        std::memcpy(ptr(width-nscrollx,y,z,k),buf,nscrollx*sizeof(T));
      } else cimg_mapYZV(*this,y,z,k) {
        std::memcpy(buf,ptr(width+nscrollx,y,z,k),-nscrollx*sizeof(T));
        std::memmove(ptr(-nscrollx,y,z,k),ptr(0,y,z,k),(width+nscrollx)*sizeof(T));
        std::memcpy(ptr(0,y,z,k),buf,-nscrollx*sizeof(T));
      }
      delete[] buf;
    } break;
    }

      if (scrolly) // Scroll along Y-axis
    switch (border_condition) {
    case 0:
      if (cimg::abs(scrolly)>=(int)height) return fill(0);
      if (scrolly>0) cimg_mapZV(*this,z,k) {
        std::memmove(ptr(0,0,z,k),ptr(0,scrolly,z,k),width*(height-scrolly)*sizeof(T));
        std::memset(ptr(0,height-scrolly,z,k),0,width*scrolly*sizeof(T));
      } else cimg_mapZV(*this,z,k) {
        std::memmove(ptr(0,-scrolly,z,k),ptr(0,0,z,k),width*(height+scrolly)*sizeof(T));
        std::memset(ptr(0,0,z,k),0,-scrolly*width*sizeof(T));
      }
      break;      
    case 1:
      if (scrolly>0) {
        const int nscrolly = (scrolly>=(int)height)?height-1:scrolly;
        if (!nscrolly) return *this;
        cimg_mapZV(*this,z,k) {
          std::memmove(ptr(0,0,z,k),ptr(0,nscrolly,z,k),width*(height-nscrolly)*sizeof(T));
          T *ptrd = ptr(0,height-nscrolly,z,k), *ptrs = ptr(0,height-1,z,k);
          for (int l=0; l<nscrolly-1; l++) { std::memcpy(ptrd,ptrs,width*sizeof(T)); ptrd+=width; }
        }
      } else {
        const int nscrolly = (-scrolly>=(int)height)?height-1:-scrolly;
        if (!nscrolly) return *this;
        cimg_mapZV(*this,z,k) {
          std::memmove(ptr(0,nscrolly,z,k),ptr(0,0,z,k),width*(height-nscrolly)*sizeof(T));
          T *ptrd = ptr(0,1,z,k), *ptrs = ptr(0,0,z,k);
          for (int l=0; l<nscrolly-1; l++) { std::memcpy(ptrd,ptrs,width*sizeof(T)); ptrd+=width; }
        }
      }    
      break;  
    case 2: {
      const int ml = cimg::mod(scrolly,height), nscrolly = (ml<=(int)height/2)?ml:(ml-(int)height);
      if (!nscrolly) return *this;
      T* buf = new T[width*cimg::abs(nscrolly)];
      if (nscrolly>0) cimg_mapZV(*this,z,k) {
        std::memcpy(buf,ptr(0,0,z,k),width*nscrolly*sizeof(T));
        std::memmove(ptr(0,0,z,k),ptr(0,nscrolly,z,k),width*(height-nscrolly)*sizeof(T));
        std::memcpy(ptr(0,height-nscrolly,z,k),buf,width*nscrolly*sizeof(T));
      } else cimg_mapZV(*this,z,k) {
        std::memcpy(buf,ptr(0,height+nscrolly,z,k),-nscrolly*width*sizeof(T));
        std::memmove(ptr(0,-nscrolly,z,k),ptr(0,0,z,k),width*(height+nscrolly)*sizeof(T));
        std::memcpy(ptr(0,0,z,k),buf,-nscrolly*width*sizeof(T));
      }
      delete[] buf;
    } break;    
    }

      if (scrollz) // Scroll along Z-axis
    switch (border_condition) {
    case 0:
      if (cimg::abs(scrollz)>=(int)depth) return fill(0);
      if (scrollz>0) cimg_mapV(*this,k) {
        std::memmove(ptr(0,0,0,k),ptr(0,0,scrollz,k),width*height*(depth-scrollz)*sizeof(T));
        std::memset(ptr(0,0,depth-scrollz,k),0,width*height*scrollz*sizeof(T));
      } else cimg_mapV(*this,k) {
        std::memmove(ptr(0,0,-scrollz,k),ptr(0,0,0,k),width*height*(depth+scrollz)*sizeof(T));
        std::memset(ptr(0,0,0,k),0,-scrollz*width*height*sizeof(T));
      }
      break;      
    case 1:
      if (scrollz>0) {
        const int nscrollz = (scrollz>=(int)depth)?depth-1:scrollz;
        if (!nscrollz) return *this;
        cimg_mapV(*this,k) {
          std::memmove(ptr(0,0,0,k),ptr(0,0,nscrollz,k),width*height*(depth-nscrollz)*sizeof(T));
          T *ptrd = ptr(0,0,depth-nscrollz,k), *ptrs = ptr(0,0,depth-1,k);
          for (int l=0; l<nscrollz-1; l++) { std::memcpy(ptrd,ptrs,width*height*sizeof(T)); ptrd+=width*height; }
        }
      } else {
        const int nscrollz = (-scrollz>=(int)depth)?depth-1:-scrollz;
        if (!nscrollz) return *this;
        cimg_mapV(*this,k) {
          std::memmove(ptr(0,0,nscrollz,k),ptr(0,0,0,k),width*height*(depth-nscrollz)*sizeof(T));
          T *ptrd = ptr(0,0,1,k), *ptrs = ptr(0,0,0,k);
          for (int l=0; l<nscrollz-1; l++) { std::memcpy(ptrd,ptrs,width*height*sizeof(T)); ptrd+=width*height; }
        }
      }    
      break;      
    case 2: {
      const int ml = cimg::mod(scrollz,depth), nscrollz = (ml<=(int)depth/2)?ml:(ml-(int)depth);
      if (!nscrollz) return *this;
      T* buf = new T[width*height*cimg::abs(nscrollz)];
      if (nscrollz>0) cimg_mapV(*this,k) {
        std::memcpy(buf,ptr(0,0,0,k),width*height*nscrollz*sizeof(T));
        std::memmove(ptr(0,0,0,k),ptr(0,0,nscrollz,k),width*height*(depth-nscrollz)*sizeof(T));
        std::memcpy(ptr(0,0,depth-nscrollz,k),buf,width*height*nscrollz*sizeof(T));
      } else cimg_mapV(*this,k) {
        std::memcpy(buf,ptr(0,0,depth+nscrollz,k),-nscrollz*width*height*sizeof(T));
        std::memmove(ptr(0,0,-nscrollz,k),ptr(0,0,0,k),width*height*(depth+nscrollz)*sizeof(T));
        std::memcpy(ptr(0,0,0,k),buf,-nscrollz*width*height*sizeof(T));
      }
      delete[] buf;
    } break;    
    }

      if (scrollv) // Scroll along V-axis
    switch (border_condition) {
    case 0:
      if (cimg::abs(scrollv)>=(int)dim) return fill(0);
      if (scrollv>0) {
        std::memmove(data,ptr(0,0,0,scrollv),width*height*depth*(dim-scrollv)*sizeof(T));
        std::memset(ptr(0,0,0,dim-scrollv),0,width*height*depth*scrollv*sizeof(T));
      } else cimg_mapV(*this,k) {
        std::memmove(ptr(0,0,0,-scrollv),data,width*height*depth*(dim+scrollv)*sizeof(T));
        std::memset(data,0,-scrollv*width*height*depth*sizeof(T));
      }
      break;      
    case 1:
      if (scrollv>0) {
        const int nscrollv = (scrollv>=(int)dim)?dim-1:scrollv;
        if (!nscrollv) return *this;
        std::memmove(data,ptr(0,0,0,nscrollv),width*height*depth*(dim-nscrollv)*sizeof(T));
        T *ptrd = ptr(0,0,0,dim-nscrollv), *ptrs = ptr(0,0,0,dim-1);
        for (int l=0; l<nscrollv-1; l++) { std::memcpy(ptrd,ptrs,width*height*depth*sizeof(T)); ptrd+=width*height*depth; }      
      } else {
        const int nscrollv = (-scrollv>=(int)dim)?dim-1:-scrollv;
        if (!nscrollv) return *this;
        std::memmove(ptr(0,0,0,nscrollv),data,width*height*depth*(dim-nscrollv)*sizeof(T));
        T *ptrd = ptr(0,0,0,1);
        for (int l=0; l<nscrollv-1; l++) { std::memcpy(ptrd,data,width*height*depth*sizeof(T)); ptrd+=width*height*depth; }      
      }    
      break;      
    case 2: {
      const int ml = cimg::mod(scrollv,dim), nscrollv = (ml<=(int)dim/2)?ml:(ml-(int)dim);
      if (!nscrollv) return *this;
      T* buf = new T[width*height*depth*cimg::abs(nscrollv)];
      if (nscrollv>0) {
        std::memcpy(buf,data,width*height*depth*nscrollv*sizeof(T));
        std::memmove(data,ptr(0,0,0,nscrollv),width*height*depth*(dim-nscrollv)*sizeof(T));
        std::memcpy(ptr(0,0,0,dim-nscrollv),buf,width*height*depth*nscrollv*sizeof(T));
      } else {
        std::memcpy(buf,ptr(0,0,0,dim+nscrollv),-nscrollv*width*height*depth*sizeof(T));
        std::memmove(ptr(0,0,0,-nscrollv),data,width*height*depth*(dim+nscrollv)*sizeof(T));
        std::memcpy(data,buf,-nscrollv*width*height*depth*sizeof(T));
      }
      delete[] buf;
    } break;    
    }

      return *this;
    }

    //! Return a scrolled image
    CImg get_scroll(const int scrollx,const int scrolly=0,const int scrollz=0,const int scrollv=0,
            const int border_condition=0) const {
      return CImg<T>(*this).scroll(scrollx,scrolly,scrollz,scrollv,border_condition);
    }
    
    //! Return a 2D representation of a 3D image, with three slices.
    CImg get_3dplanes(const unsigned int px0,const unsigned int py0,const unsigned int pz0) const {
      cimg_test(*this,"CImg<T>::get_3dplanes");
      const unsigned int
        x0=(px0>=width)?width-1:px0,
        y0=(py0>=height)?height-1:py0,
        z0=(pz0>=depth)?depth-1:pz0;
      CImg res(width+depth,height+depth,1,dim);
      res.fill((*this)[0]);
      { cimg_mapXYV(*this,x,y,k) res(x,y,0,k)        = (*this)(x,y,z0,k); }
      { cimg_mapYZV(*this,y,z,k) res(width+z,y,0,k)  = (*this)(x0,y,z,k); }
      { cimg_mapXZV(*this,x,z,k) res(x,height+z,0,k) = (*this)(x,y0,z,k); }
      return res;
    }

    //! Return the histogram of the image
    CImg<float> get_histogram(const unsigned int nblevels=256,const T val_min=(T)0,const T val_max=(T)0) const {
      cimg_test(*this,"CImg<T>::get_histogram");
      if (nblevels<1) {
        throw CImgArgumentException("CImg<%s>::get_histogram() : Can't compute an histogram with %u levels",
                                    pixel_type(),nblevels);
      }
      T vmin=val_min,vmax=val_max;
      CImg<float> res(nblevels,1,1,1,0);
      if (vmin==vmax && vmin==0) { CImgStats st(*this,false); vmin = (T)st.min; vmax = (T)st.max; }
      cimg_map(*this,ptr,T) { const int pos = (int)((*ptr-vmin)*(nblevels-1)/(vmax-vmin)); if (pos>=0 && pos<(int)nblevels) res[pos]++; }
      return res;
    }

    //! Equalize the image histogram
    CImg& equalize_histogram(const unsigned int nblevels=256) {
      cimg_test(*this,"CImg<T>::equalize_histogram");
      CImgStats st(*this,false);
      CImg<float> hist = get_histogram(nblevels,(T)st.min,(T)st.max);
      float cumul=0;
      cimg_mapX(hist,pos) { cumul+=hist[pos]; hist[pos]=cumul; }
      cimg_map(*this,ptr,T) {
        unsigned int pos = (unsigned int)((*ptr-st.min)*nblevels/(1+st.max-st.min));
        *ptr = (T)(st.min + (st.max-st.min)*hist[pos]/size());
      }
      return *this;
    }
    //! Return the histogram-equalized version of the current image.
    CImg get_equalize_histogram(const unsigned int nblevels=256) const { return CImg<T>(*this).equalize_histogram(nblevels); }

    //! Return the image of the vector norms of the current image.
    CImg<float> get_norm_pointwise(int ntype=2) const {
      cimg_test(*this,"CImg<T>::get_norm_pointwise");
      CImg<float> res(width,height,depth);
      switch(ntype) {
      case -1:                // Linf norm
        {
          cimg_mapXYZ(*this,x,y,z) {
            float n=0; cimg_mapV(*this,v) {
          const double tmp = cimg::abs((double)(*this)(x,y,z,v));
          if (tmp>n) n=(float)tmp; res(x,y,z) = n;
        }
          }
        } break;
      case 1:               // L1 norm
        {
          cimg_mapXYZ(*this,x,y,z) {
            float n=0; cimg_mapV(*this,v) n+=cimg::abs((float)((*this)(x,y,z,v))); res(x,y,z) = n;
          }
        } break;
      default:              // L2 norm
        {
          cimg_mapXYZ(*this,x,y,z) {
            float n=0; cimg_mapV(*this,v) n+=(float)((*this)(x,y,z,v)*(*this)(x,y,z,v)); res(x,y,z) = (float)std::sqrt((double)n);
          }
        } break;
      }
      return res;
    }

    //! Replace each pixel value with its vector norm
    CImg& norm_pointwise() { return CImg<T>(get_norm_pointwise()).swap(*this); }

    //! Return an image of the normalized vectors
    CImg get_orientation_pointwise() const {
      cimg_test(*this,"CImg<T>::get_orientation_pointwise");
      CImg dest(width,height,depth,dim);
      cimg_mapXYZ(dest,x,y,z) {
        float n = 0;
        cimg_mapV(*this,v) n+=(float)((*this)(x,y,z,v)*(*this)(x,y,z,v));
        n = (float)std::sqrt((double)n);
        if (n>0) cimg_mapV(dest,v) dest(x,y,z,v)=(T)((*this)(x,y,z,v)/n); else cimg_mapV(dest,v) dest(x,y,z,v)=0;
      }
      return dest;
    }

    //! Replace each pixel value with its normalized vector component
    CImg& orientation_pointwise() { return get_orientation_pointwise().swap(*this); }

    //! Split current image into a list.
    /**
       - Splitting process is done along the specified dimension \c axe.
    **/
    CImgl<T> get_split(const char axe,const unsigned int nb=0) const {
      cimg_test(*this,"CImg<T>::get_split");
      CImgl<T> res;
      switch (cimg::uncase(axe)) {
      case 'x': {
    res = CImgl<T>(nb?nb:width);
    cimgl_map(res,l) res[l] = get_crop(l*width/res.size,0,0,0,(l+1)*width/res.size-1,height-1,depth-1,dim-1);
    } break;
      case 'y': {
    res = CImgl<T>(nb?nb:height);
    cimgl_map(res,l) res[l] = get_crop(0,l*height/res.size,0,0,width-1,(l+1)*height/res.size-1,depth-1,dim-1);
    } break;
      case 'z': {
    res = CImgl<T>(nb?nb:depth);
    cimgl_map(res,l) res[l] = get_crop(0,0,l*depth/res.size,0,width-1,height-1,(l+1)*depth/res.size-1,dim-1);
    } break;
      case 'v': {
    res = CImgl<T>(nb?nb:dim);
    cimgl_map(res,l) res[l] = get_crop(0,0,0,l*dim/res.size,width-1,height-1,depth-1,(l+1)*dim/res.size-1);
    } break;
      default:
    throw CImgArgumentException("CImg<%s>::get_split() : Unknow axe '%c', must be 'x','y','z' or 'v'",pixel_type(),axe);
    break;
      }
      return res;
    }

    //! Return a list of images, corresponding to the XY-gradients of an image.
    /**
       \param scheme = Numerical scheme used for the gradient computation :
       - -1 = Backward finite differences
       - 0 = Centered finite differences
       - 1 = Forward finite differences
       - 2 = Using Sobel masks
       - 3 = Using rotation invariant masks
       - 4 = Using Deriche recusrsive filter.
       \param alpha = Blur applied to the gradient.
    **/
    CImgl<T> get_gradientXY(const int scheme=0,const float alpha=0) const {
      cimg_test(*this,"CImg<T>::get_gradientXY");
      if (alpha<0) throw CImgArgumentException("CImg<%s>::get_gradientXYZ() : Given blur parameter %g is negative",pixel_type(),alpha);
      CImgl<T> res(2,width,height,depth,dim);
      CImg_3x3(I,T);
      switch(scheme) {
      case -1: { // backward finite differences
        cimg_mapZV(*this,z,k) cimg_map3x3(*this,x,y,z,k,I) { res[0](x,y,z,k) = Icc-Ipc; res[1](x,y,z,k) = Icc-Icp; } 
      } break;
      case 1: { // forward finite differences
        cimg_mapZV(*this,z,k) cimg_map2x2(*this,x,y,z,k,I) { res[0](x,y,0,k) = Inc-Icc; res[1](x,y,z,k) = Icn-Icc; }
      } break;
      case 2: { // using Sobel mask
        const float a = 1, b = 2;
        cimg_mapZV(*this,z,k) cimg_map3x3(*this,x,y,z,k,I) {
          res[0](x,y,z,k) = (T)(-a*Ipp-b*Ipc-a*Ipn+a*Inp+b*Inc+a*Inn);
          res[1](x,y,z,k) = (T)(-a*Ipp-b*Icp-a*Inp+a*Ipn+b*Icn+a*Inn);
        }
      } break;
      case 3: { // using rotation invariant mask
        const float a = (float)(0.25*(2-std::sqrt(2.0))), b = (float)(0.5f*(std::sqrt(2.0)-1));
        cimg_mapZV(*this,z,k) cimg_map3x3(*this,x,y,z,k,I) {
          res[0](x,y,z,k) = (T)(-a*Ipp-b*Ipc-a*Ipn+a*Inp+b*Inc+a*Inn);
          res[1](x,y,z,k) = (T)(-a*Ipp-b*Icp-a*Inp+a*Ipn+b*Icn+a*Inn);
        }
      } break;
      case 4: { // using Deriche filter with low standart variation
    res[0] = get_deriche(alpha,1,'x');
    res[1] = get_deriche(alpha,1,'y');
      } break;
      default: { // central finite differences
        cimg_mapZV(*this,z,k) cimg_map3x3(*this,x,y,z,k,I) { 
          res[0](x,y,z,k) = (T)(0.5*(Inc-Ipc));
          res[1](x,y,z,k) = (T)(0.5*(Icn-Icp)); 
        } 
      } break;
      }
      if (scheme!=4 && alpha>0) cimgl_map(res,l) res[l].blur(alpha);
      return res;
    }

    //! Return a list of images, corresponding to the XYZ-gradients of an image.
    /**
       \see get_gradientXY().
    **/
    CImgl<T> get_gradientXYZ(const int scheme=0,const float alpha=0) const {
      cimg_test(*this,"CImg<T>::get_gradientXYZ");
      if (alpha<0) throw CImgArgumentException("CImg<%s>::get_gradientXYZ() : Given blur parameter %g is negative",pixel_type(),alpha);
      CImgl<T> res(3,width,height,depth,dim);
      CImg_3x3x3(I,T);
      switch(scheme) {
      case -1: { // backward finite differences
        cimg_mapV(*this,k) cimg_map3x3x3(*this,x,y,z,k,I) { 
          res[0](x,y,z,k) = Iccc-Ipcc;
          res[1](x,y,z,k) = Iccc-Icpc;
          res[2](x,y,z,k) = Iccc-Iccp; 
        }
      } break;
      case 1: { // forward finite differences
        cimg_mapV(*this,k) cimg_map3x3x3(*this,x,y,z,k,I) {
          res[0](x,y,z,k) = Incc-Iccc; 
          res[1](x,y,z,k) = Icnc-Iccc;
          res[2](x,y,z,k) = Iccn-Iccc; 
        } 
      } break;
      case 4: { // using Deriche filter with low standart variation
    res[0] = get_deriche(alpha,1,'x');
    res[1] = get_deriche(alpha,1,'y');
    res[2] = get_deriche(alpha,1,'z');
      } break;
      default: { // central finite differences
        cimg_mapV(*this,k) cimg_map3x3x3(*this,x,y,z,k,I) {
          res[0](x,y,z,k) = (T)(0.5*(Incc-Ipcc));
          res[1](x,y,z,k) = (T)(0.5*(Icnc-Icpc)); 
          res[2](x,y,z,k) = (T)(0.5*(Iccn-Iccp)); 
        } 
      } break;
      }
      if (scheme!=4 && alpha>0) cimgl_map(res,l) res[l].blur(alpha);
      return res;
    }

    //@}
    //--------------------------------------
    //--------------------------------------
    //
    //! \name Color conversion functions
    //@{
    //--------------------------------------
    //--------------------------------------
  
    //! Convert color pixels from (R,G,B) to (X,Y,Z)_709.    
    CImg& RGBtoXYZ() {
      cimg_test(*this,"CImg<T>::RGBtoXYZ");
      if (dim!=3) throw CImgInstanceException("CImg<%s>::RGBtoXYZ() : Input image dimension is dim=%u, "
                          "should be a (R,G,B) image (dim=3)",pixel_type(),dim);
      cimg_mapXYZ(*this,x,y,z) {
        const T R = (*this)(x,y,z,0), G = (*this)(x,y,z,1), B = (*this)(x,y,z,2);
        (*this)(x,y,z,0) = (T)(0.412453*R + 0.357580*G + 0.180423*B);
        (*this)(x,y,z,1) = (T)(0.212671*R + 0.715160*G + 0.072169*B);
        (*this)(x,y,z,2) = (T)(0.019334*R + 0.119193*G + 0.950227*B);
      }
      return *this;
    }

    //! Convert (X,Y,Z)_709 pixels of a color image into the (R,G,B) color space.
    CImg& XYZtoRGB() {
      cimg_test(*this,"CImg<T>::XYZtoRGB");
      if (dim!=3) throw CImgInstanceException("CImg<%s>::XYZtoRGB() : Input image dimension is dim=%u, "
                          "should be a (X,Y,Z) image (dim=3)",pixel_type(),dim);
      cimg_mapXYZ(*this,x,y,z) {
        const T X = (*this)(x,y,z,0), Y = (*this)(x,y,z,1), Z = (*this)(x,y,z,2);
        (*this)(x,y,z,0) = (T)(3.240479*X  - 1.537150*Y - 0.498535*Z);
        (*this)(x,y,z,1) = (T)(-0.969256*X + 1.875992*Y + 0.041556*Z);
        (*this)(x,y,z,2) = (T)(0.055648*X  - 0.204043*Y + 1.057311*Z);
      }
      return *this;
    }

    //! Convert (X,Y,Z)_709 pixels of a color image into the (L*,a*,b*) color space.
#define cimg_Labf(x)  ((x)>=0.008856?(std::pow(x,1/3.0)):(7.787*(x)+16.0/116.0))
#define cimg_Labfi(x) ((x)>=0.206893?((x)*(x)*(x)):(((x)-16.0/116.0)/7.787))

    CImg& XYZtoLab() {
      cimg_test(*this,"CImg<T>::XYZtoLab");
      if (dim!=3) throw CImgInstanceException("CImg<%s>::XYZtoLab() : Input image dimension is dim=%u, "
                          "should be a (X,Y,Z) image (dim=3)",pixel_type(),dim);
      const double
    Xn = 0.412453 + 0.357580 + 0.180423,
    Yn = 0.212671 + 0.715160 + 0.072169,
    Zn = 0.019334 + 0.119193 + 0.950227;
      cimg_mapXYZ(*this,x,y,z) {
        const T X = (*this)(x,y,z,0), Y = (*this)(x,y,z,1), Z = (*this)(x,y,z,2);
    const double
      XXn = X/Xn, YYn = Y/Yn, ZZn = Z/Zn,
      fX = cimg_Labf(XXn), fY = cimg_Labf(YYn), fZ = cimg_Labf(ZZn);
        (*this)(x,y,z,0) = (T)(116*fY-16);
        (*this)(x,y,z,1) = (T)(500*(fX-fY));
        (*this)(x,y,z,2) = (T)(200*(fY-fZ));
      }
      return *this;
    }

    //! Convert (L,a,b) pixels of a color image into the (X,Y,Z) color space.
    CImg& LabtoXYZ() {
      cimg_test(*this,"CImg<T>::LabtoXYZ");
      if (dim!=3) throw CImgInstanceException("CImg<%s>::LabtoXYZ() : Input image dimension is dim=%u, "
                          "should be a (X,Y,Z) image (dim=3)",pixel_type(),dim);
      const double
    Xn = 0.412453 + 0.357580 + 0.180423,
    Yn = 0.212671 + 0.715160 + 0.072169,
    Zn = 0.019334 + 0.119193 + 0.950227;
      cimg_mapXYZ(*this,x,y,z) {
        const T L = (*this)(x,y,z,0), a = (*this)(x,y,z,1), b = (*this)(x,y,z,2);
    const double
      cY = (L+16)/116.0,
      Y = Yn*cimg_Labfi(cY),
      pY = std::pow(Y/Yn,1.0/3),
      cX = a/500+pY,
      X = Xn*cX*cX*cX,
      cZ = pY-b/200,
      Z = Zn*cZ*cZ*cZ;
        (*this)(x,y,z,0) = (T)(X);
        (*this)(x,y,z,1) = (T)(Y);
        (*this)(x,y,z,2) = (T)(Z);
      }
      return *this;
    }

    //! Convert (X,Y,Z)_709 pixels of a color image into the (x,y,Y) color space.
    CImg& XYZtoxyY() {
      cimg_test(*this,"CImg<T>::XYZtoxyY()");
      if (dim!=3) throw CImgInstanceException("CImg<%s>::XYZtoxyY() : Input image dimension is dim=%u, "
                          "should be a (X,Y,Z) image (dim=3)",pixel_type(),dim);
      cimg_mapXYZ(*this,x,y,z) {
    const T X = (*this)(x,y,z,0), Y = (*this)(x,y,z,1), Z = (*this)(x,y,z,2), sum = (X+Y+Z), nsum = sum>0?sum:1;
    (*this)(x,y,z,0) = X/nsum;
    (*this)(x,y,z,1) = Y/nsum;
    (*this)(x,y,z,2) = Y;
      }
      return *this;
    }

    //! Convert (x,y,Y) pixels of a color image into the (X,Y,Z)_709 color space.
    CImg& xyYtoXYZ() {
      cimg_test(*this,"CImg<T>::xyYtoXYZ()");
      if (dim!=3) throw CImgInstanceException("CImg<%s>::xyYtoXYZ() : Input image dimension is dim=%u, "
                          "should be a (x,y,Y) image (dim=3)",pixel_type(),dim);
      cimg_mapXYZ(*this,x,y,z) {
    const T px = (*this)(x,y,z,0), py = (*this)(x,y,z,1), Y = (*this)(x,y,z,2), ny = py>0?py:1;
    (*this)(x,y,z,0) = (T)(px*Y/ny);
    (*this)(x,y,z,1) = Y;
    (*this)(x,y,z,2) = (T)((1-px-py)*Y/ny);
      }
      return *this;
    }
    
    // Other conversions functions
    CImg& RGBtoLab() { return RGBtoXYZ().XYZtoLab(); }
    CImg& LabtoRGB() { return LabtoXYZ().XYZtoRGB(); }
    CImg& RGBtoxyY() { return RGBtoXYZ().XYZtoxyY(); }
    CImg& xyYtoRGB() { return xyYtoXYZ().XYZtoRGB(); }

    // equivalent with get_*()
    CImg get_RGBtoXYZ() const { return CImg<T>(*this).RGBtoXYZ(); }
    CImg get_XYZtoRGB() const { return CImg<T>(*this).XYZtoRGB(); }
    CImg get_XYZtoLab() const { return CImg<T>(*this).XYZtoLab(); }
    CImg get_LabtoXYZ() const { return CImg<T>(*this).LabtoXYZ(); }
    CImg get_XYZtoxyY() const { return CImg<T>(*this).XYZtoxyY(); }
    CImg get_xyYtoXYZ() const { return CImg<T>(*this).xyYtoXYZ(); }
    CImg get_RGBtoLab() const { return CImg<T>(*this).RGBtoLab(); }
    CImg get_LabtoRGB() const { return CImg<T>(*this).LabtoRGB(); }
    CImg get_RGBtoxyY() const { return CImg<T>(*this).RGBtoxyY(); }
    CImg get_xyYtoRGB() const { return CImg<T>(*this).xyYtoRGB(); }

    //@}
    //--------------------------------------
    //--------------------------------------
    //
    //! \name Drawing functions
    //@{
    //--------------------------------------
    //--------------------------------------

    // Should be used only by member functions. Not an user-friendly function.
    // Pre-requisites : x0<x1, y-coordinate is valid, col is valid.
    CImg& draw_scanline(const int x0,const int x1,const int y,const T *const color,const float opacity=1,
               const bool init=false) {
      static float nopacity=0, copacity=0;
      static unsigned int whz=0;
      static const T* col = NULL;
      if (init) {
    cimg_test(*this,"CImg<T>::draw_scanline");
    if (!color) throw CImgArgumentException("CImg<%s>::draw_scanline() : specified color is (null)",pixel_type());
    nopacity = cimg::abs(opacity);
    copacity = 1-cimg::max(opacity,0.0f); 
    whz = width*height*depth;
    col = color;
      } else {
    const int nx0 = cimg::max(x0,0), nx1 = cimg::min(x1,(int)width-1), dx = nx1-nx0;
    T *ptrd = ptr(nx0,y,0,0);
    if (dx>=0) {
      if (opacity>=1) {
        int off = whz-dx-1;
        if (sizeof(T)!=1) cimg_mapV(*this,k) {
          const T& val = *(col++);
          for (int x=dx; x>=0; x--) *(ptrd++)=val;
          ptrd+=off;
        } else cimg_mapV(*this,k) { std::memset(ptrd,(int)*(col++),dx+1); ptrd+=whz; }
        col-=dim;
      } else {
        int off = whz-dx-1;
        cimg_mapV(*this,k) {
          const T& val = *(col++);
          for (int x=dx; x>=0; x--) { *ptrd = (T)(val*nopacity + *ptrd*copacity); ptrd++; }
          ptrd+=off;
        }
        col-=dim;
      }
    }
      }
      return *this;
    }
    
    CImg& draw_scanline(const T *const color,const float opacity=1) { return draw_scanline(0,0,0,color,opacity,true); }

    //! Draw a colored point in the instance image, at coordinates (\c x0,\c y0,\c z0).
    /**
       \param x0 = X-coordinate of the vector-valued pixel to plot.
       \param y0 = Y-coordinate of the vector-valued pixel to plot.
       \param z0 = Z-coordinate of the vector-valued pixel to plot.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_point(const int x0,const int y0,const int z0,
                     const T *const color,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_point");
      if (!color) throw CImgArgumentException("CImg<%s>::draw_point() : specified color is (null)",pixel_type());
      if (x0>=0 && y0>=0 && z0>=0 && x0<dimx() && y0<dimy() && z0<dimz()) {
        const T *col=color;
        const unsigned int whz = width*height*depth;
        const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
        T *ptrd = ptr(x0,y0,z0,0);
        if (opacity>=1) cimg_mapV(*this,k) { *ptrd = *(col++); ptrd+=whz; }
        else cimg_mapV(*this,k) { *ptrd=(T)(*(col++)*nopacity + *ptrd*copacity); ptrd+=whz; }
      }
      return *this;
    }

    //! Draw a colored point in the instance image, at coordinates (\c x0,\c y0).
    /**
       \param x0 = X-coordinate of the vector-valued pixel to plot.
       \param y0 = Y-coordinate of the vector-valued pixel to plot.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_point(const int x0,const int y0,const T *const color,const float opacity=1) { 
      return draw_point(x0,y0,0,color,opacity); 
    }

    //! Draw a 2D colored line in the instance image, at coordinates (\c x0,\c y0)-(\c x1,\c y1).
    /**
       \param x0 = X-coordinate of the starting point of the line.
       \param y0 = Y-coordinate of the starting point of the line.
       \param x1 = X-coordinate of the ending point of the line.
       \param y1 = Y-coordinate of the ending point of the line.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param pattern = An integer whose bits describes the line pattern.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_line(const int x0,const int y0,const int x1,const int y1,
                    const T *const color,const unsigned int pattern=~0L,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_line"); 
      if (!color) throw CImgArgumentException("CImg<%s>::draw_line() : specified color is (null)",pixel_type());
      const T* col=color;
      unsigned int hatch=1;     
      int nx0=x0, nx1=x1, ny0=y0, ny1=y1;
      if (nx0>nx1) cimg::swap(nx0,nx1,ny0,ny1);
      if (nx1<0 || nx0>=dimx()) return *this;
      if (nx0<0) { ny0-=nx0*(ny1-ny0)/(nx1-nx0); nx0=0; }
      if (nx1>=dimx()) { ny1+=(nx1-dimx())*(ny0-ny1)/(nx1-nx0); nx1=dimx()-1;}
      if (ny0>ny1) cimg::swap(nx0,nx1,ny0,ny1);
      if (ny1<0 || ny0>=dimy()) return *this;
      if (ny0<0) { nx0-=ny0*(nx1-nx0)/(ny1-ny0); ny0=0; }
      if (ny1>=dimy()) { nx1+=(ny1-dimy())*(nx0-nx1)/(ny1-ny0); ny1=dimy()-1;}
      const unsigned int dmax = (unsigned int)cimg::max(cimg::abs(nx1-nx0),ny1-ny0), whz = width*height*depth;
      const float px = dmax?(nx1-nx0)/(float)dmax:0, py = dmax?(ny1-ny0)/(float)dmax:0;
      float x = (float)nx0, y = (float)ny0;
      if (opacity>=1) for (unsigned int t=0; t<=dmax; t++) {
        if (!(~pattern) || (~pattern && pattern&hatch)) {
          T* ptrd = ptr((unsigned int)x,(unsigned int)y,0,0);      
          cimg_mapV(*this,k) { *ptrd=*(col++); ptrd+=whz; }
          col-=dim;
        }
        x+=px; y+=py; if (pattern) hatch=(hatch<<1)+(hatch>>(sizeof(unsigned int)*8-1));
      } else {
        const float nopacity = cimg::abs(opacity), copacity=1-cimg::max(opacity,0.0f);
        for (unsigned int t=0; t<=dmax; t++) {
          if (!(~pattern) || (~pattern && pattern&hatch)) {
            T* ptrd = ptr((unsigned int)x,(unsigned int)y,0,0);
            cimg_mapV(*this,k) { *ptrd = (T)(*(col++)*nopacity + copacity*(*ptrd)); ptrd+=whz; }
            col-=dim;
          }
          x+=px; y+=py; if (pattern) hatch=(hatch<<1)+(hatch>>(sizeof(unsigned int)*8-1));
        }
      }
      return *this;
    }
  
    //! Draw a 3D colored line in the instance image, at coordinates (\c x0,\c y0,\c z0)-(\c x1,\c y1,\c z1).
    /**
       \param x0 = X-coordinate of the starting point of the line.
       \param y0 = Y-coordinate of the starting point of the line.
       \param z0 = Z-coordinate of the starting point of the line.
       \param x1 = X-coordinate of the ending point of the line.
       \param y1 = Y-coordinate of the ending point of the line.
       \param z1 = Z-coordinate of the ending point of the line.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param pattern = An integer whose bits describes the line pattern.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_line(const int x0,const int y0,const int z0,const int x1,const int y1,const int z1,
                    const T *const color,const unsigned int pattern=~0L,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_line"); 
      if (!color) throw CImgArgumentException("CImg<%s>::draw_line() : specified color is (null)",pixel_type());
      const T* col=color;
      unsigned int hatch=1;
      int nx0=x0, ny0=y0, nz0=z0, nx1=x1, ny1=y1, nz1=z1;
      if (nx0>nx1) cimg::swap(nx0,nx1,ny0,ny1,nz0,nz1);
      if (nx1<0 || nx0>=dimx()) return *this;
      if (nx0<0) { const int D=nx1-nx0; ny0-=nx0*(ny1-ny0)/D; nz0-=nx0*(nz1-nz0)/D; nx0=0; }
      if (nx1>=dimx()) { const int d=nx1-dimx(), D=nx1-nx0; ny1+=d*(ny0-ny1)/D; nz1+=d*(nz0-nz1)/D; nx1=dimx()-1;}
      if (ny0>ny1) cimg::swap(nx0,nx1,ny0,ny1,nz0,nz1);
      if (ny1<0 || ny0>=dimy()) return *this;
      if (ny0<0) { const int D=ny1-ny0; nx0-=ny0*(nx1-nx0)/D; nz0-=ny0*(nz1-nz0)/D; ny0=0; }
      if (ny1>=dimy()) { const int d=ny1-dimy(), D=ny1-ny0; nx1+=d*(nx0-nx1)/D; nz1+=d*(nz0-nz1)/D; ny1=dimy()-1;}
      if (nz0>nz1) cimg::swap(nx0,nx1,ny0,ny1,nz0,nz1);
      if (nz1<0 || nz0>=dimz()) return *this;
      if (nz0<0) { const int D=nz1-nz0; nx0-=nz0*(nx1-nx0)/D; ny0-=nz0*(ny1-ny0)/D; nz0=0; }
      if (nz1>=dimz()) { const int d=nz1-dimz(), D=nz1-nz0; nx1+=d*(nx0-nx1)/D; ny1+=d*(ny0-ny1)/D; nz1=dimz()-1;}
      const unsigned int dmax = (unsigned int)cimg::max(cimg::abs(nx1-nx0),cimg::abs(ny1-ny0),nz1-nz0), whz = width*height*depth;
      const float px = dmax?(nx1-nx0)/(float)dmax:0, py = dmax?(ny1-ny0)/(float)dmax:0, pz = dmax?(nz1-nz0)/(float)dmax:0;
      float x = (float)nx0, y = (float)ny0, z = (float)nz0;
      if (opacity>=1) for (unsigned int t=0; t<=dmax; t++) { 
        if (!(~pattern) || (~pattern && pattern&hatch)) {
          T* ptrd = ptr((unsigned int)x,(unsigned int)y,(unsigned int)z,0);
          cimg_mapV(*this,k) { *ptrd=*(col++); ptrd+=whz; }        
          col-=dim; 
        }
        x+=px; y+=py; z+=pz; if (pattern) hatch=(hatch<<1)+(hatch>>(sizeof(unsigned int)*8-1));
      } else {
        const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
        for (unsigned int t=0; t<=dmax; t++) { 
          if (!(~pattern) || (~pattern && pattern&hatch)) {
            T* ptrd = ptr((unsigned int)x,(unsigned int)y,(unsigned int)z,0);
            cimg_mapV(*this,k) { *ptrd = (T)(*(col++)*nopacity + copacity*(*ptrd)); ptrd+=whz; }
            col-=dim; 
          }
          x+=px; y+=py; z+=pz; if (pattern) hatch=(hatch<<1)+(hatch>>(sizeof(unsigned int)*8-1));        
        }
      }
      return *this;
    }

    //! Draw a 2D textured line in the instance image, at coordinates (\c x0,\c y0)-(\c x1,\c y1).
    /**
       \param x0 = X-coordinate of the starting point of the line.
       \param y0 = Y-coordinate of the starting point of the line.
       \param x1 = X-coordinate of the ending point of the line.
       \param y1 = Y-coordinate of the ending point of the line.
       \param texture = a colored texture image used to draw the line color.
       \param tx0 = X-coordinate of the starting point of the texture.
       \param ty0 = Y-coordinate of the starting point of the texture.
       \param tx1 = X-coordinate of the ending point of the texture.
       \param ty1 = Y-coordinate of the ending point of the texture.
       \param opacity = opacity of the drawing.
       \note Clipping is supported, but texture coordinates do not support clipping.
    **/
    template<typename t> CImg& draw_line(const int x0,const int y0,const int x1,const int y1,
                                         const CImg<t>& texture,
                                         const int tx0,const int ty0,const int tx1,const int ty1,
                                         const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_line"); 
      cimg_test(texture,"CImg<T>::draw_line");
      if (texture.dim<dim)
        throw CImgArgumentException("CImg<%s>::draw_line() : texture has %u channel while image has %u channels",texture.dim,dim);
      int nx0=x0, ny0=y0, nx1=x1, ny1=y1, ntx0=tx0, nty0=ty0, ntx1=tx1, nty1=ty1;
      if (nx0>nx1) cimg::swap(nx0,nx1,ny0,ny1,ntx0,ntx1,nty0,nty1);
      if (nx1<0 || nx0>=dimx()) return *this;
      if (nx0<0) { const int D=nx1-nx0; ny0-=nx0*(ny1-ny0)/D; ntx0-=nx0*(ntx1-ntx0)/D; nty0-=nx0*(nty1-nty0)/D; nx0=0; }
      if (nx1>=dimx()) { const int d=nx1-dimx(),D=nx1-nx0; ny1+=d*(ny0-ny1)/D; ntx1+=d*(ntx0-ntx1)/D; nty1+=d*(nty0-nty1)/D; nx1=dimx()-1; }
      if (ny0>ny1) cimg::swap(nx0,nx1,ny0,ny1,ntx0,ntx1,nty0,nty1);
      if (ny1<0 || ny0>=dimy()) return *this;
      if (ny0<0) { const int D=ny1-ny0; nx0-=ny0*(nx1-nx0)/D; ntx0-=ny0*(ntx1-ntx0)/D; nty0-=ny0*(nty1-nty0)/D; ny0=0; }
      if (ny1>=dimy()) { const int d=ny1-dimy(),D=ny1-ny0; nx1+=d*(nx0-nx1)/D; ntx1+=d*(ntx0-ntx1)/D; nty1+=d*(nty0-nty1)/D; ny1=dimy()-1; }
      const unsigned int dmax = (unsigned int)cimg::max(cimg::abs(nx1-nx0),ny1-ny0), 
        whz = width*height*depth, twhz = texture.width*texture.height*texture.depth;
      const float px = dmax?(nx1-nx0)/(float)dmax:0, py = dmax?(ny1-ny0)/(float)dmax:0,
        tpx = dmax?(ntx1-ntx0)/(float)dmax:0, tpy = dmax?(nty1-nty0)/(float)dmax:0;
      float x = (float)nx0, y = (float)ny0, tx = (float)ntx0, ty = (float)nty0;
      if (opacity>=1) for (unsigned int tt=0; tt<=dmax; tt++) { 
        T *ptrd = ptr((unsigned int)x,(unsigned int)y,0,0);
        t *ptrs = texture.ptr((unsigned int)tx,(unsigned int)ty,0,0);
        cimg_mapV(*this,k) { *ptrd = (T)(*ptrs); ptrd+=whz; ptrs+=twhz; }
        x+=px; y+=py; tx+=tpx; ty+=tpy;
      } else {
        const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
        for (unsigned int tt=0; tt<=dmax; tt++) { 
          T *ptrd = ptr((unsigned int)x,(unsigned int)y,0,0);
          t *ptrs = texture.ptr((unsigned int)tx,(unsigned int)ty,0,0);
          cimg_mapV(*this,k) { *ptrd = (T)(nopacity*(*ptrs) + copacity*(*ptrd)); ptrd+=whz; ptrs+=twhz; }
          x+=px; y+=py; tx+=tpx; ty+=tpy;
        }
      }
      return *this;
    }

    //! Draw a 2D colored arrow in the instance image, at coordinates (\c x0,\c y0)->(\c x1,\c y1).
    /**
       \param x0 = X-coordinate of the starting point of the arrow (tail).
       \param y0 = Y-coordinate of the starting point of the arrow (tail).
       \param x1 = X-coordinate of the ending point of the arrow (head).
       \param y1 = Y-coordinate of the ending point of the arrow (head).
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param angle = aperture angle of the arrow head
       \param length = length of the arrow head. If <0, described as a percentage of the arrow length.
       \param pattern = An integer whose bits describes the line pattern.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_arrow(const int x0,const int y0,const int x1,const int y1,
                     const T *const color,
                     const float angle=30,const float length=-10,const unsigned int pattern=~0L,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_arrow");
      const float u = (float)(x0-x1), v = (float)(y0-y1), sq = u*u+v*v,
        deg = (float)(angle*cimg::PI/180), ang = (sq>0)?(float)std::atan2(v,u):0.0f,
        l = (length>=0)?length:-length*(float)std::sqrt(sq)/100;
      if (sq>0) {
        const double cl = std::cos(ang-deg), sl = std::sin(ang-deg), cr = std::cos(ang+deg), sr = std::sin(ang+deg);        
        const int 
          xl = x1+(int)(l*cl), yl = y1+(int)(l*sl),
          xr = x1+(int)(l*cr), yr = y1+(int)(l*sr),
          xc = x1+(int)((l+1)*(cl+cr))/2, yc = y1+(int)((l+1)*(sl+sr))/2;
        draw_line(x0,y0,xc,yc,color,pattern,opacity).draw_triangle(x1,y1,xl,yl,xr,yr,color,opacity);
      } else draw_point(x0,y0,color,opacity);
      return *this;
    }

    //! Draw a sprite image in the instance image, at coordinates (\c x0,\c y0,\c z0,\c v0).
    /**
       \param sprite = sprite image.
       \param x0 = X-coordinate of the sprite position in the instance image.
       \param y0 = Y-coordinate of the sprite position in the instance image.
       \param z0 = Z-coordinate of the sprite position in the instance image.
       \param v0 = V-coordinate of the sprite position in the instance image.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    template<typename t> CImg& draw_image(const CImg<t>& sprite,
                                          const int x0=0,const int y0=0,const int z0=0,const int v0=0,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_image");
      cimg_test(sprite,"CImg<T>::draw_image");
      const bool bx=(x0<0), by=(y0<0), bz=(z0<0), bv=(v0<0);
      const int 
        lX = sprite.dimx() - (x0+sprite.dimx()>dimx()?x0+sprite.dimx()-dimx():0) + (bx?x0:0),
        lY = sprite.dimy() - (y0+sprite.dimy()>dimy()?y0+sprite.dimy()-dimy():0) + (by?y0:0),
        lZ = sprite.dimz() - (z0+sprite.dimz()>dimz()?z0+sprite.dimz()-dimz():0) + (bz?z0:0),
        lV = sprite.dimv() - (v0+sprite.dimv()>dimv()?v0+sprite.dimv()-dimv():0) + (bv?v0:0);
      const t *ptrs = sprite.ptr()-(bx?x0:0)-(by?y0*sprite.dimx():0)+(bz?z0*sprite.dimx()*sprite.dimy():0)+
        (bv?v0*sprite.dimx()*sprite.dimy()*sprite.dimz():0);
      const unsigned int
        offX = width-lX, soffX = sprite.width-lX,
        offY = width*(height-lY), soffY = sprite.width*(sprite.height-lY),
        offZ = width*height*(depth-lZ), soffZ = sprite.width*sprite.height*(sprite.depth-lZ);
      const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
      T *ptrd = ptr(x0<0?0:x0,y0<0?0:y0,z0<0?0:z0,v0<0?0:v0);
      if (lX>0 && lY>0 && lZ>0 && lV>0)
        for (int v=0; v<lV; v++) {
          for (int z=0; z<lZ; z++) {
            for (int y=0; y<lY; y++) {
              if (opacity>=1) for (int x=0; x<lX; x++) *(ptrd++) = (T)(*(ptrs++));
              else for (int x=0; x<lX; x++) { *ptrd = (T)(nopacity*(*(ptrs++)) + copacity*(*ptrd)); ptrd++; }
              ptrd+=offX; ptrs+=soffX;
            }
            ptrd+=offY; ptrs+=soffY;
          }
          ptrd+=offZ; ptrs+=soffZ;
        }
      return *this;
    }

    // Add template overloading for VC++>=7.1
#if ( !defined(_MSC_VER) || _MSC_VER>1300 )
    CImg& draw_image(const CImg<T>& sprite,const int x0=0,const int y0=0,const int z0=0,const int v0=0,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_image");
      cimg_test(sprite,"CImg<T>::draw_image");
      if (this==&sprite) return draw_image(CImg<T>(sprite),x0,y0,z0,v0,opacity);
      const bool bx=(x0<0), by=(y0<0), bz=(z0<0), bv=(v0<0);
      const int 
        lX = sprite.dimx() - (x0+sprite.dimx()>dimx()?x0+sprite.dimx()-dimx():0) + (bx?x0:0),
        lY = sprite.dimy() - (y0+sprite.dimy()>dimy()?y0+sprite.dimy()-dimy():0) + (by?y0:0),
        lZ = sprite.dimz() - (z0+sprite.dimz()>dimz()?z0+sprite.dimz()-dimz():0) + (bz?z0:0),
        lV = sprite.dimv() - (v0+sprite.dimv()>dimv()?v0+sprite.dimv()-dimv():0) + (bv?v0:0);
      const T *ptrs = sprite.ptr()-(bx?x0:0)-(by?y0*sprite.dimx():0)+(bz?z0*sprite.dimx()*sprite.dimy():0)+
        (bv?v0*sprite.dimx()*sprite.dimy()*sprite.dimz():0);
      const unsigned int
        offX = width-lX, soffX = sprite.width-lX,
        offY = width*(height-lY), soffY = sprite.width*(sprite.height-lY),
        offZ = width*height*(depth-lZ), soffZ = sprite.width*sprite.height*(sprite.depth-lZ),
        slX = lX*sizeof(T);    
      const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
      T *ptrd = ptr(x0<0?0:x0,y0<0?0:y0,z0<0?0:z0,v0<0?0:v0);
      if (lX>0 && lY>0 && lZ>0 && lV>0)
        for (int v=0; v<lV; v++) {
          for (int z=0; z<lZ; z++) {
            if (opacity>=1) for (int y=0; y<lY; y++) { std::memcpy(ptrd,ptrs,slX); ptrd+=width; ptrs+=sprite.width; }
            else for (int y=0; y<lY; y++) {
              for (int x=0; x<lX; x++) { *ptrd = (T)(nopacity*(*(ptrs++)) + copacity*(*ptrd)); ptrd++; }
              ptrd+=offX; ptrs+=soffX;
            }
            ptrd+=offY; ptrs+=soffY;
          }
          ptrd+=offZ; ptrs+=soffZ;
        }
      return *this;
    }
#endif

    //! Draw a masked sprite image in the instance image, at coordinates (\c x0,\c y0,\c z0,\c v0).
    /**
       \param sprite = sprite image.
       \param mask = mask image.
       \param x0 = X-coordinate of the sprite position in the instance image.
       \param y0 = Y-coordinate of the sprite position in the instance image.
       \param z0 = Z-coordinate of the sprite position in the instance image.
       \param v0 = V-coordinate of the sprite position in the instance image.
       \param mask_valmax = Maximum pixel value of the mask image \c mask.
       \param opacity = opacity of the drawing.
       \note Pixel values of \c mask set the opacity of the corresponding pixels in \c sprite.
       \note Clipping is supported.
       \note Dimensions along x,y and z of \c sprite and \c mask must be the same.
    **/
    template<typename ti,typename tm> CImg& draw_image(const CImg<ti>& sprite,const CImg<tm>& mask,
                                                       const int x0=0,const int y0=0,const int z0=0,const int v0=0,
                                                       const tm mask_valmax=1,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_image");
      cimg_test(sprite,"CImg<T>::draw_image");
      cimg_test(mask,"CImg<T>::draw_image");
      if ((void*)this==(void*)&sprite) return draw_image(CImg<T>(sprite),mask,x0,y0,z0,v0);
      if(mask.width!=sprite.width || mask.height!=sprite.height || mask.depth!=sprite.depth)
        throw CImgArgumentException("CImg<%s>::draw_image() : mask dimension is (%u,%u,%u,%u), while sprite is (%u,%u,%u,%u)",
                                    pixel_type(),mask.width,mask.height,mask.depth,mask.dim,sprite.width,sprite.height,sprite.depth,sprite.dim);
      const bool bx=(x0<0), by=(y0<0), bz=(z0<0), bv=(v0<0);
      const int
        lX = sprite.dimx() - (x0+sprite.dimx()>dimx()?x0+sprite.dimx()-dimx():0) + (bx?x0:0),
        lY = sprite.dimy() - (y0+sprite.dimy()>dimy()?y0+sprite.dimy()-dimy():0) + (by?y0:0),
        lZ = sprite.dimz() - (z0+sprite.dimz()>dimz()?z0+sprite.dimz()-dimz():0) + (bz?z0:0),      
        lV = sprite.dimv() - (v0+sprite.dimv()>dimv()?v0+sprite.dimv()-dimv():0) + (bv?v0:0);    
      const int coff = -(bx?x0:0)-(by?y0*mask.dimx():0)-(bz?z0*mask.dimx()*mask.dimy():0)-
        (bv?v0*mask.dimx()*mask.dimy()*mask.dimz():0),
        ssize = mask.dimx()*mask.dimy()*mask.dimz();
      const ti *ptrs = sprite.ptr() + coff;
      const tm *ptrm = mask.ptr() + coff;
      const unsigned int
        offX = width-lX, soffX = sprite.width-lX,
        offY = width*(height-lY), soffY = sprite.width*(sprite.height-lY),
        offZ = width*height*(depth-lZ), soffZ = sprite.width*sprite.height*(sprite.depth-lZ);
      T *ptrd = ptr(x0<0?0:x0,y0<0?0:y0,z0<0?0:z0,v0<0?0:v0);
      if (lX>0 && lY>0 && lZ>0 && lV>0)
        for (int v=0; v<lV; v++) {
          ptrm = mask.data + (ptrm - mask.data)%ssize;
          for (int z=0; z<lZ; z++) {
            for (int y=0; y<lY; y++) {
              for (int x=0; x<lX; x++) {
                const float mopacity = *(ptrm++)*opacity,
                  nopacity = cimg::abs(mopacity), copacity = mask_valmax-cimg::max(mopacity,0.0f);
                *ptrd = (T)((nopacity*(*(ptrs++))+copacity*(*ptrd))/mask_valmax);
        ptrd++;
              }
              ptrd+=offX; ptrs+=soffX; ptrm+=soffX;
            }
            ptrd+=offY; ptrs+=soffY; ptrm+=soffY;
          }
          ptrd+=offZ; ptrs+=soffZ; ptrm+=soffZ;
        }
      return *this;
    }

    //! Draw a 4D filled rectangle in the instance image, at coordinates (\c x0,\c y0,\c z0,\c v0)-(\c x1,\c y1,\c z1,\c v1).
    /**
       \param x0 = X-coordinate of the upper-left rectangle corner in the instance image.
       \param y0 = Y-coordinate of the upper-left rectangle corner in the instance image.
       \param z0 = Z-coordinate of the upper-left rectangle corner in the instance image.
       \param v0 = V-coordinate of the upper-left rectangle corner in the instance image.
       \param x1 = X-coordinate of the lower-right rectangle corner in the instance image.
       \param y1 = Y-coordinate of the lower-right rectangle corner in the instance image.
       \param z1 = Z-coordinate of the lower-right rectangle corner in the instance image.
       \param v1 = V-coordinate of the lower-right rectangle corner in the instance image.
       \param val = scalar value used to fill the rectangle area.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_rectangle(const int x0,const int y0,const int z0,const int v0,
                         const int x1,const int y1,const int z1,const int v1,
                         const T& val,float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_rectangle");
      const bool bx=(x0<x1), by=(y0<y1), bz=(z0<z1), bv=(v0<v1);
      const int nx0=bx?x0:x1, nx1=bx?x1:x0, ny0=by?y0:y1, ny1=by?y1:y0, nz0=bz?z0:z1, nz1=bz?z1:z0, nv0=bv?v0:v1, nv1=bv?v1:v0;
      const int 
        lX = (1+nx1-nx0) + (nx1>=dimx()?dimx()-1-nx1:0) + (nx0<0?nx0:0),
        lY = (1+ny1-ny0) + (ny1>=dimy()?dimy()-1-ny1:0) + (ny0<0?ny0:0),
        lZ = (1+nz1-nz0) + (nz1>=dimz()?dimz()-1-nz1:0) + (nz0<0?nz0:0),
        lV = (1+nv1-nv0) + (nv1>=dimv()?dimv()-1-nv1:0) + (nv0<0?nv0:0);
      const unsigned int offX = width-lX, offY = width*(height-lY), offZ = width*height*(depth-lZ);
      const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
      T *ptrd = ptr(nx0<0?0:nx0,ny0<0?0:ny0,nz0<0?0:nz0,nv0<0?0:nv0);
      if (lX>0 && lY>0 && lZ>0 && lV>0)
        for (int v=0; v<lV; v++) {
          for (int z=0; z<lZ; z++) {
            for (int y=0; y<lY; y++) {
              if (opacity>=1) {
                if (sizeof(T)!=1) { for (int x=0; x<lX; x++) *(ptrd++) = val; ptrd+=offX; }
                else { std::memset(ptrd,(int)val,lX); ptrd+=width; }
              } else { for (int x=0; x<lX; x++) { *ptrd = (T)(nopacity*val+copacity*(*ptrd)); ptrd++; } ptrd+=offX; }
            }
            ptrd+=offY;
          }
          ptrd+=offZ;
        }  
      return *this;
    }

    //! Draw a 3D filled colored rectangle in the instance image, at coordinates (\c x0,\c y0,\c z0)-(\c x1,\c y1,\c z1).
    /**
       \param x0 = X-coordinate of the upper-left rectangle corner in the instance image.
       \param y0 = Y-coordinate of the upper-left rectangle corner in the instance image.
       \param z0 = Z-coordinate of the upper-left rectangle corner in the instance image.
       \param x1 = X-coordinate of the lower-right rectangle corner in the instance image.
       \param y1 = Y-coordinate of the lower-right rectangle corner in the instance image.
       \param z1 = Z-coordinate of the lower-right rectangle corner in the instance image.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_rectangle(const int x0,const int y0,const int z0,
                         const int x1,const int y1,const int z1,
                         const T *const color,const float opacity=1) {
      if (!color) throw CImgArgumentException("CImg<%s>::draw_rectangle : specified color is (null)",pixel_type());
      cimg_mapV(*this,k) draw_rectangle(x0,y0,z0,k,x1,y1,z1,k,color[k],opacity);
      return *this;
    }

    //! Draw a 2D filled colored rectangle in the instance image, at coordinates (\c x0,\c y0)-(\c x1,\c y1).
    /**
       \param x0 = X-coordinate of the upper-left rectangle corner in the instance image.
       \param y0 = Y-coordinate of the upper-left rectangle corner in the instance image.
       \param x1 = X-coordinate of the lower-right rectangle corner in the instance image.
       \param y1 = Y-coordinate of the lower-right rectangle corner in the instance image.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_rectangle(const int x0,const int y0,const int x1,const int y1,
                         const T *const color,const float opacity=1) {
      draw_rectangle(x0,y0,0,x1,y1,depth-1,color,opacity);
      return *this;
    }
  
    //! Draw a 2D filled colored triangle in the instance image, at coordinates (\c x0,\c y0)-(\c x1,\c y1)-(\c x2,\c y2).
    /**
       \param x0 = X-coordinate of the first corner in the instance image.
       \param y0 = Y-coordinate of the first corner in the instance image.
       \param x1 = X-coordinate of the second corner in the instance image.
       \param y1 = Y-coordinate of the second corner in the instance image.
       \param x2 = X-coordinate of the third corner in the instance image.
       \param y2 = Y-coordinate of the third corner in the instance image.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    CImg& draw_triangle(const int x0,const int y0,
                        const int x1,const int y1,
                        const int x2,const int y2,
                        const T *const color, const float opacity=1) {
      draw_scanline(color,opacity);
      int nx0=x0,ny0=y0,nx1=x1,ny1=y1,nx2=x2,ny2=y2;
      if (ny0>ny1) cimg::swap(nx0,nx1,ny0,ny1);
      if (ny0>ny2) cimg::swap(nx0,nx2,ny0,ny2);
      if (ny1>ny2) cimg::swap(nx1,nx2,ny1,ny2);
      if (ny0>=dimy() || ny2<0) return *this;
      const float 
        p1 = (ny1-ny0)?(nx1-nx0)/(float)(ny1-ny0):(nx1-nx0),
        p2 = (ny2-ny0)?(nx2-nx0)/(float)(ny2-ny0):(nx2-nx0),
        p3 = (ny2-ny1)?(nx2-nx1)/(float)(ny2-ny1):(nx2-nx1);
      float xleft = (float)nx0, xright = xleft, pleft = (p1<p2)?p1:p2, pright = (p1<p2)?p2:p1;
      if (ny0<0) { xleft-=ny0*pleft; xright-=ny0*pright; }
      const int ya = ny1>dimy()?height:ny1;
      for (int y=ny0<0?0:ny0; y<ya; y++) { draw_scanline((int)xleft,(int)xright,y,color,opacity); xleft+=pleft; xright+=pright; }
      if (p1<p2) { xleft=(float)nx1;  pleft=p3;  if (ny1<0) xleft-=ny1*pleft; } 
      else       { xright=(float)nx1; pright=p3; if (ny1<0) xright-=ny1*pright; }
      const int yb = ny2>=dimy()?height-1:ny2;
      for (int yy=ny1<0?0:ny1; yy<=yb; yy++) { draw_scanline((int)xleft,(int)xright,yy,color,opacity); xleft+=pleft; xright+=pright; }
      return *this;
    }
  
    //! Draw a 2D textured triangle in the instance image, at coordinates (\c x0,\c y0)-(\c x1,\c y1)-(\c x2,\c y2).
    /**
       \param x0 = X-coordinate of the first corner in the instance image.
       \param y0 = Y-coordinate of the first corner in the instance image.
       \param x1 = X-coordinate of the second corner in the instance image.
       \param y1 = Y-coordinate of the second corner in the instance image.
       \param x2 = X-coordinate of the third corner in the instance image.
       \param y2 = Y-coordinate of the third corner in the instance image.
       \param texture = texture image used to fill the triangle.
       \param tx0 = X-coordinate of the first corner in the texture image.
       \param ty0 = Y-coordinate of the first corner in the texture image.
       \param tx1 = X-coordinate of the second corner in the texture image.
       \param ty1 = Y-coordinate of the second corner in the texture image.
       \param tx2 = X-coordinate of the third corner in the texture image.
       \param ty2 = Y-coordinate of the third corner in the texture image.
       \param opacity = opacity of the drawing.
       \note Clipping is supported, but texture coordinates do not support clipping.
    **/
    template<typename t> CImg& draw_triangle(const int x0,const int y0,
                                             const int x1,const int y1,
                                             const int x2,const int y2,
                                             const CImg<t>& texture,
                                             const int tx0,const int ty0,
                                             const int tx1,const int ty1,
                                             const int tx2,const int ty2,
                                             const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_triangle"); 
      cimg_test(texture,"CImg<T>::draw_triangle");
      int nx0=x0,ny0=y0,nx1=x1,ny1=y1,nx2=x2,ny2=y2,ntx0=tx0,nty0=ty0,ntx1=tx1,nty1=ty1,ntx2=tx2,nty2=ty2,whz=width*height*depth;
      if (ny0>ny1) cimg::swap(nx0,nx1,ny0,ny1,ntx0,ntx1,nty0,nty1);
      if (ny0>ny2) cimg::swap(nx0,nx2,ny0,ny2,ntx0,ntx2,nty0,nty2);
      if (ny1>ny2) cimg::swap(nx1,nx2,ny1,ny2,ntx1,ntx2,nty1,nty2);
      if (ny0>=dimy() || ny2<0) return *this;
      const float 
        p1 = (ny1-ny0)?(nx1-nx0)/(float)(ny1-ny0):(nx1-nx0),
        p2 = (ny2-ny0)?(nx2-nx0)/(float)(ny2-ny0):(nx2-nx0),
        p3 = (ny2-ny1)?(nx2-nx1)/(float)(ny2-ny1):(nx2-nx1),
        tpx1 = (ny1-ny0)?(ntx1-ntx0)/(float)(ny1-ny0):0,
        tpy1 = (ny1-ny0)?(nty1-nty0)/(float)(ny1-ny0):0,
        tpx2 = (ny2-ny0)?(ntx2-ntx0)/(float)(ny2-ny0):0,
        tpy2 = (ny2-ny0)?(nty2-nty0)/(float)(ny2-ny0):0,
        tpx3 = (ny2-ny1)?(ntx2-ntx1)/(float)(ny2-ny1):0,
        tpy3 = (ny2-ny1)?(nty2-nty1)/(float)(ny2-ny1):0;
      const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
      float pleft,pright,tpxleft,tpyleft,tpxright,tpyright,
        xleft=(float)nx0,xright=xleft,txleft=(float)ntx0,tyleft=(float)nty0,txright=txleft,tyright=tyleft;
      if (p1<p2) { pleft=p1; pright=p2; tpxleft=tpx1; tpyleft=tpy1; tpxright=tpx2; tpyright=tpy2; } 
      else       { pleft=p2; pright=p1; tpxleft=tpx2; tpyleft=tpy2; tpxright=tpx1; tpyright=tpy1; }
      if (ny0<0) { xleft-=ny0*pleft; xright-=ny0*pright; txleft-=ny0*tpxleft; tyleft-=ny0*tpyleft;
        txright-=ny0*tpxright; tyright-=ny0*tpyright; }
      const int ya = ny1<dimy()?ny1:height;
      for (int y=(ny0<0?0:ny0); y<ya; y++) {
        const int dx = (int)xright-(int)xleft;
        const float
          tpx = dx?((int)txright-(int)txleft)/(float)dx:0,
          tpy = dx?((int)tyright-(int)tyleft)/(float)dx:0,        
          txi = (float)((xleft>=0)?(int)txleft:(int)(txleft-(int)xleft*tpx)),
          tyi = (float)((xleft>=0)?(int)tyleft:(int)(tyleft-(int)xleft*tpy));
        const int xmin=(xleft>=0)?(int)xleft:0, xmax=(xright<dimx())?(int)xright:(width-1);
        if (xmin<=xmax) {
          const int offx=whz-xmax+xmin-1;
          T* ptrd = ptr(xmin,y,0,0);
          if (opacity>=1) cimg_mapV(*this,k) {
            float tx=txi, ty=tyi;
            for (int x=xmin; x<=xmax; x++) { *(ptrd++)=(T)texture((unsigned int)tx,(unsigned int)ty,0,k); tx+=tpx; ty+=tpy; }
            ptrd+=offx;
          } else cimg_mapV(*this,k) {
            float tx=txi, ty=tyi;
            for (int x=xmin; x<=xmax; x++) { *ptrd=(T)(nopacity*texture((unsigned int)tx,(unsigned int)ty,0,k)+copacity*(*ptrd)); ptrd++; tx+=tpx; ty+=tpy; }
            ptrd+=offx;
          }
        }
        xleft+=pleft; xright+=pright; txleft+=tpxleft; tyleft+=tpyleft; txright+=tpxright; tyright+=tpyright;
      }

      if (p1<p2) {
        xleft=(float)nx1; pleft=p3; txleft=(float)ntx1; tyleft=(float)nty1; tpxleft=tpx3; tpyleft=tpy3;
        if (ny1<0) { xleft-=ny1*pleft; txleft-=ny1*tpxleft; tyleft-=ny1*tpyleft; }
      } else { 
        xright=(float)nx1; pright=p3; txright=(float)ntx1; tyright=(float)nty1; tpxright=tpx3; tpyright=tpy3;
        if (ny1<0) { xright-=ny1*pright; txright-=ny1*tpxright; tyright-=ny1*tpyright; }
      }    
      const int yb = ny2>=dimy()?(height-1):ny2;
      for (int yy=(ny1<0?0:ny1); yy<=yb; yy++) {
        const int dx = (int)xright-(int)xleft;
        const float
          tpx = dx?((int)txright-(int)txleft)/(float)dx:0,
          tpy = dx?((int)tyright-(int)tyleft)/(float)dx:0,        
          txi = (float)((xleft>=0)?(int)txleft:(int)(txleft-(int)xleft*tpx)),
          tyi = (float)((xleft>=0)?(int)tyleft:(int)(tyleft-(int)xleft*tpy));
        const int xmin=(xleft>=0)?(int)xleft:0, xmax=(xright<dimx())?(int)xright:(width-1);
        if (xmin<=xmax) {
          const int offx=whz-xmax+xmin-1;
          T* ptrd = ptr(xmin,yy,0,0);
          if (opacity>=1) cimg_mapV(*this,k) { 
            float tx=txi, ty=tyi;
            for (int x=xmin; x<=xmax; x++) { *(ptrd++)=(T)texture((unsigned int)tx,(unsigned int)ty,0,k); tx+=tpx; ty+=tpy; }
            ptrd+=offx;
          } else cimg_mapV(*this,k) { 
            float tx=txi, ty=tyi;
            for (int x=xmin; x<=xmax; x++) { *ptrd=(T)(nopacity*texture((unsigned int)tx,(unsigned int)ty,0,k)+copacity*(*ptrd)); ptrd++; tx+=tpx; ty+=tpy; }
            ptrd+=offx;
          }
        }
        xleft+=pleft; xright+=pright; txleft+=tpxleft; tyleft+=tpyleft; txright+=tpxright; tyright+=tpyright;
      }
      return *this;
    }

    //! Draw an ellipse on the instance image
    /**
       \param x0 = X-coordinate of the ellipse center.
       \param y0 = Y-coordinate of the ellipse center.
       \param r1 = First radius of the ellipse.
       \param r2 = Second radius of the ellipse.
       \param ru = X-coordinate of the orientation vector related to the first radius.
       \param rv = Y-coordinate of the orientation vector related to the first radius.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param pattern = If zero, the ellipse is filled, else pattern is an integer whose bits describe the outline pattern.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_ellipse(const int x0,const int y0,const float r1,const float r2,const float ru,const float rv,
               const T *const color,const unsigned int pattern=0L, const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_ellipse");
      draw_scanline(color,opacity);
      if (!color) throw CImgArgumentException("CImg<%s>::draw_ellipse : specified color is (null).",pixel_type());
      unsigned int hatch=1;
      const float
    nr1 = cimg::abs(r1), nr2 = cimg::abs(r2),
    norm = (float)std::sqrt(ru*ru+rv*rv),
    u = norm>0?ru/norm:1,
    v = norm>0?rv/norm:0,
    rmax = cimg::max(nr1,nr2),
    l1 = (float)std::pow(rmax/(nr1>0?nr1:1e-6),2),
    l2 = (float)std::pow(rmax/(nr2>0?nr2:1e-6),2),
    a = l1*u*u + l2*v*v,
    b = u*v*(l1-l2),
    c = l1*v*v + l2*u*u;
      const int
    yb = (int)std::sqrt(a*rmax*rmax/(a*c-b*b)),
    ymin = (y0-yb<0)?0:(y0-yb),
    ymax = (1+y0+yb>=dimy())?height-1:(1+y0+yb);
      int oxmin=0, oxmax=0;
      bool first_line = true;
      for (int y=ymin; y<=ymax; y++) {
    const float Y = (float)(y-y0), delta = b*b*Y*Y-a*(c*Y*Y-rmax*rmax), sdelta = (float)((delta>0?std::sqrt(delta):0));
    int xmin = (int)(x0-(b*Y+sdelta)/a), xmax = (int)(x0-(b*Y-sdelta)/a);
    if (!pattern) draw_scanline(xmin,xmax,y,color,opacity);
    else {
      if (!(~pattern) || (~pattern && pattern&hatch)) {
        if (first_line) { draw_scanline(xmin,xmax,y,color,opacity); first_line = false; }
        else {
          if (xmin<oxmin) draw_scanline(xmin,oxmin-1,y,color,opacity); 
          else draw_scanline(oxmin+(oxmin==xmin?0:1),xmin,y,color,opacity);
          if (xmax<oxmax) draw_scanline(xmax,oxmax-1,y,color,opacity); 
          else draw_scanline(oxmax+(oxmax==xmax?0:1),xmax,y,color,opacity);    
        }
      }
    }
    oxmin = xmin; oxmax = xmax;
    if (pattern) hatch=(hatch<<1)+(hatch>>(sizeof(unsigned int)*8-1));
      }
      return *this;
    }
    
    //! Draw an ellipse on the instance image
    /**
       \param x0 = X-coordinate of the ellipse center.
       \param y0 = Y-coordinate of the ellipse center.
       \param tensor = Diffusion tensor describing the ellipse.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param pattern = If zero, the ellipse is filled, else pattern is an integer whose bits describe the outline pattern.
       \param opacity = opacity of the drawing.
    **/
    template<typename t> CImg& draw_ellipse(const int x0,const int y0,const CImg<t> &tensor,
                        const T *color,const unsigned int pattern=0L,const float opacity=1) {
      CImgl<t> eig = tensor.get_symeigen();
      const CImg<t> &val = eig[0], &vec = eig[1];
      return draw_ellipse(x0,y0,val(0),val(1),vec(0),vec(1),color,pattern,opacity);
    }
    
    //! Draw a circle on the instance image
    /**
       \param x0 = X-coordinate of the circle center.
       \param y0 = Y-coordinate of the circle center.
       \param r = radius of the circle.
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param pattern = If zero, the circle is filled, else pattern is an integer whose bits describe the outline pattern.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_circle(const int x0,const int y0,float r,const T *const color,const unsigned int pattern=0L,const float opacity=1) {
      return draw_ellipse(x0,y0,r,r,1,0,color,pattern,opacity);
    }
  
    //! Draw a text into the instance image.
    /**
       \param text = a C-string containing the text to display.
       \param x0 = X-coordinate of the text in the instance image.
       \param y0 = Y-coordinate of the text in the instance image.
       \param fgcolor = an array of dimv() values of type \c T, defining the foreground color (NULL means 'transparent').
       \param bgcolor = an array of dimv() values of type \c T, defining the background color (NULL means 'transparent').
       \param font = List of font characters used for the drawing.
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
       \see get_font7x11().
    **/
    template<typename t> CImg& draw_text(const char *const text,
                                         const int x0,const int y0,
                                         const T *const fgcolor,const T *const bgcolor,
                                         const CImgl<t>& font,const float opacity=1) {

      cimgl_test(font,"CImg<%s>::draw_text");
      if (!text) throw CImgArgumentException("CImg<%s>::draw_text() : NULL string specified as input",pixel_type());
      if (!width || !height || !depth || !dim || !data) {
    // If needed, pre-compute needed size of the image
    int x=0, y=0, w=0;
    for (int i=0; i<cimg::strlen(text); i++) {
      const unsigned char c = text[i];
      switch (c) {
      case '\n': y+=font[' '].height; if (x>w) w=x; x=0; break;
      case '\t': x+=4*font[' '].width; break;
      default: if (c<font.size) x+=font[c].width;
      }
    }    
    if (x!=0) {
      if (x>w) w=x;
      y+=font[' '].height;
    }
    (*this) = CImg<T>(x0+w,y0+y,1,font[' '].dim,0);
    if (bgcolor) cimg_mapV(*this,k) ref_channel(k).fill(bgcolor[k]);
      }

      int x=x0, y=y0;
      CImg letter;
      for (int i=0; i<cimg::strlen(text); i++) {
        const unsigned char c = text[i];
        switch (c) {
        case '\n': y+=font[' '].height; x=x0; break;
        case '\t': x+=4*font[' '].width; break;
        default: if (c<font.size) {
            letter = font[c];
            const CImg& mask = (c+256)<(int)font.size?font[c+256]:font[c];
            if (fgcolor) for (unsigned int p=0; p<letter.width*letter.height; p++) 
          if (mask(p)) cimg_mapV(*this,k) letter(p,0,0,k)=(T)(letter(p,0,0,k)*fgcolor[k]);
            if (bgcolor) for (unsigned int p=0; p<letter.width*letter.height; p++)
          if (!mask(p)) cimg_mapV(*this,k) letter(p,0,0,k)=bgcolor[k];
            if (!bgcolor && font.size>=512) draw_image(letter,mask,x,y,0,0,(T)1,opacity); else draw_image(letter,x,y,0,0,opacity);
            x+=letter.width;
          }
          break;
        }
      }
      return *this;
    }


    //! Draw a text into the instance image.
    /**
       \param text = a C-string containing the text to display.
       \param x0 = X-coordinate of the text in the instance image.
       \param y0 = Y-coordinate of the text in the instance image.
       \param fgcolor = an array of dimv() values of type \c T, defining the foreground color (NULL means 'transparent').
       \param bgcolor = an array of dimv() values of type \c T, defining the background color (NULL means 'transparent').
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
       \see get_font7x11().
    **/
    CImg& draw_text(const char *const text,
                    const int x0,const int y0,
                    const T *const fgcolor=NULL,const T *const bgcolor=NULL,
                    const float opacity=1) {
      static bool first = true;
      static CImgl<T> default_font;
      if (first) { default_font = CImgl<T>::get_font7x11(); first = false; }
      return draw_text(text,x0,y0,fgcolor,bgcolor,default_font,opacity);
    }
  
    //! Draw a text into the instance image.
    /**
       \param x0 = X-coordinate of the text in the instance image.
       \param y0 = Y-coordinate of the text in the instance image.
       \param fgcolor = an array of dimv() values of type \c T, defining the foreground color (NULL means 'transparent').
       \param bgcolor = an array of dimv() values of type \c T, defining the background color (NULL means 'transparent').
       \param opacity = opacity of the drawing.
       \param format = a 'printf'-style format, followed by arguments.
       \note Clipping is supported.
       \see get_font7x11().
    **/
    CImg& draw_text(const int x0,const int y0,
                    const T *const fgcolor,const T *const bgcolor,
                    const float opacity,const char *format,...) {
      char tmp[2048]; 
      std::va_list ap;
      va_start(ap,format);
      std::vsprintf(tmp,format,ap);
      va_end(ap);
      return draw_text(tmp,x0,y0,fgcolor,bgcolor,opacity);
    }
    template<typename t> CImg& draw_text(const int x0,const int y0,
                                         const T *const fgcolor,const T *const bgcolor,
                                         const CImgl<t>& font, const float opacity, const char *format,...) {
      char tmp[2048]; std::va_list ap; va_start(ap,format); std::vsprintf(tmp,format,ap); va_end(ap);
      return draw_text(tmp,x0,y0,fgcolor,bgcolor,font);
    }
  
    //! Draw a vector field in the instance image.
    /**
       \param flow = a 2d image of 2d vectors used as input data.
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param sampling = length (in pixels) between each arrow.
       \param factor = length factor of each arrow (if <0, computed as a percentage of the maximum length).
       \param quiver_type = type of plot. Can be 0 (arrows) or 1 (segments).
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    template<typename t> 
    CImg& draw_quiver(const CImg<t>& flow,const T *const color,const unsigned int sampling=25,const float factor=-20,
                      const int quiver_type=0,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_quiver");
      cimg_test(flow,"CImg<T>::draw_quiver");
      if (!color) 
        throw CImgArgumentException("CImg<%s>::draw_quiver() : specified color is (null)",pixel_type());
      if (sampling<=0)
        throw CImgArgumentException("CImg<%s>::draw_quiver() : incorrect sampling value = %g",pixel_type(),sampling);
      if (flow.dim!=2)
        throw CImgArgumentException("CImg<%s>::draw_quiver() : specified flow has invalid dimensions (%u,%u,%u,%u)",
                                    pixel_type(),flow.width,flow.height,flow.depth,flow.dim);
      float vmax,fact;
      if (factor<=0) {
        CImgStats st(flow.get_norm_pointwise(2),false);
        vmax = (float)cimg::max(cimg::abs(st.min),cimg::abs(st.max));
        fact = -factor;
      } else { fact = factor; vmax = 1; }

      for (unsigned int y=sampling/2; y<height; y+=sampling)
        for (unsigned int x=sampling/2; x<width; x+=sampling) {
          const unsigned int X = x*flow.width/width, Y = y*flow.height/height;
          float u = (float)flow(X,Y,0,0)*fact/vmax, v = (float)flow(X,Y,0,1)*fact/vmax;
          if (!quiver_type) {
            const int xx = x+(int)u, yy = y+(int)v;
            draw_arrow(x,y,xx,yy,color,45.0f,sampling/5.0f,~0L,opacity);
          } else draw_line((int)(x-0.5*u),(int)(y-0.5*v),(int)(x+0.5*u),(int)(y+0.5*v),color,~0L,opacity);
        }
      return *this; 
    }

    //! Draw a vector field in the instance image, using a colormap.
    /**
       \param flow = a 2d image of 2d vectors used as input data.
       \param color = a 2d image of dimv()-D vectors corresponding to the color of each arrow.
       \param sampling = length (in pixels) between each arrow.
       \param factor = length factor of each arrow (if <0, computed as a percentage of the maximum length).
       \param quiver_type = type of plot. Can be 0 (arrows) or 1 (segments).
       \param opacity = opacity of the drawing.
       \note Clipping is supported.
    **/
    template<typename t1,typename t2>
      CImg& draw_quiver(const CImg<t1>& flow,const CImg<t2>& color,const unsigned int sampling=25,const float factor=-20,
                        const int quiver_type=0,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_quiver"); 
      cimg_test(flow,"CImg<T>::draw_quiver");
      cimg_test(color,"CImg<T>::draw_quiver");
      if (sampling<=0)
        throw CImgArgumentException("CImg<%s>::draw_quiver() : incorrect sampling value = %g",pixel_type(),sampling);
      if (flow.dim!=2)
        throw CImgArgumentException("CImg<%s>::draw_quiver() : specified flow has invalid dimensions (%u,%u,%u,%u)",
                                    pixel_type(),flow.width,flow.height,flow.depth,flow.dim);
      if (color.width!=flow.width || color.height!=flow.height)
        throw CImgArgumentException("CImg<%s>::draw_quiver() : input color data map=(%u,%u,%u,%u)\
 and data flow=(%u,%u,%u,%u) must have same dimension.",
                                    color.width,color.height,color.depth,color.data,
                                    flow.width,flow.height,flow.depth,flow.data);
      float vmax,fact;
      if (factor<=0) {
        CImgStats st(flow.get_norm_pointwise(2),false);
        vmax = (float)cimg::max(cimg::abs(st.min),cimg::abs(st.max));
        fact = -factor;
      } else { fact = factor; vmax = 1; }

      for (unsigned int y=sampling/2; y<height; y+=sampling)
        for (unsigned int x=sampling/2; x<width; x+=sampling) {
          const unsigned int X = x*flow.width/width, Y = y*flow.height/height;
          float u = (float)flow(X,Y,0,0)*fact/vmax, v = (float)flow(X,Y,0,1)*fact/vmax;
          if (!quiver_type) {
            const int xx = x+(int)u, yy = y+(int)v;
            draw_arrow(x,y,xx,yy,color.get_vector(X,Y).data,45,sampling/5,~0L,opacity);
          } else draw_line((int)(x-0.5*u),(int)(y-0.5*v),(int)(x+0.5*u),(int)(y+0.5*v),color(X,Y),~0L,opacity);
        }
      return *this; 
    }

    //! Draw a 1D graph on the instance image.
    /**
       \param data = an image containing the graph values I = f(x).
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param gtype = define the type of the plot :
                      - 0 = Plot using linear interpolation (segments).
              - 1 = Plot with bars.
              - 2 = Plot using cubic interpolation (3-polynomials).
       \param ymin = lower bound of the y-range.
       \param ymax = upper bound of the y-range.
       \param opacity = opacity of the drawing.
       \note
         - if \c ymin==ymax==0, the y-range is computed automatically from the input sample.
       \see draw_axeX(), draw_axeY(), draw_axeXY().
    **/
    template<typename t>
    CImg& draw_graph(const CImg<t>& data,const T *const color,const unsigned int gtype=0,
                     const double ymin=0,const double ymax=0,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_graph");
      if (!color) throw CImgArgumentException("CImg<%s>::draw_graph() : specified color is (null)",pixel_type());
      T *color1 = new T[dim], *color2 = new T[dim];
      cimg_mapV(*this,k) { color1[k]=(T)(color[k]*0.6f); color2[k]=(T)(color[k]*0.3f); }
      CImgStats st;
      if (ymin==ymax) { st = CImgStats(data,false); cimg::swap(st.min,st.max); } else { st.min = ymin; st.max = ymax; }
      if (st.min==st.max) { st.min--; st.max++; }
      const float ca = height>1?(float)(st.max-st.min)/(height-1):0, cb = (float)st.min;
      const int Y0 = (int)(-cb/ca);
      int pY=0;
      cimg_mapoff(data,off) {     
        const int Y = (int)((data[off]-cb)/ca);
        switch (gtype) {
        case 0: // plot with segments
          if (off>0) draw_line((int)((off-1)*width/data.size()),pY,(int)(off*width/data.size()),Y,color,~0L,opacity);
          break;
        case 1: { // plot with bars
          const unsigned int X = off*width/data.size(), nX = (off+1)*width/data.size()-1;
          draw_rectangle(X,(int)Y0,nX,Y,color1,opacity);
      draw_line(X,Y,X,(int)Y0,color2,~0L,opacity);
      draw_line(X,(int)Y0,nX,(int)Y0,Y<=Y0?color2:color,~0L,opacity);
      draw_line(nX,Y,nX,(int)Y0,color,~0L,opacity);
      draw_line(X,Y,nX,Y,Y<=Y0?color:color2,~0L,opacity);
        } break;
        }        
        pY=Y;
      }
      if (gtype==2) { // plot with cubic interpolation
    const CImgROI<t> ndata(data.size(),1,1,1,data.ptr());
        cimg_mapX(*this,x) {
          const int Y = (int)((ndata.cubic_pix1d((float)x*ndata.width/width)-cb)/ca);
          if (x>0) draw_line(x,pY,x+1,Y,color,~0L,opacity);
          pY=Y;
        }
      }
      delete[] color1; delete[] color2;
      return *this;     
    }

    //! Draw a labelled horizontal axis on the instance image.
    /** 
       \param x0 = lower bound of the x-range.
       \param x1 = upper bound of the x-range.
       \param y = Y-coordinate of the horizontal axis in the instance image.
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param precision = precision of the labels.
       \param opacity = opacity of the drawing.
       \note if \c precision==0, precision of the labels is automatically computed.
       \see draw_graph(), draw_axeY(), draw_axeXY().
    **/
    CImg& draw_axeX(const double x0,const double x1,const int y,const T *const color,
            const double precision=0,const float opacity=1) {
      if (x0==x1) return *this;
      if (x0<x1) draw_arrow(0,y,width-1,y,color,30,5,~0L,opacity);
      else draw_arrow(width-1,y,0,y,color,30,5,~0L,opacity);
      const int yt = (y+14)<dimy()?(y+3):(y-14);
      double nprecision=precision;
      if (precision<=0) { 
    const double nb_pow = std::floor(std::log10(cimg::abs(x1-x0)))-1;
    nprecision = std::pow(10.0,nb_pow);
    while ((cimg::abs(x1-x0)/nprecision)>(dimx()/40)) nprecision*=2;
      }
      const double xmin=x0<x1?x0:x1, xmax=x0<x1?x1:x0,
    tx0 = cimg::mod(xmin,nprecision)==0?xmin:((xmin+nprecision)-cimg::mod(xmin+nprecision,nprecision)),
    tx1 = cimg::mod(xmax,nprecision)==0?xmax:((xmax+nprecision)-cimg::mod(xmax+nprecision,nprecision));
      char txt[32];
      for (double x=tx0; x<=tx1; x+=nprecision) {
    std::sprintf(txt,"%g",x);           
    const int xi=(int)((x-x0)*(width-1)/(x1-x0)), xt = xi-(int)std::strlen(txt)*3;
    draw_point(xi,y-1,color,opacity).draw_point(xi,y+1,color,opacity).
      draw_text(txt,xt<0?0:xt,yt,color,NULL,opacity);
      }
      return *this;
    }

    //! Draw a labelled vertical axis on the instance image.
    /** 
       \param x = X-coordinate of the vertical axis in the instance image.
       \param y0 = lower bound of the y-range.
       \param y1 = upper bound of the y-range.
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param precision = precision of the labels.
       \param opacity = opacity of the drawing.
       \note if \c precision==0, precision of the labels is automatically computed.
       \see draw_graph(), draw_axeX(), draw_axeXY().
    **/
    CImg& draw_axeY(const int x,const double y0,const double y1,const T *const color,
            const double precision=0,const float opacity=1) {
      if (y0==y1) return *this;
      if (y0<y1) draw_arrow(x,0,x,height-1,color,30,5,~0L,opacity);
      else draw_arrow(x,height-1,x,0,color,30,5,~0L,opacity);
      double nprecision=precision;
      if (precision<=0) {
    const double nb_pow = std::floor(std::log10(cimg::abs(y1-y0)))-1;
    nprecision = std::pow(10.0,nb_pow);
    while ((cimg::abs(y1-y0)/nprecision)>(dimy()/40)) nprecision*=2;
      }
      const double ymin=y0<y1?y0:y1, ymax=y0<y1?y1:y0,
    ty0 = cimg::mod(ymin,nprecision)==0?ymin:((ymin+nprecision)-cimg::mod(ymin+nprecision,nprecision)),
    ty1 = cimg::mod(ymax,nprecision)==0?ymax:((ymax+nprecision)-cimg::mod(ymax+nprecision,nprecision));
      char txt[32];
      for (double y=ty0; y<=ty1; y+=nprecision) {
    std::sprintf(txt,"%g",y);
    const int yi = (int)((y-y0)*(height-1)/(y1-y0)), xt = x-(int)std::strlen(txt)*7;
    draw_point(x-1,yi,color,opacity).draw_point(x+1,yi,color,opacity);
    if (xt>0) draw_text(txt,xt,yi-5,color,NULL,opacity);
    else draw_text(txt,x+3,yi-5,color,NULL,opacity);
      }
      return *this;
    }

    //! Draw a labelled coordinate system (X,Y) on the instance image.
    /** 
       \param x0 = lower bound of the x-range.
       \param x1 = upper bound of the x-range.
       \param y0 = lower bound of the y-range.
       \param y1 = upper bound of the y-range.
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param precisionx = precision of the labels along the X-axis.
       \param precisiony = precision of the labels along the Y-axis.
       \param opacity = opacity of the drawing.
       \note if precision==0, precision of the labels along the specified axix is automatically computed.
       \see draw_graph(), draw_axeX(), draw_axeY().
    **/
    CImg& draw_axeXY(const double x0,const double x1,const double y0,const double y1,const T *const color,
             const double precisionx=0,const double precisiony=0,const float opacity=1) {
      if (x0*x1<=0) {
        const int xz = (int)(-x0*(width-1)/(x1-x0));
        if (xz>=0 && xz<dimx()) draw_axeY(xz,y0,y1,color,precisiony,opacity);
      }
      if (y0*y1<=0) {
        const int yz = (int)(-y0*(height-1)/(y1-y0));
        if (yz>=0 && yz<dimy()) draw_axeX(x0,x1,yz,color,precisionx,opacity);
      }
      return *this;
    }
  
    // Local class used by function CImg<>::draw_fill()
    template<typename T1,typename T2> struct _draw_fill {
      const T1 *const color;
      const float sigma,opacity;
      const CImg<T1> value;
      CImg<T2> region;

      _draw_fill(const CImg<T1>& img,const int x,const int y,const int z,
                 const T *const pcolor,const float psigma,const float popacity):
        color(pcolor),sigma(psigma),opacity(popacity),
        value(img.get_vector(x,y,z)), region(CImg<T2>(img.width,img.height,img.depth,1,(T2)false)) {
        cimg_test(img,"CImg<T>::draw_fill");
        if (!color) throw CImgArgumentException("CImg<%s>::draw_fill() : specified color is (null)",img.pixel_type());
      }

       _draw_fill& operator=(const _draw_fill& d) {
            color = d.color;
            sigma = d.sigma;
            opacity = d.opacity;
            value = d.value;
            region = d.region;
        }

      bool comp(const CImg<T1>& A,const CImg<T1>& B) const {
        bool res=true;
        const T *pA=A.data+A.size();
        for (const T *pB=B.data+B.size(); res && pA>A.data; res=(cimg::abs(*(--pA)-(*(--pB)))<=sigma) );
        return res;
      }

      void fill(CImg<T1>& img,const int x,const int y,const int z) {
        if (x<0 || x>=img.dimx() || y<0 || y>=img.dimy() || z<0 || z>=img.dimz()) return;
        if (!region(x,y,z) && comp(value,img.get_vector(x,y,z))) {
          const T *col=color;
          const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
          int xmin,xmax;
          if (opacity>=1) cimg_mapV(img,k) img(x,y,z,k)=*(col++);
          else cimg_mapV(img,k) img(x,y,z,k)=(T1)(*(col++)*opacity+copacity*img(x,y,z,k));
          col-=img.dim;
          region(x,y,z) = (T2)true;
          for (xmin=x-1; xmin>=0 && comp(value,img.get_vector(xmin,y,z)); xmin--) {
            if (opacity>=1) cimg_mapV(img,k) img(xmin,y,z,k) = *(col++);
            else cimg_mapV(img,k) img(xmin,y,z,k)=(T1)(*(col++)*nopacity+copacity*img(xmin,y,z,k)); 
            col-=img.dim;
            region(xmin,y,z)=(T2)true;
          }
          for (xmax=x+1; xmax<img.dimx() && comp(value,img.get_vector(xmax,y,z)); xmax++) {
            if (opacity>=1) cimg_mapV(img,k) img(xmax,y,z,k) = *(col++);
            else cimg_mapV(img,k) img(xmax,y,z,k)=(T1)(*(col++)*nopacity+copacity*img(xmax,y,z,k));
            col-=img.dim;
            region(xmax,y,z)=(T2)true; 
          }
          xmin++; xmax--;
          for (; xmin<=xmax; xmin++) { 
            fill(img,xmin,y-1,z); 
            fill(img,xmin,y+1,z);
            fill(img,xmin,y,z-1); 
            fill(img,xmin,y,z+1);
          }
        }
      }        
    };

    //! Draw a 3D filled region starting from a point (\c x,\c y,\ z) in the instance image.
    /**
       \param x = X-coordinate of the starting point of the region to fill.
       \param y = Y-coordinate of the starting point of the region to fill.
       \param z = Z-coordinate of the starting point of the region to fill.
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param region = image that will contain the mask of the filled region mask, as an output.
       \param sigma = tolerance concerning neighborhood values.
       \param opacity = opacity of the drawing.

       \return \p region is initialized with the binary mask of the filled region.
    **/
    template<typename t> CImg& draw_fill(const int x,const int y,const int z,
                                         const T *const color, CImg<t>& region,const float sigma=0,
                                         const float opacity=1) {
      _draw_fill<T,t> F(*this,x,y,z,color,sigma,opacity);
      F.fill(*this,x,y,z);
      region = F.region;
      return *this;
    }

    //! Draw a 3D filled region starting from a point (\c x,\c y,\ z) in the instance image.
    /**
       \param x = X-coordinate of the starting point of the region to fill.
       \param y = Y-coordinate of the starting point of the region to fill.
       \param z = Z-coordinate of the starting point of the region to fill.
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param sigma = tolerance concerning neighborhood values.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_fill(const int x,const int y,const int z,const T *const color,const float sigma=0,const float opacity=1) {
      CImg<bool> tmp;
      return draw_fill(x,y,z,color,tmp,sigma,opacity);
    }

    //! Draw a 2D filled region starting from a point (\c x,\c y) in the instance image.
    /**
       \param x = X-coordinate of the starting point of the region to fill.
       \param y = Y-coordinate of the starting point of the region to fill.
       \param color = an array of dimv() values of type \c T, defining the drawing color.
       \param sigma = tolerance concerning neighborhood values.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_fill(const int x,const int y,const T *const color,const float sigma=0,const float opacity=1) {      
      CImg<bool> tmp;
      return draw_fill(x,y,0,color,tmp,sigma,opacity);
    }

    //! Draw a plasma square in the instance image.
    /**
       \param x0 = X-coordinate of the upper-left corner of the plasma.
       \param y0 = Y-coordinate of the upper-left corner of the plasma.
       \param x1 = X-coordinate of the lower-right corner of the plasma.
       \param y1 = Y-coordinate of the lower-right corner of the plasma.
       \param alpha = Alpha-parameter of the plasma.
       \param beta = Beta-parameter of the plasma.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_plasma(const int x0,const int y0,const int x1,const int y1,
                      const double alpha=1.0,const double beta=1.0,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_plasma");
      int nx0=x0,nx1=x1,ny0=y0,ny1=y1;
      if (nx1<nx0) cimg::swap(nx0,nx1);
      if (ny1<ny0) cimg::swap(ny0,ny1);
      if (nx0<0) nx0=0;
      if (nx1>=dimx()) nx1=width-1;
      if (ny0<0) ny0=0;
      if (ny1>=dimy()) ny1=height-1;
      const int xc = (nx0+nx1)/2, yc = (ny0+ny1)/2, dx=(xc-nx0), dy=(yc-ny0);
      const double dc = std::sqrt((double)(dx*dx+dy*dy))*alpha + beta;
      cimg_mapV(*this,k) {
        if (opacity>=1) {
          (*this)(xc,ny0,0,k) = (T)(0.5*((*this)(nx0,ny0,0,k)+(*this)(nx1,ny0,0,k)));
          (*this)(xc,ny1,0,k) = (T)(0.5*((*this)(nx0,ny1,0,k)+(*this)(nx1,ny1,0,k)));
          (*this)(nx0,yc,0,k) = (T)(0.5*((*this)(nx0,ny0,0,k)+(*this)(nx0,ny1,0,k)));
          (*this)(nx1,yc,0,k) = (T)(0.5*((*this)(nx1,ny0,0,k)+(*this)(nx1,ny1,0,k)));
          (*this)(xc,yc,0,k)  = (T)(0.25*((*this)(nx0,ny0,0,k)+(*this)(nx1,ny0,0,k) +
                                          (*this)(nx1,ny1,0,k)+(*this)(nx0,ny1,0,k)) + dc*cimg::grand());
        } else {
          const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
          (*this)(xc,ny0,0,k) = (T)(0.5*((*this)(nx0,ny0,0,k)+(*this)(nx1,ny0,0,k))*nopacity + copacity*(*this)(xc,ny0,0,k));
          (*this)(xc,ny1,0,k) = (T)(0.5*((*this)(nx0,ny1,0,k)+(*this)(nx1,ny1,0,k))*nopacity + copacity*(*this)(xc,ny1,0,k));
          (*this)(nx0,yc,0,k) = (T)(0.5*((*this)(nx0,ny0,0,k)+(*this)(nx0,ny1,0,k))*nopacity + copacity*(*this)(nx0,yc,0,k));
          (*this)(nx1,yc,0,k) = (T)(0.5*((*this)(nx1,ny0,0,k)+(*this)(nx1,ny1,0,k))*nopacity + copacity*(*this)(nx1,yc,0,k));
          (*this)(xc,yc,0,k)  = (T)(0.25*(((*this)(nx0,ny0,0,k)+(*this)(nx1,ny0,0,k) +
                                           (*this)(nx1,ny1,0,k)+(*this)(nx0,ny1,0,k)) + dc*cimg::grand())*nopacity
                                    + copacity*(*this)(xc,yc,0,k));
        }
      }
      if (xc!=nx0 || yc!=ny0) { 
        draw_plasma(nx0,ny0,xc,yc,alpha,beta,opacity);
        draw_plasma(xc,ny0,nx1,yc,alpha,beta,opacity);
        draw_plasma(nx0,yc,xc,ny1,alpha,beta,opacity);
        draw_plasma(xc,yc,nx1,ny1,alpha,beta,opacity); 
      }
      return *this;
    }

    //! Draw a plasma in the instance image.
    /**
       \param alpha = Alpha-parameter of the plasma.
       \param beta = Beta-parameter of the plasma.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_plasma(const double alpha=1.0,const double beta=1.0,const float opacity=1) {
      return draw_plasma(0,0,width-1,height-1,alpha,beta,opacity);
    }
  
    //! Draw a 1D gaussian function in the instance image.
    /**
       \param xc = X-coordinate of the gaussian center.
       \param sigma = Standart variation of the gaussian distribution.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_gaussian(const float xc,const double sigma,const T *const color,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_gaussian");
      const double sigma2 = 2*sigma*sigma;
      const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
      const unsigned int whz = width*height*depth;
      const T *col = color;
      cimg_mapX(*this,x) {
        const float dx = (x-xc);
        const double val = std::exp( -dx*dx/sigma2 );
        T *ptrd = ptr(x,0,0,0);
        if (opacity>=1) cimg_mapV(*this,k) { *ptrd = (T)(val*(*col++)); ptrd+=whz; }
        else cimg_mapV(*this,k) { *ptrd = (T)(nopacity*val*(*col++) + copacity*(*ptrd)); ptrd+=whz; } 
        col-=dim;
      }
      return *this;
    }

    //! Draw an anisotropic 2D gaussian function in the instance image.
    /**
       \param xc = X-coordinate of the gaussian center.
       \param yc = Y-coordinate of the gaussian center.       
       \param tensor = 2x2 covariance matrix.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
    **/
    template<typename t> CImg& draw_gaussian(const float xc,const float yc,const CImg<t>& tensor,
                         const T *const color,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_gaussian"); 
      if (tensor.width!=2 || tensor.height!=2 || tensor.depth!=1 || tensor.dim!=1) 
    throw CImgArgumentException("CImg<%s>::draw_gaussian() : Tensor parameter must be a 2x2 matrix,"
                    "given is (%u,%u,%u,%u)",
                    pixel_type(),tensor.width,tensor.height,tensor.depth,tensor.dim);
      const CImg<t> invT = tensor.get_inverse(), invT2 = (invT*invT)/(-2.0);
      const t &a=invT2(0,0), &b=2*invT2(1,0), &c=invT2(1,1);
      const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
      const unsigned int whz = width*height*depth;
      const T *col = color;
      cimg_mapXY(*this,x,y) {
        const float dx = (x-xc), dy = (y-yc);
        const double val = std::exp(a*dx*dx + b*dx*dy + c*dy*dy);
        T *ptrd = ptr(x,y,0,0);
        if (opacity>=1) cimg_mapV(*this,k) { *ptrd = (T)(val*(*col++)); ptrd+=whz; }
        else cimg_mapV(*this,k) { *ptrd = (T)(nopacity*val*(*col++) + copacity*(*ptrd)); ptrd+=whz; }
        col-=dim;
      }
      return *this;
    }

    //! Draw an isotropic 2D gaussian function in the instance image
    /**
       \param xc = X-coordinate of the gaussian center.
       \param yc = Y-coordinate of the gaussian center.       
       \param sigma = standart variation of the gaussian distribution.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_gaussian(const float xc,const float yc,const float sigma,const T *const color,const float opacity=1) {
      return draw_gaussian(xc,yc,CImg<float>::diagonal(sigma,sigma),color,opacity);
    }
    
    //! Draw an anisotropic 3D gaussian function in the instance image.
    /**
       \param xc = X-coordinate of the gaussian center.
       \param yc = Y-coordinate of the gaussian center.
       \param zc = Z-coordinate of the gaussian center.
       \param tensor = 3x3 covariance matrix.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
    **/
    template<typename t> CImg& draw_gaussian(const float xc,const float yc,const float zc,const CImg<t>& tensor,
                         const T *const color,const float opacity=1) {
      cimg_test(*this,"CImg<T>::draw_gaussian");
      if (tensor.width!=3 || tensor.height!=3 || tensor.depth!=1 || tensor.dim!=1)
    throw CImgArgumentException("CImg<%s>::draw_gaussian() : Tensor parameter must be a 3x3 matrix,"
                    "given is (%u,%u,%u,%u)",
                    pixel_type(),tensor.width,tensor.height,tensor.depth,tensor.dim);
      const CImg<t> invT = tensor.get_inverse(), invT2 = (invT*invT)/(-2.0);
      const t a=invT(0,0), b=2*invT(1,0), c=2*invT(2,0), d=invT(1,1), e=2*invT(2,1), f=invT(2,2);
      const float nopacity = cimg::abs(opacity), copacity = 1-cimg::max(opacity,0.0f);
      const unsigned int whz = width*height*depth; 
      const T *col = color;
      cimg_mapXYZ(*this,x,y,z) {
        const float dx = (x-xc), dy = (y-yc), dz = (z-zc);
        const double val = std::exp(a*dx*dx + b*dx*dy + c*dx*dz + d*dy*dy + e*dy*dz + f*dz*dz);
        T *ptrd = ptr(x,y,z,0);
        if (opacity>=1) cimg_mapV(*this,k) { *ptrd = (T)(val*(*col++)); ptrd+=whz; }
        else cimg_mapV(*this,k) { *ptrd = (T)(nopacity*val*(*col++) + copacity*(*ptrd)); ptrd+=whz; }
        col-=dim;
      }
      return *this;
    }
    
    //! Draw an isotropic 3D gaussian function in the instance image
   /**
       \param xc = X-coordinate of the gaussian center.
       \param yc = Y-coordinate of the gaussian center.
       \param zc = Z-coordinate of the gaussian center.
       \param sigma = standart variation of the gaussian distribution.
       \param color = array of dimv() values of type \c T, defining the drawing color.
       \param opacity = opacity of the drawing.
    **/
    CImg& draw_gaussian(const float xc,const float yc,const float zc,
            const double sigma,const T *const color,const float opacity=1) {
      return draw_gaussian(xc,yc,zc,CImg<float>::diagonal(sigma,sigma,sigma),color,opacity);
    }
    
    //@}
    //---------------------------------------
    //---------------------------------------
    //
    //! \name Filtering functions
    //@{
    //---------------------------------------
    //---------------------------------------
  
    //! Return the correlation of the image by a mask.
    /**
       The result \p res of the correlation of an image \p img by a mask \p mask is defined to be :
       
       res(x,y,z) = sum_{i,j,k} img(x+i,y+j,z+k)*mask(i,j,k)

       \param mask = the correlation kernel.
       \param cond = the border condition type (0=zero, 1=dirichlet)
       \param weighted_correl = enable local normalization.
    **/
    template<typename t> CImg get_correlate(const CImg<t>& mask,const unsigned int cond=1,const bool weighted_correl=false) const {
      cimg_test_scalar(mask,"CImg<T>::get_correlate");
      CImg dest(*this,false);
      if (cond && mask.width==mask.height && ((mask.depth==1 && mask.width<=5) || (mask.depth==mask.width && mask.width<=3))) {
        // A special optimization is done for 2x2,3x3,4x4,5x5,2x2x2 and 3x3x3 mask (with cond=1)
        switch (mask.depth) {
        case 3: {
          CImg_3x3x3(I,T);
          if (!weighted_correl) cimg_mapZV(*this,z,v) cimg_map3x3x3(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_corr3x3x3(I,mask);
          else cimg_mapZV(*this,z,v) cimg_map3x3x3(*this,x,y,z,v,I) {
            const double norm = (double)cimg_squaresum3x3x3(I);
            dest(x,y,z,v) = (norm!=0)?(T)(cimg_corr3x3x3(I,mask)/std::sqrt(norm)):0;
          }
        } break;
        case 2: {
          CImg_2x2x2(I,T);
          if (!weighted_correl) cimg_mapZV(*this,z,v) cimg_map2x2x2(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_corr2x2x2(I,mask);
          else cimg_mapZV(*this,z,v) cimg_map2x2x2(*this,x,y,z,v,I) {
            const double norm = (double)cimg_squaresum2x2x2(I);
            dest(x,y,z,v) = (norm!=0)?(T)(cimg_corr2x2x2(I,mask)/std::sqrt(norm)):0;
          }
        } break;
        default:
        case 1:
          switch (mask.width) {
          case 5: {
            CImg_5x5(I,T);
            if (!weighted_correl) cimg_mapZV(*this,z,v) cimg_map5x5(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_corr5x5(I,mask);
            else cimg_mapZV(*this,z,v) cimg_map5x5(*this,x,y,z,v,I) {
              const double norm = (double)cimg_squaresum5x5(I);
              dest(x,y,z,v) = (norm!=0)?(T)(cimg_corr5x5(I,mask)/std::sqrt(norm)):0;
            }            
          } break;          
          case 4: {
            CImg_4x4(I,T);
            if (!weighted_correl) cimg_mapZV(*this,z,v) cimg_map4x4(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_corr4x4(I,mask);
            else cimg_mapZV(*this,z,v) cimg_map4x4(*this,x,y,z,v,I) {
              const double norm = (double)cimg_squaresum4x4(I);
              dest(x,y,z,v) = (norm!=0)?(T)(cimg_corr4x4(I,mask)/std::sqrt(norm)):0;
            }            
          } break;              
          case 3: {
            CImg_3x3(I,T);
            if (!weighted_correl) cimg_mapZV(*this,z,v) cimg_map3x3(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_corr3x3(I,mask);
            else cimg_mapZV(*this,z,v) cimg_map3x3(*this,x,y,z,v,I) {
              const double norm = (double)cimg_squaresum3x3(I);
              dest(x,y,z,v) = (norm!=0)?(T)(cimg_corr3x3(I,mask)/std::sqrt(norm)):0;
            }            
          } break;   
          case 2: {
            CImg_2x2(I,T);
            if (!weighted_correl) cimg_mapZV(*this,z,v) cimg_map2x2(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_corr2x2(I,mask);
            else cimg_mapZV(*this,z,v) cimg_map2x2(*this,x,y,z,v,I) {
              const double norm = (double)cimg_squaresum2x2(I);
              dest(x,y,z,v) = (norm!=0)?(T)(cimg_corr2x2(I,mask)/std::sqrt(norm)):0;
            }            
          } break;  
          case 1: dest = mask(0)*(*this); break;
          }
        }
      } else { 
        // Generic version for other masks      
        const int cxm=mask.width/2, cym=mask.height/2, czm=mask.depth/2, fxm=cxm-1+(mask.width%2), fym=cym-1+(mask.height%2), fzm=czm-1+(mask.depth%2);
        cimg_mapV(*this,v) 
          if (!weighted_correl) {    // Classical correlation
            for (int z=czm; z<dimz()-czm; z++) for (int y=cym; y<dimy()-cym; y++) for (int x=cxm; x<dimx()-cxm; x++) {
              double val = 0;
              for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++)
                val+= (*this)(x+xm,y+ym,z+zm,v)*mask(cxm+xm,cym+ym,czm+zm,0);
              dest(x,y,z,v)=(T)val;
            }
            if (cond) cimg_mapYZV(*this,y,z,v)
              for (int x=0; x<dimx(); (y<cym || y>=dimy()-cym || z<czm || z>=dimz()-czm)?x++:((x<cxm-1 || x>=dimx()-cxm)?x++:(x=dimx()-cxm))) {
                double val = 0;
                for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++)
                  val+= neumann_pix3d(x+xm,y+ym,z+zm,v)*mask(cxm+xm,cym+ym,czm+zm,0);
                dest(x,y,z,v)=(T)val;
              }
            else cimg_mapYZV(*this,y,z,v)
              for (int x=0; x<dimx(); (y<cym || y>=dimy()-cym || z<czm || z>=dimz()-czm)?x++:((x<cxm-1 || x>=dimx()-cxm)?x++:(x=dimx()-cxm))) {
                double val = 0;
                for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++)  for (int xm=-cxm; xm<=fxm; xm++)
                  val+= dirichlet_pix3d(x+xm,y+ym,z+zm,v,0)*mask(cxm+xm,cym+ym,czm+zm,0);
                dest(x,y,z,v)=(T)val;
              }
          } else {    // Weighted correlation
            for (int z=czm; z<dimz()-czm; z++) for (int y=cym; y<dimy()-cym; y++) for (int x=cxm; x<dimx()-cxm; x++) {
              double val = 0, norm = 0;
              for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++) {
                const T cval = (*this)(x+xm,y+ym,z+zm,v);
                val+= cval*mask(cxm+xm,cym+ym,czm+zm,0);
                norm+= cval*cval;
              }
              dest(x,y,z,v)=(norm!=0)?(T)(val/std::sqrt(norm)):0;
            }
            if (cond) cimg_mapYZV(*this,y,z,v)
              for (int x=0; x<dimx(); (y<cym || y>=dimy()-cym || z<czm || z>=dimz()-czm)?x++:((x<cxm-1 || x>=dimx()-cxm)?x++:(x=dimx()-cxm))) {
                double val = 0, norm = 0;
                for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++) {
                  const T cval = neumann_pix3d(x+xm,y+ym,z+zm,v);
                  val+= cval*mask(cxm+xm,cym+ym,czm+zm,0);
                  norm+=cval*cval;
                }
                dest(x,y,z,v)=(norm!=0)?(T)(val/std::sqrt(norm)):0;
              }
            else cimg_mapYZV(*this,y,z,v)
              for (int x=0; x<dimx(); (y<cym || y>=dimy()-cym || z<czm || z>=dimz()-czm)?x++:((x<cxm-1 || x>=dimx()-cxm)?x++:(x=dimx()-cxm))) {
                double val = 0, norm = 0;
                for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++) {
                  const T cval = dirichlet_pix3d(x+xm,y+ym,z+zm,v,0);
                  val+= cval*mask(cxm+xm,cym+ym,czm+zm,0);
                  norm+= cval*cval;
                }
                dest(x,y,z,v)=(norm!=0)?(T)(val/std::sqrt(norm)):0;
              }
          }
      }
      return dest;
    }
    //! Correlate the image by a mask
    /**
       This is the in-place version of get_correlate.
       \see get_correlate
    **/
    template<typename t> CImg& correlate(const CImg<t>& mask,const unsigned int cond=1,const bool weighted_correl=false) { 
      return get_correlate(mask,cond,weighted_correl).swap(*this); 
    }
  
    //! Return the convolution of the image by a mask
    /**
       The result \p res of the convolution of an image \p img by a mask \p mask is defined to be :
       
       res(x,y,z) = sum_{i,j,k} img(x-i,y-j,z-k)*mask(i,j,k)

       \param mask = the correlation kernel.
       \param cond = the border condition type (0=zero, 1=dirichlet)
       \param weighted_convol = enable local normalization.
    **/
    template<typename t> CImg get_convolve(const CImg<t>& mask,const unsigned int cond=1,const bool weighted_convol=false) const {
      cimg_test_scalar(mask,"CImg<T>::get_convolve");
      CImg dest(*this,false);
      if (cond && mask.width==mask.height && ((mask.depth==1 && mask.width<=5) || (mask.depth==mask.width && mask.width<=3))) { // optimized version
        switch (mask.depth) {
        case 3: {
          CImg_3x3x3(I,T);
          if (!weighted_convol) cimg_mapZV(*this,z,v) cimg_map3x3x3(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_conv3x3x3(I,mask);
          else cimg_mapZV(*this,z,v) cimg_map3x3x3(*this,x,y,z,v,I) {
            const double norm = (double)cimg_squaresum3x3x3(I);
            dest(x,y,z,v) = (norm!=0)?(T)(cimg_conv3x3x3(I,mask)/std::sqrt(norm)):(T)0;
          }
        } break;
        case 2: {
          CImg_2x2x2(I,T);
          if (!weighted_convol) cimg_mapZV(*this,z,v) cimg_map2x2x2(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_conv2x2x2(I,mask);
          else cimg_mapZV(*this,z,v) cimg_map2x2x2(*this,x,y,z,v,I) {
            const double norm = (double)cimg_squaresum2x2x2(I);
            dest(x,y,z,v) = (norm!=0)?(T)(cimg_conv2x2x2(I,mask)/std::sqrt(norm)):(T)0;
          }
        } break;
        default:
        case 1:
          switch (mask.width) {
          case 5: {
            CImg_5x5(I,T);
            if (!weighted_convol) cimg_mapZV(*this,z,v) cimg_map5x5(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_conv5x5(I,mask);
            else cimg_mapZV(*this,z,v) cimg_map5x5(*this,x,y,z,v,I) {
              const double norm = (double)cimg_squaresum5x5(I);
              dest(x,y,z,v) = (norm!=0)?(T)(cimg_conv5x5(I,mask)/std::sqrt(norm)):(T)0;
            }            
          } break;          
          case 4: {
            CImg_4x4(I,T);
            if (!weighted_convol) cimg_mapZV(*this,z,v) cimg_map4x4(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_conv4x4(I,mask);
            else cimg_mapZV(*this,z,v) cimg_map4x4(*this,x,y,z,v,I) {
              const double norm = (double)cimg_squaresum4x4(I);
              dest(x,y,z,v) = (norm!=0)?(T)(cimg_conv4x4(I,mask)/std::sqrt(norm)):(T)0;
            }
          } break;              
          case 3: {
            CImg_3x3(I,T);
            if (!weighted_convol) cimg_mapZV(*this,z,v) cimg_map3x3(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_conv3x3(I,mask);
            else cimg_mapZV(*this,z,v) cimg_map3x3(*this,x,y,z,v,I) {
              const double norm = (double)cimg_squaresum3x3(I);
              dest(x,y,z,v) = (norm!=0)?(T)(cimg_conv3x3(I,mask)/std::sqrt(norm)):(T)0;
            }            
          } break;   
          case 2: {
            CImg_2x2(I,T);
            if (!weighted_convol) cimg_mapZV(*this,z,v) cimg_map2x2(*this,x,y,z,v,I) dest(x,y,z,v) = (T)cimg_conv2x2(I,mask);
            else cimg_mapZV(*this,z,v) cimg_map2x2(*this,x,y,z,v,I) {
              const double norm = (double)cimg_squaresum2x2(I);
              dest(x,y,z,v) = (norm!=0)?(T)(cimg_conv2x2(I,mask)/std::sqrt(norm)):(T)0;
            } 
          } break;  
          case 1: dest = mask(0)*(*this); break;
          }
        }
      } else { // generic version
          
        const int cxm=mask.width/2, cym=mask.height/2, czm=mask.depth/2, fxm=cxm-1+(mask.width%2), fym=cym-1+(mask.height%2), fzm=czm-1+(mask.depth%2);
        cimg_mapV(*this,v) 
          if (!weighted_convol) {    // Classical convolution
            for (int z=czm; z<dimz()-czm; z++) for (int y=cym; y<dimy()-cym; y++) for (int x=cxm; x<dimx()-cxm; x++) {
              double val = 0;
              for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++)
                val+= (*this)(x-xm,y-ym,z-zm,v)*mask(cxm+xm,cym+ym,czm+zm,0);
              dest(x,y,z,v)=(T)val;
            }
            if (cond) cimg_mapYZV(*this,y,z,v)
              for (int x=0; x<dimx(); (y<cym || y>=dimy()-cym || z<czm || z>=dimz()-czm)?x++:((x<cxm-1 || x>=dimx()-cxm)?x++:(x=dimx()-cxm))) {
                double val = 0;
                for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++)
                  val+= neumann_pix3d(x-xm,y-ym,z-zm,v)*mask(cxm+xm,cym+ym,czm+zm,0);
                dest(x,y,z,v)=(T)val;
              }
            else cimg_mapYZV(*this,y,z,v)
              for (int x=0; x<dimx(); (y<cym || y>=dimy()-cym || z<czm || z>=dimz()-czm)?x++:((x<cxm-1 || x>=dimx()-cxm)?x++:(x=dimx()-cxm))) {
                double val = 0;
                for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++)  for (int xm=-cxm; xm<=fxm; xm++)
                  val+= dirichlet_pix3d(x-xm,y-ym,z-zm,v,0)*mask(cxm+xm,cym+ym,czm+zm,0);
                dest(x,y,z,v)=(T)val;
              }
          } else {    // Weighted convolution
            for (int z=czm; z<dimz()-czm; z++) for (int y=cym; y<dimy()-cym; y++) for (int x=cxm; x<dimx()-cxm; x++) {
              double val = 0, norm = 0;
              for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++) {
                const T cval = (*this)(x-xm,y-ym,z-zm,v);
                val+= cval*mask(cxm+xm,cym+ym,czm+zm,0);
                norm+= cval*cval;
              }
              dest(x,y,z,v)=(norm!=0)?(T)(val/std::sqrt(norm)):(T)0;
            }
            if (cond) cimg_mapYZV(*this,y,z,v)
              for (int x=0; x<dimx(); (y<cym || y>=dimy()-cym || z<czm || z>=dimz()-czm)?x++:((x<cxm-1 || x>=dimx()-cxm)?x++:(x=dimx()-cxm))) {
                double val = 0, norm = 0;
                for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++) for (int xm=-cxm; xm<=fxm; xm++) {
                  const T cval = neumann_pix3d(x-xm,y-ym,z-zm,v);
                  val+= cval*mask(cxm+xm,cym+ym,czm+zm,0);
                  norm+=cval*cval;
                }
                dest(x,y,z,v)=(norm!=0)?(T)(val/std::sqrt(norm)):(T)0;
              }
            else cimg_mapYZV(*this,y,z,v)
              for (int x=0; x<dimx(); (y<cym || y>=dimy()-cym || z<czm || z>=dimz()-czm)?x++:((x<cxm-1 || x>=dimx()-cxm)?x++:(x=dimx()-cxm))) {
                double val = 0, norm = 0;
                for (int zm=-czm; zm<=fzm; zm++) for (int ym=-cym; ym<=fym; ym++)  for (int xm=-cxm; xm<=fxm; xm++) {
                  const T cval = dirichlet_pix3d(x-xm,y-ym,z-zm,v,0);
                  val+= cval*mask(cxm+xm,cym+ym,czm+zm,0);
                  norm+= cval*cval;
                }
                dest(x,y,z,v)=(norm!=0)?(T)(val/std::sqrt(norm)):(T)0;
              }
          }
      }
      return dest;
    }
  
    //! Convolve the image by a mask
    /**
       This is the in-place version of get_convolve().
       \see get_convolve()
    **/
    template<typename t> CImg& convolve(const CImg<t>& mask,const unsigned int cond=1,const bool weighted_convol=false) {
      return get_convolve(mask,cond,weighted_convol).swap(*this); 
    }

    //! Add noise to the image
    /**
       This is the in-place version of get_noise.
       \see get_noise.
    **/
    CImg& noise(const double sigma=-20,const unsigned int ntype=0) {
      cimg_test(*this,"CImg<T>::noise");
      T tmp;
      double nsigma = sigma, max = (double)cimg::get_type_max(tmp), min = (double)cimg::get_type_min(tmp);
      static bool first_time = true;
      if (first_time) { std::srand((unsigned int)::time(NULL)); first_time = false; }
      CImgStats st;
      if (nsigma==0) return *this;
      if (nsigma<0 || ntype==2) st = CImgStats(*this,false);
      if (nsigma<0) nsigma = -nsigma*(st.max-st.min)/100.0;
      switch (ntype) {
      case 0: { // Gaussian noise
    cimg_map(*this,ptr,T) {
      double val = *ptr+nsigma*cimg::grand(); 
      if (val>max) val = max;
      if (val<min) val = min;
      *ptr = (T)val;
    }
      } break;
      case 1: { // Uniform noise
    cimg_map(*this,ptr,T) {
      double val = *ptr+nsigma*cimg::crand();
      if (val>max) val = max;
      if (val<min) val = min;
      *ptr = (T)val;
    }
      } break;
      case 2: { // Salt & Pepper noise
        if (st.max==st.min) { st.min=0; st.max=255; }
        cimg_map(*this,ptr,T) if (cimg::rand()*100<nsigma) *ptr=(T)(cimg::rand()<0.5?st.max:st.min);
      } break;
      }
      return *this;
    }
    
    //! Return a noisy image
    /**
       \param sigma = power of the noise. if sigma<0, it corresponds to the percentage of the maximum image value.
       \param ntype = noise type. can be 0=gaussian, 1=uniform or 2=Salt and Pepper.
       \return A noisy version of the instance image.
    **/
    CImg get_noise(const double sigma=-20,const unsigned int ntype=0) const { return CImg<T>(*this).noise(sigma,ntype); }


#define cimg_deriche_map(x0,y0,z0,k0,nb,offset,T) {                           \
    ima = ptr(x0,y0,z0,k0);                                                   \
    I2 = *ima; ima+=offset; I1 = *ima; ima+=offset;                           \
    Y2 = *(Y++) = sumg0*I2; Y1 = *(Y++) = g0*I1 + sumg1*I2;                   \
    for (i=2; i<(nb); i++) { I1 = *ima; ima+=offset;                          \
        Y0 = *(Y++) = a1*I1 + a2*I2 + b1*Y1 + b2*Y2;                          \
        I2=I1; Y2=Y1; Y1=Y0; }                                                \
    ima-=offset; I2 = *ima; Y2 = Y1 = parity*sumg1*I2; *ima = (T)(*(--Y)+Y2); \
    ima-=offset; I1 = *ima; *ima = (T)(*(--Y)+Y1);                            \
    for (i=(nb)-3; i>=0; i--) { Y0=a3*I1+a4*I2+b1*Y1+b2*Y2; ima-=offset;      \
      I2=I1; I1=*ima; *ima=(T)(*(--Y)+Y0); Y2=Y1; Y1=Y0; }                    \
  }

    //! Apply a deriche filter on the image
    /**
       This is the in-place version of get_deriche
       \see get_deriche.
    **/
    CImg& deriche(const float sigma=1,const int order=0,const char axe='x',const unsigned int cond=1) {
      cimg_test(*this,"CImg<T>::deriche");
      if (sigma<0 || order<0 || order>2)
    throw CImgArgumentException("CImg<%s>::deriche() : Bad arguments (sigma=%g, order=%d)",pixel_type(),sigma,order);
      const float alpha=sigma>0?(1.695f/sigma):20,ea=(float)std::exp(alpha),ema=(float)std::exp(-alpha),em2a=ema*ema,b1=2*ema,b2=-em2a;
      float ek,ekn,parity,a1,a2,a3,a4,g0,sumg1,sumg0;
      double *Y,Y0,Y1,Y2;
      int i,offset,nb;
      T *ima,I1,I2;
      switch(order) {
      case 1:                 // first derivative
        ek = -(1-ema)*(1-ema)*(1-ema)/(2*(ema+1)*ema); a1 = a4 = 0;  a2 = ek*ema; a3 = -ek*ema; parity =-1;\
        if (cond) { sumg1 = (ek*ea) / ((ea-1)*(ea-1)); g0 = 0; sumg0 = g0+sumg1; } \
        else g0 = sumg0 = sumg1 = 0;
        break;
      case 2:               // second derivative
        ekn = ( -2*(-1+3*ea-3*ea*ea+ea*ea*ea)/(3*ea+1+3*ea*ea+ea*ea*ea) );
        ek = -(em2a-1)/(2*alpha*ema); a1 = ekn;  a2 = -ekn*(1+ek*alpha)*ema; a3 = ekn*(1-ek*alpha)*ema; a4 = -ekn*em2a; parity =1;
        if (cond) { sumg1 = ekn/2; g0 = ekn; sumg0 = g0+sumg1; }
        else g0=sumg0=sumg1=0;
        break;
      default:              // smoothing
        ek = (1-ema)*(1-ema) / (1+2*alpha*ema - em2a); a1 = ek;  a2 = ek*ema*(alpha-1); a3 = ek*ema*(alpha+1); a4 = -ek*em2a; parity = 1;
        if (cond) { sumg1 = ek*(alpha*ea+ea-1) / ((ea-1)*(ea-1)); g0 = ek; sumg0 = g0+sumg1; }
        else  g0=sumg0=sumg1=0;
        break;
      }
      // filter init
      Y = new double[cimg::max(width,height,depth)];
      switch(cimg::uncase(axe)) {
      case 'x': if (width>1)  { offset = 1;            nb = width;  cimg_mapYZV(*this,y,z,k) cimg_deriche_map(0,y,z,k,nb,offset,T); }    break;
      case 'y': if (height>1) { offset = width;        nb = height; cimg_mapXZV(*this,x,z,k) cimg_deriche_map(x,0,z,k,nb,offset,T); }    break;
      case 'z': if (depth>1)  { offset = width*height; nb = depth;  cimg_mapXYV(*this,x,y,k) cimg_deriche_map(x,y,0,k,nb,offset,T); }    break;
      default: throw CImgArgumentException("CImg<%s>::deriche() : unknow axe '%c', must be 'x','y' or 'z'",pixel_type(),axe);
      }
      delete[] Y;
      return *this;
    }
    //! Return the result of the Deriche filter
    /**
       The Canny-Deriche filter is a recursive algorithm allowing to compute blurred derivatives of
       order 0,1 or 2 of an image.
       \see blur
    **/
    CImg get_deriche(const float sigma=1,const int order=0,const char axe='x',const unsigned int cond=1) const {
      return CImg<T>(*this).deriche(sigma,order,axe,cond);
    }

    //! Blur the image with a Deriche filter (anisotropically)
    /**
       This is the in-place version of get_blur().
       \see get_blur().
    **/
    CImg& blur(const float sigmax,const float sigmay,const float sigmaz,const unsigned int cond=1) {
      cimg_test(*this,"CImg<T>::blur");
      if (width>1  && sigmax>0) deriche(sigmax,0,'x',cond);
      if (height>1 && sigmay>0) deriche(sigmay,0,'y',cond);
      if (depth>1  && sigmaz>0) deriche(sigmaz,0,'z',cond);
      return *this;
    }

    //! Blur the image with a Deriche filter (quasi-gaussian filter)
    CImg& blur(const float sigma=1,const unsigned int cond=1) { return blur(sigma,sigma,sigma,cond); }

    //! Return a blurred version of the image, using a Deriche filter (quasi gaussian filter)
    /**
       Blur the image with an anisotropic exponential filter (Deriche filter of order 0).
    **/
    CImg get_blur(const float sigmax,const float sigmay,const float sigmaz,const unsigned int cond=1) const {
      return CImg<T>(*this).blur(sigmax,sigmay,sigmaz,cond); 
    }
    
    //! Return a blurred version of the image, using a Deriche filter (quasi gaussian filter)
    CImg get_blur(const float sigma=1,const unsigned int cond=1) const { return CImg<T>(*this).blur(sigma,cond); }

    //! Blur an image following a field of diffusion tensors.
    template<typename t> 
    CImg& blur_anisotropic(const CImg<t>& G, const float amplitude=10.0f, const float dl=0.8f,const float da=45.0f,
               const float gauss_prec=2.0f, const bool linear=false) {
      
      // Check arguments and init variables
      cimg_test(*this,"blur_anisotropic");
      cimg_test(G,"blur_anisotropic");
      if ((G.dim!=3 && G.dim!=6) || G.width!=width || G.height!=height || G.depth!=depth)
    throw CImgArgumentException("CImg<%s>::blur_anisotropic() : Input tensor field G has wrong dimensions (%u,%u,%u,%u).",
                pixel_type(),G.width,G.height,G.depth,G.dim);
      
      const int dx1 = dimx()-1, dy1 = dimy()-1, dz1 = dimz()-1;
      const bool threed = (G.dim>=6);
      CImg<t> dest(width,height,depth,dim,0), tmp(dim), W(width,height,depth,threed?3:2);
      int N = 0;
      
      if (threed)
    // 3D version of the algorithm
    //-----------------------------
    for (float phi=(180%(int)da)/2.0f; phi<=180; phi+=da) {
      const float phir = phi*cimg::PI/180, datmp = da/std::cos(phir), da2 = datmp<1?360:datmp;
      for (float theta=0; theta<360; (theta+=da2),N++) {
        const float thetar = theta*cimg::PI/180,
          vx = (float)std::cos(thetar)*std::cos(phir),
          vy = (float)std::sin(thetar)*std::cos(phir),
          vz = (float)std::sin(phir);
        const t 
          *pa = G.ptr(0,0,0,0), *pb = G.ptr(0,0,0,1), *pc = G.ptr(0,0,0,2),
          *pd = G.ptr(0,0,0,3), *pe = G.ptr(0,0,0,4), *pf = G.ptr(0,0,0,5);
        t *pd0 = W.ptr(0,0,0,0), *pd1 = W.ptr(0,0,0,1), *pd2 = W.ptr(0,0,0,2);
        cimg_mapXYZ(G,xg,yg,zg) {
          const t
        a = *(pa++), b = *(pb++), c = *(pc++),
        d = *(pd++), e = *(pe++), f = *(pf++),
        u = a*vx + b*vy + c*vz,
        v = b*vx + d*vy + e*vz,
        w = c*vx + e*vy + f*vz,
        n = dl/std::sqrt(1e-5+u*u+v*v+w*w);
          *(pd0++) = u*n;
          *(pd1++) = v*n;
          *(pd2++) = w*n;
        }

        cimg_mapXYZ(*this,x,y,z) {
          tmp.fill(0);
          const t cu = W(x,y,z,0), cv = W(x,y,z,1), cw = W(x,y,z,2);
          const float
        fsigma = (float)(2*std::sqrt((cu*cu+cv*cv+cw*cw)*amplitude)),
        length = gauss_prec*fsigma,
        fsigma2 = 2*fsigma*fsigma;
          float l, S=0, pu=cu, pv=cv, pw=cw, X=(float)x, Y=(float)y, Z=z+cw;
          if (linear) for (l=0; l<length; l+=dl) {
        const float coef = (float)std::exp(-l*l/fsigma2);
        t u = (t)(W.linear_pix3d(X,Y,Z,0)), v = (t)(W.linear_pix3d(X,Y,Z,1)), w = (t)(W.linear_pix3d(X,Y,Z,2));
        if ((pu*u+pv*v+pw*w)<0) { u=-u; v=-v; w=-w; }
        cimg_mapV(*this,k) tmp[k]+=(t)(coef*linear_pix3d(X,Y,Z,k));
        X+=(pu=u); Y+=(pv=v); Z+=(pw=w); S+=coef;
          } else for (l=0; l<length; l+=dl) {
        const float 
          coef = (float)std::exp(-l*l/fsigma2),
          Xn = X<0?0:(X>=dx1?dx1:X),
          Yn = Y<0?0:(Y>=dy1?dy1:Y),
          Zn = Z<0?0:(Z>=dz1?dz1:Z);
        const int xi = (int)(Xn+0.5f), yi = (int)(Yn+0.5f), zi = (int)(Zn+0.5f);
        t u = W(xi,yi,zi,0), v = W(xi,yi,zi,1), w = W(xi,yi,zi,2);
        if ((pu*u+pv*v+pw*w)<0) { u=-u; v=-v; w=-w; }
        cimg_mapV(*this,k) tmp[k]+=(t)(coef*(*this)(xi,yi,zi,k));
        X+=(pu=u); Y+=(pv=v); Z+=(pw=w); S+=coef;
          }
          if (S>0) cimg_mapV(dest,k) dest(x,y,z,k)+=tmp[k]/S;
          else cimg_mapV(dest,k) dest(x,y,z,k)+=(t)((*this)(x,y,z,k));
        }
      }
    } else
      // 2D version of the algorithm
      //-----------------------------
      for (float theta=(360%(int)da)/2.0f; theta<360; (theta+=da),N++) {
        const float thetar = theta*cimg::PI/180, vx = std::cos(thetar), vy = std::sin(thetar);

        const t *pa = G.ptr(0,0,0,0), *pb = G.ptr(0,0,0,1), *pc = G.ptr(0,0,0,2);
        t *pd0 = W.ptr(0,0,0,0), *pd1 = W.ptr(0,0,0,1);
        cimg_mapXY(G,xg,yg) {
          const t
        a = *(pa++), b = *(pb++), c = *(pc++), 
        u = a*vx + b*vy, v = b*vx + c*vy,
        n = dl/std::sqrt(1e-5+u*u+v*v);
          *(pd0++) = u*n;
          *(pd1++) = v*n;
        }
    
        cimg_mapXY(*this,x,y) {
          tmp.fill(0);
          const t cu = W(x,y,0,0), cv = W(x,y,0,1);
          const float
        fsigma = (float)(2*std::sqrt((cu*cu+cv*cv)*amplitude)),
        length = gauss_prec*fsigma,
        fsigma2 = 2*fsigma*fsigma;
          float l, S=0, pu=cu, pv=cv, X=(float)x, Y=(float)y;
          if (linear) for (l=0; l<length; l+=dl) {
        const float coef = (float)std::exp(-l*l/fsigma2);
        t u = (t)(W.linear_pix2d(X,Y,0,0)), v = (t)(W.linear_pix2d(X,Y,0,1));
        if ((pu*u+pv*v)<0) { u=-u; v=-v; }
        cimg_mapV(*this,k) tmp[k]+=(t)(coef*linear_pix2d(X,Y,0,k));
        X+=(pu=u); Y+=(pv=v); S+=coef;
          } else for (l=0; l<length; l+=dl) {
        const float 
          coef = (float)std::exp(-l*l/fsigma2),
          Xn = X<0?0:(X>=dx1?dx1:X),
          Yn = Y<0?0:(Y>=dy1?dy1:Y);
        const int xi = (int)(Xn+0.5f), yi = (int)(Yn+0.5f);
        t u = W(xi,yi,0,0), v = W(xi,yi,0,1);
        if ((pu*u+pv*v)<0) { u=-u; v=-v; }
        cimg_mapV(*this,k) tmp[k]+=(t)(coef*(*this)(xi,yi,0,k));
        X+=(pu=u); Y+=(pv=v); S+=coef;
          }
          if (S>0) cimg_mapV(dest,k) dest(x,y,0,k)+=tmp[k]/S;
          else cimg_mapV(dest,k) dest(x,y,0,k)+=(t)((*this)(x,y,0,k));
        }
      }
      const float *ptrs = dest.data+dest.size(); cimg_map(*this,ptrd,T) *ptrd = (T)(*(--ptrs)/N);
      return *this;
    }

    //! Blur an image following a field of diffusion tensors.
    /**
       \param G = Field of square roots of diffusion tensors used to drive the smoothing.
       \param amplitude = amplitude of the smoothing.
       \param dl = spatial discretization.
       \param da = angular discretization.
       \param gauss_prec = precision of the gaussian function.
       \param linear = enable/disable linear interpolation
    **/
    template<typename t>
    CImg get_blur_anisotropic(const CImg<t>& G, const float amplitude=10.0f, const float dl=0.8f,const float da=45.0f,
                  const float gauss_prec=2.0f, const bool linear=false) const {
      return CImg<T>(*this).blur_anisotropic(G,amplitude,dl,da,gauss_prec,linear);
    }

    //! Blur an image following a field of diffusion tensors.
    CImg& blur_anisotropic(const float amplitude, const float sharpness=1.5f, const float anisotropy=0.7f,
               const float alpha=0.2f,const float sigma=0.8f, const float dl=0.8f,const float da=45.0f,
               const float gauss_prec=2.0f, const bool linear=false) {
  
      cimg_test(*this,"blur_anisotropic");
      const bool threed = (depth>1);
      CImg<float> G(width,height,depth,(threed?6:3),0);
      const float power1 = 0.5f*sharpness, power2 = (1+anisotropy)/(1e-10+1-anisotropy)*power1;
      float nmax = 0;

      if (threed) { // Field for 3D volumes    
    CImg<float> val(3),vec(3,3);
    CImg_3x3x3(I,float);
    CImg<T> blurred = get_blur(alpha);
    cimg_mapV(*this,k) cimg_map3x3x3(blurred,x,y,z,k,I) {
      const float 
        ixf = Incc-Iccc, iyf = Icnc-Iccc, izf = Iccn-Iccc,
        ixb = Iccc-Ipcc, iyb = Iccc-Icpc, izb = Iccc-Iccp;
      G(x,y,z,0) += 0.5f*(ixf*ixf + ixb*ixb);
      G(x,y,z,1) += 0.25f*(ixf*iyf + ixf*iyb + ixb*iyf + ixb*iyb);
      G(x,y,z,2) += 0.25f*(ixf*izf + ixf*izb + ixb*izf + ixb*izb);
      G(x,y,z,3) += 0.5f*(iyf*iyf + iyb*iyb);
      G(x,y,z,4) += 0.25f*(iyf*izf + iyf*izb + iyb*izf + iyb*izb);
      G(x,y,z,5) += 0.5f*(izf*izf + izb*izb);
    }
    G.blur(sigma);
    cimg_mapXYZ(*this,x,y,z) {
      G.get_tensor(x,y,z).symeigen(val,vec);
      const float l1 = val[2], l2 = val[1], l3 = val[0],
        ux = vec(2,0), uy = vec(2,1), uz = vec(2,2),
        vx = vec(1,0), vy = vec(1,1), vz = vec(1,2),
        wx = vec(0,0), wy = vec(0,1), wz = vec(0,2),
        n1 = (float)(1.0/std::pow(1.0f+l1+l2+l3,power1)),
        n2 = (float)(1.0/std::pow(1.0f+l1+l2+l3,power2)),
        nm = cimg::max(n1,n2);
      G(x,y,z,0) = n1*(ux*ux + vx*vx) + n2*wx*wx;
      G(x,y,z,1) = n1*(ux*uy + vx*vy) + n2*wx*wy;
      G(x,y,z,2) = n1*(ux*uz + vx*vz) + n2*wx*wz;
      G(x,y,z,3) = n1*(uy*uy + vy*vy) + n2*wy*wy;
      G(x,y,z,4) = n1*(uy*uz + vy*vz) + n2*wy*wz;
      G(x,y,z,5) = n1*(uz*uz + vz*vz) + n2*wz*wz;
      if (nm>nmax) nmax = nm;
    }
      } else { // Field for 2D images
    CImg<float> val(2),vec(2,2);
    CImg_3x3(I,float);
    CImg<T> blurred = get_blur(alpha);
    cimg_mapV(*this,k) cimg_map3x3(blurred,x,y,0,k,I) {
      const float
        ixf = Inc-Icc, iyf = Icn-Icc,
        ixb = Icc-Ipc, iyb = Icc-Icp;
      G(x,y,0,0) += 0.5f*(ixf*ixf+ixb*ixb);
      G(x,y,0,1) += 0.25f*(ixf*iyf+ixf*iyb+ixb*iyf+ixb*iyb);
      G(x,y,0,2) += 0.5f*(iyf*iyf+iyb*iyb);
    }
    G.blur(sigma);
    cimg_mapXY(*this,x,y) {
      G.get_tensor(x,y).symeigen(val,vec);
      const float l1 = val[0], l2 = val[1],
        ux = vec[0], uy = vec[1],
        vx = vec[2], vy = vec[3],
        n1 = (float)(1.0/std::pow(1.0f+l1+l2,power1)),
        n2 = (float)(1.0/std::pow(1.0f+l1+l2,power2)),
        nm = cimg::max(n1,n2);
      G(x,y,0,0) = n1*ux*ux + n2*vx*vx;
      G(x,y,0,1) = n1*ux*uy + n2*vx*vy;
      G(x,y,0,2) = n1*uy*uy + n2*vy*vy;
      if (nm>nmax) nmax = nm;
    }
      }
      G/=nmax;
      return blur_anisotropic(G,amplitude,dl,da,gauss_prec,linear);
    }

    //! Blur an image following a field of diffusion tensors.
    /**
       \param amplitude = amplitude of the smoothing.
       \param sharpness = define the contour preservation.
       \param anisotropy = define the smoothing anisotropy.
       \param sigma = regularity of the geometry.
       \param dl = spatial discretization.
       \param da = angular discretization.
       \param gauss_prec = precision of the gaussian function.
       \param linear = enable/disable linear interpolation
    **/
    CImg get_blur_anisotropic(const float amplitude, const float sharpness=1.5f, const float anisotropy=0.7f,
                  const float alpha=0.2f, const float sigma=0.8f, const float dl=0.8f,
                  const float da=45.0f, const float gauss_prec=2.0f, const bool linear=false) const {
  
      return CImg<T>(*this).blur_anisotropic(amplitude,sharpness,anisotropy,alpha,sigma,dl,da,gauss_prec,linear);
    }

    //! Return a eroded image (\p times erosion).
    CImg get_erode(const unsigned int n=1) {
      CImg_3x3x3(I,T);
      if (n==1) {
        CImg dest(*this);
        cimg_mapV(*this,k) cimg_map3x3x3(*this,x,y,z,k,I) 
      if (Iccc && (!Incc || !Ipcc || !Icnc || !Icpc || !Iccn || !Iccp)) dest(x,y,z,k) = 0;
        return dest;
      }
      CImg img1(*this),img2(*this,false);
      CImg *src = &img1, *dest = &img2, *tmp = NULL;
      for (unsigned int iter=0; iter<n; iter++) {
        *dest = *src;
        cimg_mapV(*src,k) cimg_map3x3x3(*src,x,y,z,k,I) 
      if (Iccc && (!Incc || !Ipcc || !Icnc || !Icpc || !Iccn || !Iccp)) (*dest)(x,y,z,k) = 0;
        tmp = src;
        src = dest;
        dest = tmp;
      }
      return *src;      
    }
    
    //! Return the Fast Fourier Transform of an image (along a specified axis)
    CImgl<T> get_FFT(const char axe,const bool inverse=false) const {
      return CImgl<T>(*this,CImg<T>(width,height,depth,dim,0)).FFT(axe,inverse);      
    }

    //! Return the Fast Fourier Transform on an image
    CImgl<T> get_FFT(const bool inverse=false) const {
      return CImgl<T>(*this,CImg<T>(width,height,depth,dim,0)).FFT(inverse);      
    }

    //! Erode the image \p n times.
    CImg& erode(const unsigned int n=1) { return get_erode(n).swap(*this); }

    //! Return an dilated image (\p times dilatation).
    CImg get_dilate(const unsigned int n=1) {
      CImgStats stats(*this);
      const T tmax = stats.max!=0?(T)stats.max:(T)1;
      CImg_3x3x3(I,T);
      if (n==1) {
        CImg dest(*this);
        cimg_mapV(*this,k) cimg_map3x3x3(*this,x,y,z,k,I) 
      if (!Iccc && (Incc || Ipcc || Icnc || Icpc || Iccn || Iccp)) dest(x,y,z,k) = tmax;
        return dest;
      }
      CImg img1(*this),img2(*this,false);
      CImg *src = &img1, *dest = &img2, *tmp = NULL;
      for (unsigned int iter=0; iter<n; iter++) {
        *dest = *src;
        cimg_mapV(*src,k) cimg_map3x3x3(*src,x,y,z,k,I) 
      if (!Iccc && (Incc || Ipcc || Icnc || Icpc || Iccn || Iccp)) (*dest)(x,y,z,k) = tmax;
        tmp = src;
        src = dest;
        dest = tmp;
      }
      return *src;      
    }
    //! Dilate the image \p n times.
    CImg& dilate(const unsigned int n=1) { return get_dilate(n).swap(*this); }

    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name Matrix and vector computation
    //@{
    //------------------------------------------
    //------------------------------------------

    //! Return a vector with specified coefficients
    static CImg vector(const T& a1) { return CImg<T>(1,1).fill(a1); }
    static CImg vector(const T& a1,const T& a2) { return CImg<T>(1,2).fill(a1,a2); }
    static CImg vector(const T& a1,const T& a2,const T& a3) { return CImg<T>(1,3).fill(a1,a2,a3); }
    static CImg vector(const T& a1,const T& a2,const T& a3,const T& a4) { return CImg<T>(1,4).fill(a1,a2,a3,a4); }
    static CImg vector(const T& a1,const T& a2,const T& a3,const T& a4,const T& a5) { return CImg<T>(1,5).fill(a1,a2,a3,a4,a5); }
    static CImg vector(const T& a1,const T& a2,const T& a3,const T& a4,const T& a5,const T& a6) { return CImg<T>(1,6).fill(a1,a2,a3,a4,a5,a6); }
    static CImg vector(const T& a1,const T& a2,const T& a3,const T& a4,
               const T& a5,const T& a6,const T& a7) { return CImg<T>(1,7).fill(a1,a2,a3,a4,a5,a6,a7); }
    static CImg vector(const T& a1,const T& a2,const T& a3,const T& a4,
               const T& a5,const T& a6,const T& a7,const T& a8) { return CImg<T>(1,8).fill(a1,a2,a3,a4,a5,a6,a7,a8); }
    static CImg vector(const T& a1,const T& a2,const T& a3,const T& a4,
               const T& a5,const T& a6,const T& a7,const T& a8,const T& a9) { return CImg<T>(1,9).fill(a1,a2,a3,a4,a5,a6,a7,a8,a9); }

    //! Return a square matrix with specified coefficients
    static CImg matrix(const T& a1) { return vector(a1); }
    static CImg matrix(const T& a1,const T& a2,
               const T& a3,const T& a4) { return CImg<T>(2,2).fill(a1,a2,a3,a4); }
    static CImg matrix(const T& a1,const T& a2,const T& a3,
               const T& a4,const T& a5,const T& a6,
               const T& a7,const T& a8,const T& a9) { return CImg<T>(3,3).fill(a1,a2,a3,a4,a5,a6,a7,a8,a9); }
    static CImg matrix(const T& a1,const T& a2,const T& a3,const T& a4,
               const T& a5,const T& a6,const T& a7,const T& a8,
               const T& a9,const T& a10,const T& a11,const T& a12,
               const T& a13,const T& a14,const T& a15,const T& a16) {
      return CImg<T>(4,4).fill(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16);
    }

    //! Return a diffusion tensor with specified coefficients
    static CImg tensor(const T& a1) { return matrix(a1); }
    static CImg tensor(const T& a1,const T& a2,const T& a3) { return matrix(a1,a2,a2,a3); }
    static CImg tensor(const T& a1,const T& a2,const T& a3,const T& a4,const T& a5,const T& a6) {
      return matrix(a1,a2,a3,a2,a4,a5,a3,a5,a6);
    }

    //! Return a diagonal matrix with specified coefficients
    static CImg diagonal(const T& a1) { return matrix(a1); }
    static CImg diagonal(const T& a1,const T& a2) { return matrix(a1,0,0,a2); }
    static CImg diagonal(const T& a1,const T& a2,const T& a3) { return matrix(a1,0,0,0,a2,0,0,0,a3); }
    static CImg diagonal(const T& a1,const T& a2,const T& a3,const T& a4) { return matrix(a1,0,0,0,0,a2,0,0,0,0,a3,0,0,0,0,a4); }
    //! Operator* (matrix product)
    template<typename t> CImg operator*(const CImg<t>& img) const {
      cimg_test_matrix(*this,"CImg<T>::operator*");
      cimg_test_matrix(img,"CImg<T>::operator*");
      if (width!=img.height) 
        throw CImgArgumentException("CImg<%s>::operator*() : can't multiply a matrix *this = (%ux%u) by a matrix (%ux%u)",
                                    pixel_type(),width,height,img.width,img.height);
      CImg res(img.width,height);
      double val;
      cimg_mapXY(res,i,j) { val=0; cimg_mapX(*this,k) val+=(*this)(k,j)*img(i,k); res(i,j) = (T)val; }
      return res;
    }
    //! Operator*= (matrix product)
    template<typename t> CImg& operator*=(const CImg<t>& img) { return ((*this)*img).swap(*this); }
  
    //! Return a new image corresponding to the vector located at (\p x,\p y,\p z) of the current vector-valued image.
    CImg get_vector(const unsigned int x=0,const unsigned int y=0,const unsigned int z=0) const {
      CImg dest(dim);
      cimg_mapV(*this,k) dest[k]=(*this)(x,y,z,k);
      return dest;
    }
  
    //! Return a new image corresponding to the \a square \a matrix located at (\p x,\p y,\p z) of the current vector-valued image.
    CImg get_matrix(const unsigned int x=0,const unsigned int y=0,const unsigned int z=0) const {
      const int n = (int)std::sqrt((double)dim);
      CImg dest(n,n);
      cimg_mapV(*this,k) dest[k]=(*this)(x,y,z,k);
      return dest;
    }
  
    //! Return a new image corresponding to the \a diffusion \a tensor located at (\p x,\p y,\p z) of the current vector-valued image.
    CImg get_tensor(const unsigned int x=0,const unsigned int y=0,const unsigned int z=0) const {      
      if (dim==6) return tensor((*this)(x,y,z,0),(*this)(x,y,z,1),(*this)(x,y,z,2),
                (*this)(x,y,z,3),(*this)(x,y,z,4),(*this)(x,y,z,5));
      if (dim==3) return tensor((*this)(x,y,z,0),(*this)(x,y,z,1),(*this)(x,y,z,2));
      return tensor((*this)(x,y,z,0));
    }

    //! Set the image \p vec as the \a vector \a valued pixel located at (\p x,\p y,\p z) of the current vector-valued image.
    CImg& set_vector(const CImg& vec,const unsigned int x=0,const unsigned int y=0,const unsigned int z=0) {
      return draw_point(x,y,z,vec.data,1);
    }
    //! Set the image \p vec as the \a square \a matrix-valued pixel located at (\p x,\p y,\p z) of the current vector-valued image.
    CImg& set_matrix(const CImg& mat,const unsigned int x=0,const unsigned int y=0,const unsigned int z=0) {
      return set_vector(mat,x,y,z);
    }
    //! Set the image \p vec as the \a tensor \a valued pixel located at (\p x,\p y,\p z) of the current vector-valued image.
    CImg& set_tensor(const CImg& ten,const unsigned int x=0,const unsigned int y=0,const unsigned int z=0) {
      if (ten.height==2) {
        (*this)(x,y,z,0)=ten[0];
        (*this)(x,y,z,1)=ten[1];
        (*this)(x,y,z,2)=ten[3];
      }
      else {
        (*this)(x,y,z,0)=ten[0];
        (*this)(x,y,z,1)=ten[1];
        (*this)(x,y,z,2)=ten[2];
        (*this)(x,y,z,3)=ten[4];
        (*this)(x,y,z,4)=ten[5];
        (*this)(x,y,z,5)=ten[8];
      }
      return *this;
    }
    //! Set the current matrix to be the identity matrix.
    CImg& identity_matrix(const unsigned int N) { return get_identity_matrix(N).swap(*this); }
      
    //! Return a matrix \p dim * \p dim equal to \p factor * \a Identity.
    static CImg get_identity_matrix(const unsigned int N) {
      CImg<T> res(N,N,1,1,0);
      cimg_mapX(res,x) res(x,x)=1;
      return res;
    }
  
    //! Return the transpose version of the current matrix.
    CImg get_transpose() const {
      CImg<T> res(height,width,depth,dim);
      cimg_mapXYZV(*this,x,y,z,v) res(y,x,z,v) = (*this)(x,y,z,v);      
      return res;
    }
    
    //! Replace the current matrix by its transpose.
    CImg& transpose() {
      if (width==1) { width=height; height=1; return *this; }
      if (height==1) { height=width; width=1; return *this; }
      if (width==height) {
    cimg_mapYZV(*this,y,z,v) for (int x=y; x<(int)width; x++) cimg::swap((*this)(x,y,z,v),(*this)(y,x,z,v));
    return *this;
      }
      return (*this)=get_transpose();
    }

    //! Get a diagonal matrix, whose diagonal coefficients are the coefficients of the input image
    CImg get_diagonal() const {
      cimg_test(*this,"CImg<T>::get_diagonal");
      CImg res(size(),size(),1,1,0);
      cimg_mapoff(*this,off) res(off,off)=(*this)(off);
      return res;
    }
    //! Replace a vector by a diagonal matrix containing the original vector coefficients.
    CImg& diagonal() { return get_diagonal().swap(*this); }

    //! Inverse the current matrix.
    CImg& inverse() {
      cimg_test_square(*this,"CImg<T>::inverse");
      switch (width) {
      case 2: {
    const double 
      a = data[0], c = data[1],
      b = data[2], d = data[3],
      dete = det();
    if (dete) { 
      data[0] = (T)(d/dete);  data[1] = (T)(-c/dete);
      data[2] = (T)(-b/dete), data[3] = (T)(a/dete); 
    } else {
      cimg::warn(true,"CImg<%s>::inverse() : Matrix determinant is 0, can't invert matrix",pixel_type());
      fill(0);
    }
      } break;
      case 3: {
    const double
      a = data[0], d = data[1], g = data[2],
      b = data[3], e = data[4], h = data[5],
      c = data[6], f = data[7], i = data[8],
      dete = det();
    if (dete) {
      data[0] = (T)((i*e-f*h)/dete), data[1] = (T)((g*f-i*d)/dete), data[2] = (T)((d*h-g*e)/dete);
      data[3] = (T)((h*c-i*b)/dete), data[4] = (T)((i*a-c*g)/dete), data[5] = (T)((g*b-a*h)/dete);
      data[6] = (T)((b*f-e*c)/dete), data[7] = (T)((d*c-a*f)/dete), data[8] = (T)((a*e-d*b)/dete);
    } else {
      cimg::warn(true,"CImg<%s>::inverse() : Matrix determinant is 0, can't invert matrix",pixel_type());
      fill(0);
    }
      } break;
      default: {
    CImg<T> U(width,width),S(1,width),V(width,width);
    SVD(U,S,V,false);
    U.transpose();
    cimg_mapY(S,k) if (S[k]!=0) S[k]=1/S[k];
    else cimg::warn(true,"CImg<%s>::inverse() : Matrix determinant is 0, can't invert matrix",pixel_type());
    S.diagonal();
    *this = V*S*U;
      } break;
      }
      return *this;
    }

    //! Return the inverse of the current matrix.
    CImg get_inverse() const { return CImg<T>(*this).inverse(); }

    //! Return the trace of the current matrix.
    double trace() const {
      cimg_test_square(*this,"CImg<T>::trace");
      double res=0;
      cimg_mapX(*this,k) res+=(*this)(k,k);
      return res;
    }
    //! Return the dot product of the current vector/matrix with the vector/matrix \p img.
    double dot(const CImg& img) const {
      cimg_test(*this,"CImg<T>::dot"); cimg_test(img,"CImg<T>::dot");
      const unsigned long nb = cimg::min(size(),img.size());
      double res=0;
      for (unsigned long off=0; off<nb; off++) res+=data[off]*img[off];
      return res;
    }
    
    //! Return the cross product between two 3d vectors
    CImg& cross(const CImg& img) {
      if (width!=1 || height<3 || img.width!=1 || img.height<3)
        throw CImgInstanceException("CImg<%s>::cross() : cannot get cross product between two matrices (%u,%u) and (%u,%u)",
                                    pixel_type(),width,height,img.width,img.height);
      const T x = (*this)[0], y = (*this)[1], z = (*this)[2];
      (*this)[0] = y*img[2]-z*img[1];
      (*this)[1] = z*img[0]-x*img[2];
      (*this)[2] = x*img[1]-y*img[0];
      return *this;
    }
    //! Return the cross product between two 3d vectors
    CImg get_cross(const CImg& img) const { return CImg<T>(*this).cross(img); }

    //! Return the determinant of the current matrix.
    double det() const {
      cimg_test_square(*this,"CImg<T>::det");
      switch (width) {
      case 1: return (*this)(0,0);
      case 2: return (*this)(0,0)*(*this)(1,1)-(*this)(0,1)*(*this)(1,0);
      case 3: 
        {
          const double
            a = data[0], d = data[1], g = data[2],
            b = data[3], e = data[4], h = data[5],
            c = data[6], f = data[7], i = data[8];
          return i*a*e-a*h*f-i*b*d+b*g*f+c*d*h-c*g*e;
        }
      }
      return 0;
    }
    //! Return the norm of the current vector/matrix. \p ntype = norm type (0=L2, 1=L1, -1=Linf).
    double norm(const int ntype=2) const {
      cimg_test(*this,"CImg<T>::norm");
      double res = 0;
      switch (ntype) {
      case -1: {
    cimg_mapoff(*this,off) {
      const double tmp = cimg::abs((double)data[off]);
      if (tmp>res) res = tmp;
    }
    return res; 
      } break;
      case 1 : { 
    cimg_mapoff(*this,off) res+=cimg::abs((double)data[off]); 
    return res;
      } break;
      default: { return std::sqrt(dot(*this)); }
      }
      return 0;
    }
    //! Return the sum of all the pixel values in an image.
    double sum() const {
      cimg_test(*this,"CImg<T>::sum");        
      double res=0;
      cimg_map(*this,ptr,T) res+=*ptr;
      return res;
    }

    //! Compute the SVD of a general matrix.   
    template<typename t> const CImg& SVD(CImg<t>& U, CImg<t>& S, CImg<t>& V,const bool sorting=true) const {
      cimg_test_matrix(*this,"CImg<T>::SVD");
      U = *this;
      if (S.size()<width) S = CImg<t>(1,width);
      if (V.width<width || V.height<height) V = CImg<t>(width,width);
      CImg<t> rv1(width);  
      t anorm=0,c,f,g=0,h,s,scale=0;
      int l=0,nm=0;
     
      cimg_mapX(U,i) {
    l = i+1; rv1[i] = scale*g; g = s = scale = 0;
    if (i<dimy()) {
      for (int k=i; k<dimy(); k++) scale+= cimg::abs(U(i,k));
      if (scale) {
        for (int k=i; k<dimy(); k++) { U(i,k)/=scale; s+= U(i,k)*U(i,k); }
        f = U(i,i); g = (t)((f>=0?-1:1)*std::sqrt(s)); h=f*g-s; U(i,i)=f-g;
        for (int j=l; j<dimx(); j++) {
          s = 0; for (int k=i; k<dimy(); k++) s+= U(i,k)*U(j,k);
          f = s/h;
          { for (int k=i; k<dimy(); k++) U(j,k)+= f*U(i,k); }
        }
        { for (int k=i; k<dimy(); k++) U(i,k)*= scale; }
      }
    }
    S[i]=scale*g;
    
    g = s = scale = 0;
    if (i<dimy() && i!=dimx()-1) {
      for (int k=l; k<dimx(); k++) scale += cimg::abs(U(k,i));
      if (scale) {
        for (int k=l; k<dimx(); k++) { U(k,i)/= scale; s+= U(k,i)*U(k,i); }
        f = U(l,i); g = (t)((f>=0?-1:1)*std::sqrt(s)); h = f*g-s; U(l,i) = f-g;
        { for (int k=l; k<dimx(); k++) rv1[k]=U(k,i)/h; }
        for (int j=l; j<dimy(); j++) {
          s=0; for (int k=l; k<dimx(); k++) s+= U(k,j)*U(k,i);
          { for (int k=l; k<dimx(); k++) U(k,j)+= s*rv1[k]; }
        }
        { for (int k=l; k<dimx(); k++) U(k,i)*= scale; }
      }
    }
    anorm=cimg::max((t)anorm,(cimg::abs(S[i])+cimg::abs(rv1[i])));
      }
      
      { for (int i=dimx()-1; i>=0; i--) {
    if (i<dimx()-1) {
      if (g) {
        { for (int j=l; j<dimx(); j++) V(i,j) =(U(j,i)/U(l,i))/g; }
        for (int j=l; j<dimx(); j++) {
          s=0; for (int k=l; k<dimx(); k++) s+= U(k,i)*V(j,k);
          { for (int k=l; k<dimx(); k++) V(j,k)+= s*V(i,k); }
        }
      }
      for (int j=l; j<dimx(); j++) V(j,i)=V(i,j)=0.0;
    }
    V(i,i) = 1.0; g = rv1[i]; l = i;
      }
      }
      
      { for (int i=cimg::min(dimx(),dimy())-1; i>=0; i--) {
    l = i+1; g = S[i];
    for (int j=l; j<dimx(); j++) U(j,i) = 0;
    if (g) {
      g = 1/g;
      for (int j=l; j<dimx(); j++) {
        s=0; for (int k=l; k<dimy(); k++) s+= U(i,k)*U(j,k);
        f = (s/U(i,i))*g;
        { for (int k=i; k<dimy(); k++) U(j,k)+= f*U(i,k); }
      }
      { for (int j=i; j<dimy(); j++) U(i,j)*= g; }
    } else for (int j=i; j<dimy(); j++) U(i,j)=0;
    U(i,i)++;
      }
      }
      
      for (int k=dimx()-1; k>=0; k--) {
    for (int its=0; its<40; its++) {
      bool flag = true;
      for (l=k; l>=0; l--) {
        nm = l-1;
        if ((cimg::abs(rv1[l])+anorm)==anorm) { flag = false; break; }
        if ((cimg::abs(S[nm])+anorm)==anorm) break;
      }
      if (flag) {
        c = 0; s = 1;
        for (int i=l; i<=k; i++) {
          f = s*rv1[i]; rv1[i] = c*rv1[i];
          if ((cimg::abs(f)+anorm)==anorm) break;
          g = S[i]; h = (t)cimg::pythagore(f,g); S[i] = h; h = 1/h; c = g*h; s = -f*h;
          cimg_mapY(U,j) { const t y = U(nm,j), z = U(i,j); U(nm,j) = y*c+z*s; U(i,j) = z*c-y*s; }
        }
      }
      const t& z = S[k];
      if (l==k) { if (z<0) { S[k] = -z; cimg_mapX(U,j) V(k,j) = -V(k,j); } break; }
      cimg::warn(its>=39,"CImg<%s>::SVD() : SVD failed to converge",pixel_type());
      nm = k-1; 
      t x = S[l], y = S[nm]; 
      g = rv1[nm]; h = rv1[k];
      f = ((y-z)*(y+z)+(g-h)*(g+h))/(2*h*y);
      g = (t)cimg::pythagore(f,1.0);
      f = ((x-z)*(x+z)+h*((y/(f+ (f>=0?g:-g)))-h))/x;
      c = s = 1;
      for (int j=l; j<=nm; j++) {
        const int i = j+1;
        g = rv1[i]; h = s*g; g = c*g;
        t y = S[i];
        t z = (t)cimg::pythagore(f,h); 
        rv1[j] = z; c = f/z; s = h/z;
        f = x*c+g*s; g = g*c-x*s; h = y*s; y*=c;
        cimg_mapX(U,jj) { const t x = V(j,jj), z = V(i,jj); V(j,jj) = x*c+z*s; V(i,jj) = z*c-x*s; }
        z = (t)cimg::pythagore(f,h); S[j] = z;
        if (z) { z = 1/z; c = f*z; s = h*z; }
        f = c*g+s*y; x = c*y-s*g;
        { cimg_mapY(U,jj) { const t y = U(j,jj); z = U(i,jj); U(j,jj) = y*c+z*s; U(i,jj) = z*c-y*s; }}
      }
      rv1[l] = 0; rv1[k]=f; S[k]=x;
    }
      }
      
      if (sorting) {
    CImg<int> permutations(width);
    S.quicksort(permutations,false);
    cimg_mapX(permutations,x) {
      const int n = permutations(x);
      if (x<n) {
        cimg_mapY(U,k) cimg::swap(U(x,k),U(n,k));
        cimg_mapY(V,l) cimg::swap(V(x,l),V(n,l));
      }
    }
      }
      return *this;
    }

    //! Compute the SVD of a general matrix.
    template<typename t> const CImg& SVD(CImgl<t>& USV) const {
      if (USV.size<3) USV = CImgl<t>(3);
      return SVD(USV[0],USV[1],USV[2]);      
    }
    
    //! Compute the SVD of a general matrix.
    CImgl<T> get_SVD(const bool sorting=true) const {
      CImgl<T> res(3);
      SVD(res[0],res[1],res[2],sorting);
      return res;
    }
        
    //! Compute the eigenvalues and eigenvectors of a matrix.
    template<typename t> const CImg<T>& eigen(CImg<t>& val, CImg<t> &vec) const {
      cimg_test_square(*this,"CImg<T>::eigen");
      if (val.size()<width) val = CImg<t>(1,width);
      if (vec.size()<width*width) vec = CImg<t>(width,width);
      switch(width) {
      case 1: { val[0]=(t)(*this)[0]; vec[0]=(t)1; } break;
      case 2: {
    const double a = (*this)[0], b = (*this)[1], c = (*this)[2], d = (*this)[3], e = a+d;
    double f = e*e-4*(a*d-b*c);
    cimg::warn(f<0,"CImg<%s>::eigen() : Complex eigenvalues",pixel_type());
    f = std::sqrt(f);
    const double l1 = 0.5*(e-f), l2 = 0.5*(e+f);
    double u, v, n;
    val[0]=(t)l2; val[1]=(t)l1;
    if (cimg::abs(b)>cimg::abs(a-l2)) { u = 1; v = (l2-a)/b; }
    else { if (a-l2!=0) { u = -b/(a-l2); v = 1; } else { u = 0; v = 1; } }
    n = std::sqrt(u*u+v*v); u/=n; v/=n; vec(0,0) = (t)u; vec(0,1) = (t)v;
    if (cimg::abs(b)>cimg::abs(a-l1)) { u = 1; v = (l1-a)/b; }
    else { if (a-l1!=0) { u = -b/(a-l1); v = 1; } else { u = 1; v = 0; } }
    n = std::sqrt(u*u+v*v); u/=n; v/=n; vec(1,0) = (t)u; vec(1,1) = (t)v;
      } break;
      default: 
        throw CImgInstanceException("CImg<%s>::eigen() : Eigenvalues computation of general matrices is limited to 2x2 matrices "
                    "(given is %ux%u)", pixel_type(),width,height);
      }
      return *this;
    }

    //! Return the eigenvalues and eigenvectors of a matrix.
    CImgl<T> get_eigen() const { CImgl<T> res(2); eigen(res[0],res[1]); return res; }
    
    //! Compute the eigenvalues and eigenvectors of a matrix.
    template<typename t> const CImg<T>& eigen(CImgl<t>& eig) const {
      if (eig.size<2) eig = CImgl<t>(2);
      eigen(eig[0],eig[1]);
      return *this; 
    }
    
    //! Compute the eigenvalues and eigenvectors of a symmetric matrix.
    template<typename t> const CImg<T>& symeigen(CImg<t>& val, CImg<t>& vec) const {
      cimg_test_square(*this,"CImg<T>::symeigen");
      if (val.size()<width) val = CImg<t>(width);
      if (vec.data && vec.size()<width*width) vec = CImg<t>(width,width);
      if (width<3) return eigen(val,vec);     
      CImg<t> V(width,width);
      return SVD(vec,val,V,true);
    }

    //! Compute the eigenvalues and eigenvectors of a symmetric matrix.
    CImgl<T> get_symeigen() const { CImgl<T> res(2); symeigen(res[0],res[1]); return res; }

    //! Compute the eigenvalues and eigenvectors of a symmetric matrix.
    template<typename t> const CImg<T>& symeigen(CImgl<t>& eig) const {
      if (eig.size<2) eig = CImgl<t>(2);
      symeigen(eig[0],eig[1]);
      return *this;
    }

    template<typename t> CImg<T>& _quicksort(const int min,const int max,CImg<t>& permutations,const bool increasing) {
      if (min<max) {      
    const int mid = (min+max)/2;
    if (increasing) {
      if ((*this)[min]>(*this)[mid]) {
        cimg::swap((*this)[min],(*this)[mid]); cimg::swap(permutations[min],permutations[mid]); }
      if ((*this)[mid]>(*this)[max]) {
        cimg::swap((*this)[max],(*this)[mid]); cimg::swap(permutations[max],permutations[mid]); }
      if ((*this)[min]>(*this)[mid]) {
        cimg::swap((*this)[min],(*this)[mid]); cimg::swap(permutations[min],permutations[mid]); }
    } else {
      if ((*this)[min]<(*this)[mid]) {
        cimg::swap((*this)[min],(*this)[mid]); cimg::swap(permutations[min],permutations[mid]); }
      if ((*this)[mid]<(*this)[max]) {
        cimg::swap((*this)[max],(*this)[mid]); cimg::swap(permutations[max],permutations[mid]); }
      if ((*this)[min]<(*this)[mid]) {
        cimg::swap((*this)[min],(*this)[mid]); cimg::swap(permutations[min],permutations[mid]); }
    }
    if (max-min>=3) {
      const T pivot = (*this)[mid];
      int i = min, j = max;
      if (increasing) {
        do {
          while ((*this)[i]<pivot) i++; while ((*this)[j]>pivot) j--;
          if (i<=j) {
        cimg::swap((*this)[i],(*this)[j]);
        cimg::swap(permutations[i],permutations[j]);
        i++; j--; }
        } while (i<=j);
      } else {
        do {
          while ((*this)[i]>pivot) i++; while ((*this)[j]<pivot) j--;
          if (i<=j) {
        cimg::swap((*this)[i],(*this)[j]);
        cimg::swap(permutations[i],permutations[j]);
        i++; j--; }
        } while (i<=j);
      }
      if (min<j) _quicksort(min,j,permutations,increasing);
      if (i<max) _quicksort(i,max,permutations,increasing);
    }
      }
      return *this;
    }

    //! Sort values of a vector and get permutations.
    template<typename t>
    CImg<T>& quicksort(CImg<t>& permutations,const bool increasing=true) {
      cimg_test(*this,"CImg<T>::quicksort");
      if (permutations.size()!=size()) permutations = CImg<t>(size());
      cimg_mapoff(permutations,off) permutations[off] = off;
      return _quicksort(0,size()-1,permutations,increasing); 
    }

    //! Sort values of a vector.
    CImg<T>& quicksort(const bool increasing=true) { CImg<T> foo; return quicksort(foo,increasing); }

    //! Get a sorted version a of vector, with permutations.
    template<typename t> CImg<T>& get_quicksort(CImg<t>& permutations,const bool increasing=true) {
      return CImg<T>(*this).quicksort(permutations,increasing);
    }

    //! Get a sorted version a of vector.
    CImg<T>& get_quicksort(const bool increasing=true) { 
      return CImg<T>(*this).quicksort(increasing); 
    }
    
    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name Display functions
    //@{
    //------------------------------------------
    //------------------------------------------
  
    //! Display an image into a CImgDisplay window.
    const CImg& display(CImgDisplay& disp,const unsigned int ymin=0,const unsigned int ymax=~0) const { disp.display(*this,ymin,ymax); return *this; }

    //! Same as \ref cimg::wait()
    const CImg& wait(const unsigned int milliseconds) const { cimg::wait(milliseconds); return *this;  }
  
    //! Display an image in a window with a title \p title, and wait a 'closed' or 'keyboard' event.\n
    //! Parameters \p min_size and \p max_size set the minimum and maximum dimensions of the display window.
    //! If negative, they corresponds to a percentage of the original image size.
    const CImg& display(const char* title,const int min_size=128,const int max_size=1024) const {
      cimg_test(*this,"CImg<T>::display");
      CImgDisplay *disp;
      unsigned int w = width+(depth>1?depth:0), h = height+(depth>1?depth:0), XYZ[3];
      print(title);
      const unsigned int dmin = cimg::min(w,h), minsiz = min_size>=0?min_size:(-min_size)*dmin/100;
      if (dmin<minsiz) { w=w*minsiz/dmin; w+=(w==0); h=h*minsiz/dmin; h+=(h==0); }
      const unsigned int dmax = cimg::max(w,h), maxsiz = max_size>=0?max_size:(-max_size)*dmax/100;
      if (dmax>maxsiz) { w=w*maxsiz/dmax; w+=(w==0); h=h*maxsiz/dmax; h+=(h==0); }
      disp = new CImgDisplay(CImg<unsigned char>(w,h,1,1,0),title,0,3);
      XYZ[0] = width/2; XYZ[1] = height/2; XYZ[2] = depth/2;
      while (!disp->closed && !disp->key) feature_selection(NULL,1,*disp,XYZ);
      delete disp;
      return *this;
    }

    //! Display an image in a window, with a default title. See also \see display() for details on parameters.
    const CImg& display(const int min_size=128,const int max_size=1024) const { return display("",min_size,max_size); }
  
    //! High-level interface to select features from images
    const CImg& feature_selection(int *const selection, const int feature_type,CImgDisplay &disp,
                                  unsigned int *const XYZ=NULL,const unsigned char *const color=NULL) const {
      cimg_test(*this,"CImg<T>::feature_selection");
      if (disp.events<3) 
        throw CImgArgumentException("CImg<%s>::feature_selection() : Input display must be able to catch keyboard"
                    "and mouse events (events>=3). Given display has 'events = %s'.",
                    pixel_type(),disp.events);
      unsigned char fgcolor[3]={255,255,105}, bgcolor[3]={0,0,0};
      if (color) std::memcpy(fgcolor,color,sizeof(unsigned char)*cimg::min(3,dimv()));
      int carea=0,area=0,phase=0,
        X0=(XYZ?XYZ[0]:width/2)%width, Y0=(XYZ?XYZ[1]:height/2)%height, Z0=(XYZ?XYZ[2]:depth/2)%depth, 
        X=-1,Y=-1,Z=-1,oX=-1,oY=-1,oZ=-1,X1=-1,Y1=-1,Z1=-1;
      unsigned int hatch=feature_type?0xF0F0F0F0:~0L;
      bool feature_selected = false, ytext = false;
      CImg<unsigned char> visu, visu0;
      char text[1024];
    
      while (!disp.key && !disp.closed && !feature_selected) {

        // Init visu0 if necessary
        if (disp.resized || !visu0.data) { 
          if (disp.resized) disp.resize();
          if (depth==1) visu0=get_normalize(0,(T)255); else visu0=get_3dplanes(X0,Y0,Z0).get_normalize(0,(T)255);
          visu0.resize(disp.width,disp.height,1,cimg::min(3,dimv()));
        }
        visu = visu0;      
    
        // Handle motion and selection
        const int mx = disp.mouse_x, my = disp.mouse_y, b = disp.button;
        if (mx>=0 && my>=0) {
          const int mX = mx*(width+(depth>1?depth:0))/disp.width, mY = my*(height+(depth>1?depth:0))/disp.height;
          if (mX<dimx() && mY<dimy())   { area=1; X=mX; Y=mY; Z=phase?Z1:Z0; }
          if (mX<dimx() && mY>=dimy())  { area=2; X=mX; Y=phase?Y1:Y0; Z=mY-height; }
          if (mX>=dimx() && mY<dimy())  { area=3; X=phase?X1:X0; Y=mY; Z=mX-width;  }
          if (mX>=dimx() && mY>=dimy()) { X=X0; Y=Y0; Z=Z0; }
          if ((!(phase%2) && (b&1)) || (phase%2 && !(b&1))) { 
            if (!carea) carea=area;
            if (!(phase++)) { X0=X; Y0=Y; Z0=Z; }
          }
          if (b&2) { if (!phase) { X0=X; Y0=Y; Z0=Z; } else { X1=Y1=Z1=-1; phase=carea=0; }}
          if ((b&2 || phase) && depth>1) 
            visu0 = get_3dplanes(X,Y,Z).normalize(0,(T)255).resize(disp.width,disp.height,1,cimg::min(3,dimv()));
          if (phase) {
            if (!feature_type) feature_selected = phase?true:false;
            else {
              if (depth>1) feature_selected = (phase==3)?true:false;
              else feature_selected = (phase==2)?true:false;
            }   
            if (!feature_selected) {
              if (phase<2) { X1=X; Y1=Y; Z1=Z; }
              else switch(carea) {
              case 1: Z1=Z; break;
              case 2: Y1=Y; break;
              case 3: X1=X; break;
              }
            }
          }
          if (!phase || !feature_type) {
            if (depth>1) std::sprintf(text,"Coords (%d,%d,%d)={ ",X,Y,Z); else std::sprintf(text,"Coords (%d,%d)={ ",X,Y);
            cimg_mapV(*this,k) std::sprintf(text+cimg::strlen(text),"%g ",(double)(*this)(X,Y,Z,k));
            std::sprintf(text+cimg::strlen(text),"}");
            if (!feature_type) { X1=X0; Y1=Y0; Z1=Z0; }
          } else
            switch (feature_type) {
            case 1: {
                const double dX=(double)(X0-X1), dY=(double)(Y0-Y1), dZ=(double)(Z0-Z1), norm = std::sqrt(dX*dX+dY*dY+dZ*dZ);
                if (depth>1) std::sprintf(text,"Vect (%d,%d,%d)-(%d,%d,%d), norm=%g",X0,Y0,Z0,X1,Y1,Z1,norm);
                else std::sprintf(text,"Vect (%d,%d)-(%d,%d), norm=%g",X0,Y0,X1,Y1,norm);
        } break;
            case 2:
              if (depth>1) std::sprintf(text,"Box (%d,%d,%d)-(%d,%d,%d), Size=(%d,%d,%d)",
                                        X0<X1?X0:X1,Y0<Y1?Y0:Y1,Z0<Z1?Z0:Z1,
                                        X0<X1?X1:X0,Y0<Y1?Y1:Y0,Z0<Z1?Z1:Z0,
                                        1+cimg::abs(X0-X1),1+cimg::abs(Y0-Y1),1+cimg::abs(Z0-Z1));
              else  std::sprintf(text,"Box (%d,%d)-(%d,%d), Size=(%d,%d)",
                                 X0<X1?X0:X1,Y0<Y1?Y0:Y1,X0<X1?X1:X0,Y0<Y1?Y1:Y0,1+cimg::abs(X0-X1),1+cimg::abs(Y0-Y1));
              break;
        case 3:
              if (depth>1) std::sprintf(text,"Ellipse (%d,%d,%d)-(%d,%d,%d), Radii=(%d,%d,%d)",
                    X0,Y0,Z0,X1,Y1,Z1,1+cimg::abs(X0-X1),1+cimg::abs(Y0-Y1),1+cimg::abs(Z0-Z1));
              else  std::sprintf(text,"Ellipse (%d,%d)-(%d,%d), Radii=(%d,%d)",
                 X0,Y0,X1,Y1,1+cimg::abs(X0-X1),1+cimg::abs(Y0-Y1));

              break;
            }
          if (my<12) ytext=true;
          if (my>=visu.dimy()-11) ytext=false;
          visu.draw_text(text,0,ytext?visu.dimy()-11:0,fgcolor,bgcolor,0.7f);
        } else { X=Y=Z=-1; if (phase) disp.button=phase%2; }
    
        // Draw image + selection on display window
        if (X>=0 && Y>=0 && Z>=0) {
          hatch=cimg::ror(hatch);
          if (feature_type==1 && phase) {
            const int d=(depth>1)?depth:0,
              x0=(int)((X0+0.5f)*disp.width/(width+d)), y0=(int)((Y0+0.5f)*disp.height/(height+d)),
              x1=(int)((X1+0.5f)*disp.width/(width+d)), y1=(int)((Y1+0.5f)*disp.height/(height+d));
            visu.draw_arrow(x0,y0,x1,y1,fgcolor,30.0f,5.0f,hatch);
            if (d) {
              const int zx0=(int)((width+Z0+0.5f)*disp.width/(width+d)), zx1=(int)((width+Z1+0.5f)*disp.width/(width+d)),
                zy0=(int)((height+Z0+0.5f)*disp.height/(height+d)), zy1=(int)((height+Z1+0.5f)*disp.height/(height+d));
              visu.draw_arrow(zx0,y0,zx1,y1,fgcolor,30.0f,5.0f,hatch).draw_arrow(x0,zy0,x1,zy1,fgcolor,30.0f,5.0f,hatch);
            }
          } else switch(feature_type) {
      case 2: {
            const bool cond=(phase&&feature_type);
            const int d=(depth>1)?depth:0,
              nX0=cond?X0:X, nY0=cond?Y0:Y, nZ0=cond?Z0:Z,
              nX1=cond?X1:X, nY1=cond?Y1:Y, nZ1=cond?Z1:Z,
              x0=(nX0<nX1?nX0:nX1)*disp.width/(width+d),
              y0=(nY0<nY1?nY0:nY1)*disp.height/(height+d),
              x1=((nX0<nX1?nX1:nX0)+1)*disp.width/(width+d)-1,
              y1=((nY0<nY1?nY1:nY0)+1)*disp.height/(height+d)-1;
            const unsigned int nhatch=phase?hatch:~0L;
        visu.draw_rectangle(x0,y0,x1,y1,fgcolor,0.2f).draw_line(x0,y0,x1,y0,fgcolor,nhatch).
          draw_line(x1,y0,x1,y1,fgcolor,nhatch).draw_line(x1,y1,x0,y1,fgcolor,nhatch).draw_line(x0,y1,x0,y0,fgcolor,nhatch);
            if (d) {
              const int
                zx0=(int)((width+(nZ0<nZ1?nZ0:nZ1))*disp.width/(width+d)),
                zy0=(int)((height+(nZ0<nZ1?nZ0:nZ1))*disp.height/(height+d)),
                zx1=(int)((width+(nZ0<nZ1?nZ1:nZ0)+1)*disp.width/(width+d))-1,
                zy1=(int)((height+(nZ0<nZ1?nZ1:nZ0)+1)*disp.height/(height+d))-1;
              visu.draw_rectangle(zx0,y0,zx1,y1,fgcolor,0.2f).draw_line(zx0,y0,zx1,y0,fgcolor,nhatch).
                draw_line(zx1,y0,zx1,y1,fgcolor,nhatch).draw_line(zx1,y1,zx0,y1,fgcolor,nhatch).draw_line(zx0,y1,zx0,y0,fgcolor,nhatch);
              visu.draw_rectangle(x0,zy0,x1,zy1,fgcolor,0.2f).draw_line(x0,zy0,x1,zy0,fgcolor,nhatch).
                draw_line(x1,zy0,x1,zy1,fgcolor,nhatch).draw_line(x1,zy1,x0,zy1,fgcolor,nhatch).draw_line(x0,zy1,x0,zy0,fgcolor,nhatch);
            }
      } break;
      case 3: {
            const bool cond=(phase&&feature_type);
            const int d=(depth>1)?depth:0,
              x0=(cond?X0:X)*disp.width/(width+d),
              y0=(cond?Y0:Y)*disp.height/(height+d),
              x1=(cond?X1:X)*disp.width/(width+d)-1,
              y1=(cond?Y1:Y)*disp.height/(height+d)-1;
            const unsigned int nhatch=phase?hatch:~0L;
        visu.draw_ellipse(x0,y0,(float)(x1-x0),(float)(y1-y0),1.0f,0.0f,fgcolor,0L,0.2f).
             draw_ellipse(x0,y0,(float)(x1-x0),(float)(y1-y0),1.0f,0.0f,fgcolor,nhatch);
            if (d) {
              const int
                zx0=(int)((width+(cond?Z0:Z))*disp.width/(width+d)),
                zy0=(int)((height+(cond?Z0:Z))*disp.height/(height+d)),
                zx1=(int)((width+(cond?Z1:Z)+1)*disp.width/(width+d))-1,
                zy1=(int)((height+(cond?Z1:Z)+1)*disp.height/(height+d))-1;
              visu.draw_ellipse(zx0,y0,(float)(zx1-zx0),(float)(y1-y0),1.0f,0.0f,fgcolor,0L,0.2f).
                   draw_ellipse(zx0,y0,(float)(zx1-zx0),(float)(y1-y0),1.0f,0.0f,fgcolor,nhatch).
                   draw_ellipse(x0,zy0,(float)(x1-x0),(float)(zy1-zy0),1.0f,0.0f,fgcolor,0L,0.2f).
                   draw_ellipse(x0,zy0,(float)(x1-x0),(float)(zy1-zy0),1.0f,0.0f,fgcolor,nhatch);
            }
      } break;
          }
        }
        visu.display(disp).wait(32);
        if ((!feature_selected && !phase && oX==X && oY==Y && oZ==Z) || (X<0 || Y<0 || Z<0)) disp.wait();
        oX=X; oY=Y; oZ=Z;
      }

      // Return result
      if (XYZ) { XYZ[0] = X; XYZ[1] = Y; XYZ[2] = Z; }
      if (feature_selected) {
        if (feature_type==2) {
          if (X0>X1) cimg::swap(X0,X1);
          if (Y0>Y1) cimg::swap(Y0,Y1);
          if (Z0>Z1) cimg::swap(Z0,Z1);
        }
        if (selection) {
          if (X1<0 || Y1<0 || Z1<0) X0=Y0=Z0=X1=Y1=Z1=-1;
          switch(feature_type) {
          case 1:
          case 2:  selection[3] = X1; selection[4] = Y1; selection[5] = Z1;
          default: selection[0] = X0; selection[1] = Y0; selection[2] = Z0;
          }
        }
      } else if (selection) selection[0]=selection[1]=selection[2]=selection[3]=selection[4]=selection[5]=-1;
      disp.button=0;
      return *this;
    }

    //! High-level interface to select features in images
    const CImg& feature_selection(int *const selection, const int feature_type,
                                  unsigned int *const XYZ=NULL,const unsigned char *const color=NULL) const {
      unsigned int w = width + (depth>1?depth:0), h = height + (depth>1?depth:0);
      const unsigned int dmin = cimg::min(w,h), minsiz = 256;
      if (dmin<minsiz) { w=w*minsiz/dmin; h=h*minsiz/dmin; }
      const unsigned int dmax = cimg::max(w,h), maxsiz = 1024;
      if (dmax>maxsiz) { w=w*maxsiz/dmax; h=h*maxsiz/dmax; }
      CImgDisplay disp(w,h,"",0,3);
      return feature_selection(selection,feature_type,disp,XYZ,color);
    }
  
    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name Input-Output functions
    //@{
    //------------------------------------------
    //------------------------------------------

    //! Load an image from a file.
    /**
       \param filename = name of the image file to load.
       \return A CImg<T> instance containing the pixel data defined in the image file.
       \note The extension of \c filename defines the file format. If no filename
       extension is provided, CImg<T>::load() will try to load a RAW file.
    **/
    static CImg load(const char *filename) {
      if (!filename) throw CImgArgumentException("CImg<%s>::load() : Can't load (null) filename",pixel_type());
      const char *ext = cimg::filename_split(filename);
      if (!cimg::strcasecmp(ext,"asc")) return load_ascii(filename);
      if (!cimg::strcasecmp(ext,"dlm")) return load_dlm(filename);
      if (!cimg::strcasecmp(ext,"inr")) return load_inr(filename);
      if (!cimg::strcasecmp(ext,"hdr")) return load_analyze(filename);
      if (!cimg::strcasecmp(ext,"pan")) return load_pandore(filename);
      if (!cimg::strcasecmp(ext,"bmp")) return load_bmp(filename);
      if (!cimg::strcasecmp(ext,"png")) return load_png(filename);
      if (!cimg::strcasecmp(ext,"jpg") ||
      !cimg::strcasecmp(ext,"jpeg")) return load_jpeg(filename);
      if (!cimg::strcasecmp(ext,"ppm") || 
      !cimg::strcasecmp(ext,"pgm") ||
      !cimg::strcasecmp(ext,"pnm")) return load_pnm(filename);
      if (!cimg::strcasecmp(ext,"raw") || ext[0]=='\0') return load_raw(filename);      
      return load_convert(filename);
    }

    //! Load an image from an ASCII file
    static CImg load_ascii(const char *filename) {
      std::FILE *file = cimg::fopen(filename,"rb");
      char line[256] = {0};
      std::fscanf(file,"%255[^\n]",line);
      unsigned int off;
      int err=1, dx=0, dy=1, dz=1, dv=1;
      std::sscanf(line,"%d %d %d %d",&dx,&dy,&dz,&dv);
      if (!dx || !dy || !dz || !dv)
    throw CImgIOException("CImg<%s>::load_ascii() : File '%s' does not appear to be a valid ASC file.\n"
                  "Specified image dimensions are (%d,%d,%d,%d)",pixel_type(),filename,dx,dy,dz,dv);
      CImg dest(dx,dy,dz,dv);
      double val;
      T *ptr = dest.data;
      for (off=0; off<dest.size() && err==1; off++) {
    err = std::fscanf(file,"%lf%*[^0-9.eE+-]",&val); 
    *(ptr++)=(T)val; 
      }
      cimg::warn(off<dest.size(),"CImg<%s>::load_ascii() : File '%s', only %u values read, instead of %u",
         pixel_type(),filename,off,dest.size());
      cimg::fclose(file);
      return dest;
    }

    //! Load an image from a DLM file
    static CImg load_dlm(const char *filename) {
      std::FILE *file = cimg::fopen(filename,"r");
      unsigned int cdx=0,dx=0,dy=0;
      double val;
      char c, delimiter[256]={0}, tmp[256];
      int err;
      while ((err = std::fscanf(file,"%lf%255[^0-9.eE+-]",&val,delimiter))!=EOF) {
    c=0;
    if (err>0) cdx++;
    if (!std::sscanf(delimiter,"%255[^\n]%c",tmp,&c) || c=='\n') { dx = cimg::max(cdx,dx); dy++; cdx=0; }
      }
      if (!dx || !dy) throw CImgIOException("CImg<%s>::load_dlm() : File '%s' does not appear to be a "
                        "valid DLM file (width = %d, height = %d)\n",pixel_type(),filename,dx,dy);
      std::rewind(file);
      CImg<T> dest(dx,dy,1,1,0);
      unsigned int x = 0, y = 0;
      while ((err = std::fscanf(file,"%lf%255[^0-9.eE+-]",&val,delimiter))!=EOF) {
    c=0;
    if (err>0) dest(x++,y) = (T)val;
    if (!std::sscanf(delimiter,"%255[^\n]%c",tmp,&c) || c=='\n') { x=0; y++; }
      }
      cimg::fclose(file);
      return dest;
    }

    //! Load an image from a PNM file
    static CImg load_pnm(const char *filename) {
      std::FILE *file=cimg::fopen(filename,"rb");
      char item[1024]={0};
      unsigned int ppm_type,width,height,colormax=255;
      int err;
      
      while ((err=std::fscanf(file,"%1023[^\n]",item))!=EOF && (item[0]=='#' || !err)) std::fgetc(file);
      if(std::sscanf(item," P%u",&ppm_type)!=1) 
        throw CImgIOException("CImg<%s>::load_pnm() : file '%s',PPM header 'P?' not found",pixel_type(),filename);
      while ((err=std::fscanf(file," %1023[^\n]",item))!=EOF && (item[0]=='#' || !err)) std::fgetc(file);
      if (std::sscanf(item," %u %u",&width,&height)!=2)
        throw CImgIOException("CImg<%s>::load_pnm() : file '%s',WIDTH and HEIGHT not defined",pixel_type(),filename);
      while ((err=std::fscanf(file," %1023[^\n]",item))!=EOF && (item[0]=='#' || !err)) std::fgetc(file);
      std::fgetc(file);
      cimg::warn(std::sscanf(item,"%u",&colormax)!=1,
         "CImg<%s>::load_pnm() : file '%s',COLORMAX not defined",pixel_type(),filename);
      
      CImg dest;
      int rval,gval,bval;

      switch (ppm_type) {
      case 2: { // Grey Ascii
    dest = CImg<T>(width,height,1,1);
    T* rdata = dest.ptr();
    cimg_mapoff(dest,off) { std::fscanf(file,"%d",&rval); *(rdata++)=(T)rval; }
      } break;
      case 3: { // Color Ascii
    dest = CImg<T>(width,height,1,3);
    T *rdata = dest.ptr(0,0,0,0), *gdata = dest.ptr(0,0,0,1), *bdata = dest.ptr(0,0,0,2);
    cimg_mapXY(dest,x,y) { 
      std::fscanf(file,"%d %d %d",&rval,&gval,&bval);
      *(rdata++)=(T)rval; 
      *(gdata++)=(T)gval; 
      *(bdata++)=(T)bval; }
      } break;
      case 5: { // Grey Binary
    if (colormax<256) { // 8 bits
      CImg<unsigned char> raw(width,height,1,1);
      cimg::fread(raw.data,sizeof(unsigned char),width*height,file);
      dest = CImg<T>(raw);
    } else { // 16 bits
      CImg<unsigned short> raw(width,height,1,1);
      cimg::fread(raw.data,sizeof(unsigned short),width*height,file);
      dest = CImg<T>(raw);
    }
      } break;
      case 6: { // Color Binary
    if (colormax<256) { // 8 bits
      CImg<unsigned char> raw(width,height,1,3);
      cimg::fread(raw.data,sizeof(unsigned char),width*height*3,file);
      dest = CImg<T>(raw.data,width,height,1,3,true);
    } else { // 16 bits
      CImg<unsigned short> raw(width,height,1,3);
      cimg::fread(raw.data,sizeof(unsigned short),width*height*3,file);
      dest = CImg<T>(raw.data,width,height,1,3,true);
    }
      } break;
      default:
    cimg::fclose(file);
    throw CImgIOException("CImg<%s>::load_pnm() : file '%s', PPM type 'P%d' not supported",pixel_type(),filename,ppm_type);
      }
      cimg::fclose(file);
      return dest;
    }

    //! Load an image from a BMP file.
    static CImg load_bmp(const char *filename) {
      unsigned char header[64];
      std::FILE *file = cimg::fopen(filename,"rb");
      cimg::fread(header,sizeof(unsigned char),54,file);
      if (header[0]!='B' || header[1]!='M')
    throw CImgIOException("CImg<%s>::load_bmp() : filename '%s' does not appear to be a valid BMP file",
                  pixel_type(),filename);
      
      // Read header and pixel buffer
      int
    file_size   = header[0x02] + (header[0x03]<<8) + (header[0x04]<<16) + (header[0x05]<<24),
    offset      = header[0x0A] + (header[0x0B]<<8) + (header[0x0C]<<16) + (header[0x0D]<<24),
    dx          = header[0x12] + (header[0x13]<<8) + (header[0x14]<<16) + (header[0x15]<<24),
    dy          = header[0x16] + (header[0x17]<<8) + (header[0x18]<<16) + (header[0x19]<<24),
    compression = header[0x1E] + (header[0x1F]<<8) + (header[0x20]<<16) + (header[0x21]<<24),
    nb_colors   = header[0x2E] + (header[0x2F]<<8) + (header[0x30]<<16) + (header[0x31]<<24),
    bpp         = header[0x1C] + (header[0x1D]<<8),
    *palette    = NULL;
      const int 
    dx_bytes   = (bpp==1)?(dx/8+(dx%8?1:0)):((bpp==4)?(dx/2+(dx%2?1:0)):(dx*bpp/8)),
    align      = (4-dx_bytes%4)%4,
    buf_size   = cimg::min(cimg::abs(dy)*(dx_bytes+align),file_size-offset);

      if (bpp<16) { if (!nb_colors) nb_colors=1<<bpp; } else nb_colors=0;
      if (nb_colors) { palette = new int[nb_colors]; cimg::fread(palette,sizeof(int),nb_colors,file); }
      const int    xoffset = offset-54-4*nb_colors;      
      if (xoffset>0) std::fseek(file,xoffset,SEEK_CUR);
      const unsigned char *buffer  = new unsigned char[buf_size], *ptrs = buffer;
      cimg::fread(buffer,sizeof(unsigned char),buf_size,file);
      cimg::fclose(file);

      // Decompress buffer (if necessary)
      if (compression) return load_convert(filename);
      
      // Read pixel data
      CImg res(dx,cimg::abs(dy),1,3);
      switch (bpp) {
      case 1: { // Monochrome
    for (int y=res.height-1; y>=0; y--) { 
      unsigned char mask = 0x80, val = 0;
      cimg_mapX(res,x) {
        if (mask==0x80) val = *(ptrs++);
        const unsigned char *col = (unsigned char*)(palette+(val&mask?1:0));
        res(x,y,2) = (T)*(col++);
        res(x,y,1) = (T)*(col++);
        res(x,y,0) = (T)*(col++);
        mask = cimg::ror(mask);
      } ptrs+=align; }
      } break;
      case 4: { // 16 colors
    for (int y=res.height-1; y>=0; y--) { 
      unsigned char mask = 0xF0, val = 0;
      cimg_mapX(res,x) {
        if (mask==0xF0) val = *(ptrs++);
        const unsigned char color = (mask<16)?(val&mask):((val&mask)>>4);
        unsigned char *col = (unsigned char*)(palette+color);
        res(x,y,2) = (T)*(col++);
        res(x,y,1) = (T)*(col++);
        res(x,y,0) = (T)*(col++);
        mask = cimg::ror(mask,4);
      } ptrs+=align; }
      } break;
      case 8: { //  256 colors
    for (int y=res.height-1; y>=0; y--) { cimg_mapX(res,x) {
      const unsigned char *col = (unsigned char*)(palette+*(ptrs++));
      res(x,y,2) = (T)*(col++);
      res(x,y,1) = (T)*(col++);
      res(x,y,0) = (T)*(col++);
    } ptrs+=align; }
      } break;
      case 16: { // 16 bits colors
    for (int y=res.height-1; y>=0; y--) { cimg_mapX(res,x) {
      const unsigned char c1 = *(ptrs++), c2 = *(ptrs++);
      const unsigned short col = c1+(c2<<8);
      res(x,y,2) = (T)(col&0x1F);
      res(x,y,1) = (T)((col>>5)&0x1F);
      res(x,y,0) = (T)((col>>10)&0x1F);
    } ptrs+=align; }
      } break;    
      case 24: { // 24 bits colors
    for (int y=res.height-1; y>=0; y--) { cimg_mapX(res,x) {
      res(x,y,2) = (T)*(ptrs++);
      res(x,y,1) = (T)*(ptrs++);
      res(x,y,0) = (T)*(ptrs++);
    } ptrs+=align; }
      } break;
      case 32: { // 32 bits colors
    for (int y=res.height-1; y>=0; y--) { cimg_mapX(res,x) {
      res(x,y,2) = (T)*(ptrs++);
      res(x,y,1) = (T)*(ptrs++);
      res(x,y,0) = (T)*(ptrs++);
      ptrs++;
    } ptrs+=align; }
      } break;
      }

      if (palette) delete[] palette;
      delete[] buffer;
      if (dy<0) res.flip('y');
      return res;
    }
    
    //! Load an image from a PNG file.
    // Note : Most of this function has been written by Eric Fausett
    static CImg load_png(const char *filename) {
#ifndef cimg_use_png
      return load_convert(filename);
#else
      // Open file and check for PNG validity
      unsigned char pngCheck[8];
      std::FILE *file = cimg::fopen(filename,"rb");
      cimg::fread(pngCheck,sizeof(unsigned char),8,file);
      if(png_sig_cmp(pngCheck,0,8)){
    cimg::fclose(file);
    throw CImgIOException("CImg<%s>::load_png() : filename '%s' does not appear to be a valid PNG file",pixel_type(),filename);
      }
      
      // Setup PNG structures for read
      png_voidp user_error_ptr=0;
      png_error_ptr user_error_fn=0, user_warning_fn=0;
      png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,    // Verifies libpng version correct
                           user_error_ptr, user_error_fn, user_warning_fn);
      if(!png_ptr){
    cimg::fclose(file);
    throw CImgIOException("CImg<%s>::load_png() : trouble initializing 'png_ptr' data structure",pixel_type());
      }
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if(!info_ptr){
    cimg::fclose(file);
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);            
    throw CImgIOException("CImg<%s>::load_png() : trouble initializing 'info_ptr' data structure",pixel_type());
      }
      png_infop end_info = png_create_info_struct(png_ptr);
      if(!end_info){
    cimg::fclose(file);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);            
    throw CImgIOException("CImg<%s>::load_png() : trouble initializing 'end_info' data structure",pixel_type());
      }
      
      // Error handling callback for png file reading
      if (setjmp(png_jmpbuf(png_ptr))){
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    cimg::fclose(file);
    throw CImgIOException("CImg<%s>::load_png() : unspecified error reading PNG file '%s'",pixel_type(),filename);
      }      
      png_init_io(png_ptr, file);      
      png_set_sig_bytes(png_ptr, 8);
      
      // Get PNG Header Info up to data block
      png_read_info(png_ptr, info_ptr);      
      png_uint_32 width, height;
      int bit_depth, color_type, interlace_type;
      png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type,
           int_p_NULL, int_p_NULL);
      int new_bit_depth = bit_depth;
      int new_color_type = color_type;
      
      // Transforms to unify image data        
      if (new_color_type == PNG_COLOR_TYPE_PALETTE){
    png_set_palette_to_rgb(png_ptr);
    new_color_type -= PNG_COLOR_MASK_PALETTE;
    new_bit_depth = 8;
      }
      if (new_color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8){
    png_set_gray_1_2_4_to_8(png_ptr);
    new_bit_depth = 8;
      }
      if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);
      if (new_color_type == PNG_COLOR_TYPE_GRAY || new_color_type == PNG_COLOR_TYPE_GRAY_ALPHA){
    png_set_gray_to_rgb(png_ptr);
    new_color_type |= PNG_COLOR_MASK_COLOR;
      }
      if (new_color_type == PNG_COLOR_TYPE_RGB)    png_set_filler(png_ptr, 0xffffU, PNG_FILLER_AFTER);      
      png_read_update_info(png_ptr, info_ptr);      
      if (!(new_bit_depth==8 || new_bit_depth==16))
    throw CImgIOException("CImg<%s>::load_png() : Wrong bit coding 'bit_depth=%u'",pixel_type(),new_bit_depth);
      const int byte_depth = new_bit_depth>>3;
      
      // Allocate Memory for Image Read
      png_bytep *imgData = new png_bytep[height];
      for (unsigned int row=0; row < height; row++) imgData[row] = new png_byte[byte_depth * 4 * width];
      png_read_image(png_ptr, imgData);
      png_read_end(png_ptr, end_info);
      
      // Read pixel data
      if (!(new_color_type==PNG_COLOR_TYPE_RGB || new_color_type==PNG_COLOR_TYPE_RGB_ALPHA))
    throw CImgIOException("CImg<%s>::load_png() : Wrong color coding new_color_type=%u",pixel_type(),new_color_type);
      const bool no_alpha_channel = (new_color_type==PNG_COLOR_TYPE_RGB);
      CImg res(width,height,1,no_alpha_channel?3:4);
      const unsigned long off = width*height;
      T *ptr1 = res.data, *ptr2 = ptr1+off, *ptr3 = ptr2+off, *ptr4 = ptr3+off;
      switch(new_bit_depth){
      case 8: {
    cimg_mapY(res,y){
      const unsigned char *ptrs = imgData[y];
      cimg_mapX(res,x){
        *(ptr1++) = (T)*(ptrs++);
        *(ptr2++) = (T)*(ptrs++);
        *(ptr3++) = (T)*(ptrs++);
        if (no_alpha_channel) ptrs++; else *(ptr4++) = (T)*(ptrs++);
      }
    }
      } break;
      case 16: {
    cimg_mapY(res,y){
      const unsigned short *ptrs = (unsigned short*)(imgData[y]);
      cimg_mapX(res,x){
        *(ptr1++) = (T)*(ptrs++);
        *(ptr2++) = (T)*(ptrs++);
        *(ptr3++) = (T)*(ptrs++);
        if (no_alpha_channel) ptrs++; else *(ptr4++) = (T)*(ptrs++);
      }
    }
      } break;
      }
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      
      // Deallocate Image Read Memory
      for (unsigned int n=0; n<height; n++) delete[] imgData[n];
      delete[] imgData;      
      cimg::fclose(file);
      return res;      
#endif      
    }


    //! Load a file in JPEG format.
    static CImg load_jpeg(const char *filename) {
#ifndef cimg_use_jpeg
      return load_convert(filename);
#else
      struct jpeg_decompress_struct cinfo;
      struct jpeg_error_mgr jerr;
      std::FILE *file = cimg::fopen(filename,"rb");
      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_decompress(&cinfo);
      jpeg_stdio_src(&cinfo,file);
      jpeg_read_header(&cinfo,TRUE);
      jpeg_start_decompress(&cinfo);
      
      if (cinfo.output_components!=3) {
    cimg::warn(true,"CImg<%s>::load_jpeg() : Don't know how to read image '%s' with libpeg, trying ImageMagick's convert",
           pixel_type(),filename);
    return load_convert(filename);
      }
      
      const unsigned int row_stride = cinfo.output_width * cinfo.output_components;
      unsigned char *buf = new unsigned char[cinfo.output_width*cinfo.output_height*3], *buf2 = buf;
      JSAMPROW row_pointer[1];
      while (cinfo.output_scanline < cinfo.output_height) {
    row_pointer[0] = &buf[cinfo.output_scanline*row_stride];
    jpeg_read_scanlines(&cinfo,row_pointer,1);
      }
      jpeg_finish_decompress(&cinfo);
      jpeg_destroy_decompress(&cinfo);
      cimg::fclose(file);
      
      CImg<T> dest(cinfo.output_width,cinfo.output_height,1,3);
      T *ptr_r = dest.ptr(0,0,0,0),
    *ptr_g = dest.ptr(0,0,0,1),
    *ptr_b = dest.ptr(0,0,0,2);
      cimg_mapXY(dest,x,y) {
    *(ptr_r++) = *(buf2++);
    *(ptr_g++) = *(buf2++);
    *(ptr_b++) = *(buf2++);
      }
      delete[] buf;
      return dest;
#endif
    }
    
    //! Load an image from a RGBA file.
    static CImg load_rgba(const char *filename,const unsigned int dimw,const unsigned int dimh) {
      std::FILE *file = cimg::fopen(filename,"rb");
      unsigned char *buffer = new unsigned char[dimw*dimh*4];
      cimg::fread(buffer,sizeof(unsigned char),dimw*dimh*4,file);
      cimg::fclose(file);
      CImg res = CImg<T>(buffer,dimw,dimh,1,4,true);
      delete[] buffer;
      return res;      
    }

    //! Load an image from a RGB file.
    static CImg load_rgb(const char *filename,const unsigned int dimw,const unsigned int dimh) {
      std::FILE *file = cimg::fopen(filename,"rb");
      unsigned char *buffer = new unsigned char[dimw*dimh*3];
      cimg::fread(buffer,sizeof(unsigned char),dimw*dimh*3,file);
      cimg::fclose(file);
      CImg res = CImg<T>(buffer,dimw,dimh,1,3,true);
      delete[] buffer;
      return res;
    }
    
    //! Load an image from an INRIMAGE-4 file.
#define cimg_load_inr_case(Tf,sign,pixsize,Ts)                            \
  if (!loaded && fopt[6]==pixsize && fopt[4]==Tf && fopt[5]==sign) {      \
      Ts *xval, *val = new Ts[fopt[0]*fopt[3]];                           \
      cimg_mapYZ(dest,y,z) {                                              \
          cimg::fread(val,pixsize/8,fopt[0]*fopt[3],file);                \
          if (fopt[7]!=endian) cimg::endian_swap(val,fopt[0]*fopt[3]);    \
          xval = val; cimg_mapX(dest,x) cimg_mapV(dest,k)                 \
                          dest(x,y,z,k) = (T)*(xval++);                   \
        }                                                                 \
      delete[] val;                                                       \
      loaded = true;                                                      \
    }
    
    static void _load_inr(std::FILE *file,int out[8],float *voxsize=NULL) {
      char item[1024],tmp1[64],tmp2[64];
      out[0]=out[1]=out[2]=out[3]=out[5]=1; out[4]=out[6]=out[7]=-1;
      std::fscanf(file,"%63s",item);
      if(cimg::strncasecmp(item,"#INRIMAGE-4#{",13)!=0) 
    throw CImgIOException("CImg<%s>::load_inr() : File does not appear to be a valid INR file.\n"
                  "(INRIMAGE-4 identifier not found)",pixel_type());
      while (std::fscanf(file," %63[^\n]%*c",item)!=EOF && cimg::strncmp(item,"##}",3)) {
        std::sscanf(item," XDIM%*[^0-9]%d",out);
        std::sscanf(item," YDIM%*[^0-9]%d",out+1);
        std::sscanf(item," ZDIM%*[^0-9]%d",out+2);
        std::sscanf(item," VDIM%*[^0-9]%d",out+3);
        std::sscanf(item," PIXSIZE%*[^0-9]%d",out+6);
        if (voxsize) {
          std::sscanf(item," VX%*[^0-9.eE+-]%f",voxsize);
          std::sscanf(item," VY%*[^0-9.eE+-]%f",voxsize+1);
          std::sscanf(item," VZ%*[^0-9.eE+-]%f",voxsize+2);
        }
        if (std::sscanf(item," CPU%*[ =]%s",tmp1)) out[7]=cimg::strncasecmp(tmp1,"sun",3)?0:1;
        switch(std::sscanf(item," TYPE%*[ =]%s %s",tmp1,tmp2)) {
        case 0: break;
        case 2: out[5] = cimg::strncasecmp(tmp1,"unsigned",8)?1:0; std::strcpy(tmp1,tmp2);
        case 1:
          if (!cimg::strncasecmp(tmp1,"int",3)   || !cimg::strncasecmp(tmp1,"fixed",5))  out[4]=0;
          if (!cimg::strncasecmp(tmp1,"float",5) || !cimg::strncasecmp(tmp1,"double",6)) out[4]=1;
          if (!cimg::strncasecmp(tmp1,"packed",6))                                       out[4]=2;
          if (out[4]>=0) break;
        default: throw CImgIOException("cimg::inr_header_read() : Invalid TYPE '%s'",tmp2);
        }
      }
      if(out[0]<0 || out[1]<0 || out[2]<0 || out[3]<0)
        throw CImgIOException("CImg<%s>::load_inr() : Bad dimensions in .inr file = ( %d , %d , %d , %d )",
                              pixel_type(),out[0],out[1],out[2],out[3]);
      if(out[4]<0 || out[5]<0) throw CImgIOException("CImg<%s>::load_inr() : TYPE is not fully defined",pixel_type());
      if(out[6]<0) throw CImgIOException("CImg<%s>::load_inr() : PIXSIZE is not fully defined",pixel_type());
      if(out[7]<0) throw CImgIOException("CImg<%s>::load_inr() : Big/Little Endian coding type is not defined",pixel_type());
    }
    
    static CImg load_inr(const char *filename, float *voxsize = NULL) {
      std::FILE *file = cimg::fopen(filename,"rb");
      int fopt[8], endian=cimg::endian()?1:0;
      bool loaded = false;
      if (voxsize) voxsize[0]=voxsize[1]=voxsize[2]=1;
      _load_inr(file,fopt,voxsize);
      CImg<T> dest = CImg<T>(fopt[0],fopt[1],fopt[2],fopt[3]);
      cimg_load_inr_case(0,0,8, unsigned char);
      cimg_load_inr_case(0,1,8, char);
      cimg_load_inr_case(0,0,16,unsigned short);
      cimg_load_inr_case(0,1,16,short);
      cimg_load_inr_case(0,0,32,unsigned int);
      cimg_load_inr_case(0,1,32,int);
      cimg_load_inr_case(1,0,32,float);
      cimg_load_inr_case(1,1,32,float);
      cimg_load_inr_case(1,0,64,double);
      cimg_load_inr_case(1,1,64,double);
      if (!loaded) throw CImgIOException("CImg<%s>::load_inr() : File '%s', can't read images of the type specified in the file",
                     pixel_type(),filename);
      cimg::fclose(file);
      return dest;
    }
   
#define cimg_load_pandore_case(nid,nbdim,nwidth,nheight,ndepth,ndim,stype)  \
  case nid: {                                                         \
    cimg::fread(dims,sizeof(unsigned int),nbdim,file);                \
    if (endian) cimg::endian_swap(dims,nbdim);                        \
    dest = CImg<T>(nwidth,nheight,ndepth,ndim);                       \
    stype *buffer = new stype[dest.size()];                           \
    cimg::fread(buffer,sizeof(stype),dest.size(),file);               \
    if (endian) cimg::endian_swap(buffer,dest.size());                \
    T *ptrd = dest.ptr();                                             \
    cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));               \
    buffer-=dest.size();                                              \
    delete[] buffer;                                                  \
   }                                                                  \
   break;
    
    static CImg load_pandore(const char *filename) {
      std::FILE *file = cimg::fopen(filename,"rb");
      typedef unsigned char uchar;
      typedef unsigned short ushort;
      typedef unsigned int uint;  
      typedef unsigned long ulong; 
      CImg dest;
      char tmp[16];
      cimg::fread(tmp,sizeof(char),12,file);
      if (cimg::strncasecmp("PANDORE",tmp,7)) 
    throw CImgIOException("CImg<%s>::load_pandore() : File '%s' does not appear to be a valid PANDORE file.\n"
                  "(PANDORE identifier not found)",pixel_type(),filename);
      unsigned int id,dims[8];
      long ptbuf[4];
      cimg::fread(&id,sizeof(int),1,file);
      const bool endian = (id>255);
      if (endian) cimg::endian_swap(id);
      cimg::fread(tmp,sizeof(char),20,file);
      switch (id) {
    cimg_load_pandore_case(2,2,dims[1],1,1,1,uchar);
    cimg_load_pandore_case(3,2,dims[1],1,1,1,long);
    cimg_load_pandore_case(4,2,dims[1],1,1,1,float);
    cimg_load_pandore_case(5,3,dims[2],dims[1],1,1,uchar);
    cimg_load_pandore_case(6,3,dims[2],dims[1],1,1,long);
    cimg_load_pandore_case(7,3,dims[2],dims[1],1,1,float);
    cimg_load_pandore_case(8,4,dims[3],dims[2],dims[1],1,uchar);
    cimg_load_pandore_case(9,4,dims[3],dims[2],dims[1],1,long);
    cimg_load_pandore_case(10,4,dims[3],dims[2],dims[1],1,float);
      case 11: { // Region 1D
    cimg::fread(dims,sizeof(unsigned int),3,file);
    if (endian) cimg::endian_swap(dims,3);
    dest = CImg<T>(dims[1],1,1,1);
    if (dims[2]<256) {
      unsigned char *buffer = new unsigned char[dest.size()];
      cimg::fread(buffer,sizeof(unsigned char),dest.size(),file);
      T *ptrd = dest.ptr();
      cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
      buffer-=dest.size();
      delete[] buffer;
    } else {
      if (dims[2]<65536) {
        unsigned short *buffer = new unsigned short[dest.size()];
        cimg::fread(buffer,sizeof(unsigned short),dest.size(),file);
        if (endian) cimg::endian_swap(buffer,dest.size());
        T *ptrd = dest.ptr();
        cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
        buffer-=dest.size();
        delete[] buffer;
      } else {
        unsigned long *buffer = new unsigned long[dest.size()];
        cimg::fread(buffer,sizeof(unsigned long),dest.size(),file);
        if (endian) cimg::endian_swap(buffer,dest.size());
        T *ptrd = dest.ptr();
        cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
        buffer-=dest.size();
        delete[] buffer;
      }
    }    
      }
    break;
      case 12: { // Region 2D
    cimg::fread(dims,sizeof(unsigned int),4,file);
    if (endian) cimg::endian_swap(dims,4);
    dest = CImg<T>(dims[2],dims[1],1,1);
    if (dims[3]<256) {
      unsigned char *buffer = new unsigned char[dest.size()];
      cimg::fread(buffer,sizeof(unsigned char),dest.size(),file);
      T *ptrd = dest.ptr();
      cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
      buffer-=dest.size();
      delete[] buffer;
    } else {
      if (dims[3]<65536) {
        unsigned short *buffer = new unsigned short[dest.size()];
        cimg::fread(buffer,sizeof(unsigned short),dest.size(),file);
        if (endian) cimg::endian_swap(buffer,dest.size());
        T *ptrd = dest.ptr();
        cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
        buffer-=dest.size();
        delete[] buffer;
      } else {
        unsigned long *buffer = new unsigned long[dest.size()];
        cimg::fread(buffer,sizeof(unsigned long),dest.size(),file);
        if (endian) cimg::endian_swap(buffer,dest.size());
        T *ptrd = dest.ptr();
        cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
        buffer-=dest.size();
        delete[] buffer;
      }
    }    
      }
    break;
      case 13: { // Region 3D
    cimg::fread(dims,sizeof(unsigned int),5,file);
    if (endian) cimg::endian_swap(dims,5);
    dest = CImg<T>(dims[3],dims[2],dims[1],1);
    if (dims[4]<256) {
      unsigned char *buffer = new unsigned char[dest.size()];
      cimg::fread(buffer,sizeof(unsigned char),dest.size(),file);
      T *ptrd = dest.ptr();
      cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
      buffer-=dest.size();
      delete[] buffer;
    } else {
      if (dims[4]<65536) {
        unsigned short *buffer = new unsigned short[dest.size()];
        cimg::fread(buffer,sizeof(unsigned short),dest.size(),file);
        if (endian) cimg::endian_swap(buffer,dest.size());
        T *ptrd = dest.ptr();
        cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
        buffer-=dest.size();
        delete[] buffer;
      } else {
        unsigned long *buffer = new unsigned long[dest.size()];
        cimg::fread(buffer,sizeof(unsigned long),dest.size(),file);
        if (endian) cimg::endian_swap(buffer,dest.size());
        T *ptrd = dest.ptr();
        cimg_mapoff(dest,off) *(ptrd++) = (T)(*(buffer++));
        buffer-=dest.size();
        delete[] buffer;
      }
    }    
      }
    break;
    cimg_load_pandore_case(16,4,dims[2],dims[1],1,3,uchar);
    cimg_load_pandore_case(17,4,dims[2],dims[1],1,3,long);
    cimg_load_pandore_case(18,4,dims[2],dims[1],1,3,float);
    cimg_load_pandore_case(19,5,dims[3],dims[2],dims[1],3,uchar);
    cimg_load_pandore_case(20,5,dims[3],dims[2],dims[1],3,long);
    cimg_load_pandore_case(21,5,dims[3],dims[2],dims[1],3,float);
    cimg_load_pandore_case(22,2,dims[1],1,1,dims[0],uchar);
    cimg_load_pandore_case(23,2,dims[1],1,1,dims[0],long);
    cimg_load_pandore_case(24,2,dims[1],1,1,dims[0],ulong);
    cimg_load_pandore_case(25,2,dims[1],1,1,dims[0],float);
    cimg_load_pandore_case(26,3,dims[2],dims[1],1,dims[0],uchar);
    cimg_load_pandore_case(27,3,dims[2],dims[1],1,dims[0],long);
    cimg_load_pandore_case(28,3,dims[2],dims[1],1,dims[0],ulong);
    cimg_load_pandore_case(29,3,dims[2],dims[1],1,dims[0],float);
    cimg_load_pandore_case(30,4,dims[3],dims[2],dims[1],dims[0],uchar);
    cimg_load_pandore_case(31,4,dims[3],dims[2],dims[1],dims[0],long);
    cimg_load_pandore_case(32,4,dims[3],dims[2],dims[1],dims[0],ulong);
    cimg_load_pandore_case(33,4,dims[3],dims[2],dims[1],dims[0],float);    
      case 34: // Points 1D    
    cimg::fread(ptbuf,sizeof(long),1,file);
    if (endian) cimg::endian_swap(ptbuf,1);
    dest = CImg<T>(1); dest[0]=(T)ptbuf[0];
    break;
      case 35: // Points 2D
    cimg::fread(ptbuf,sizeof(long),2,file);
    if (endian) cimg::endian_swap(ptbuf,2);
    dest = CImg<T>(2); dest[0]=(T)ptbuf[1]; dest[1]=(T)ptbuf[0];
    break;
      case 36: // Points 3D
    cimg::fread(ptbuf,sizeof(long),3,file);
    if (endian) cimg::endian_swap(ptbuf,3);
    dest = CImg<T>(3); dest[0]=(T)ptbuf[2]; dest[1]=(T)ptbuf[1]; dest[2]=(T)ptbuf[0];
    break;
      default:
    throw CImgIOException("CImg<%s>::load_pandore() : File '%s', can't read images with ID_type=%d",pixel_type(),filename,id);
      }
      return dest;
    }


    //! Load an image from an ANALYZE7.5 file
    static CImg load_analyze(const char *filename, float *voxsize = NULL) {
      
      // Open header and data files
      std::FILE *file_header=NULL, *file=NULL;
      char body[1024];
      const char *ext = cimg::filename_split(filename,body);
      if (!cimg::strcasecmp(ext,"hdr") || !cimg::strcasecmp(ext,"img")) {
    std::sprintf(body+cimg::strlen(body),".hdr");
    file_header = cimg::fopen(body,"rb");
    std::sprintf(body+cimg::strlen(body)-3,"img");
    file = cimg::fopen(body,"rb");
      } else throw CImgIOException("CImg<%s>::load_analyze() : Cannot load filename '%s' as an analyze format",pixel_type(),filename);

      // Read header
      bool endian = false;
      unsigned int header_size;
      cimg::fread(&header_size,sizeof(int),1,file_header);
      if (header_size>=4096) { endian = true; cimg::endian_swap(header_size); }
      unsigned char *header = new unsigned char[header_size];
      cimg::fread(header+sizeof(int),sizeof(char),header_size-sizeof(int),file_header);
      cimg::fclose(file_header);
      if (endian) {
    cimg::endian_swap((short*)(header+40),5);
        cimg::endian_swap((short*)(header+70),1);
        cimg::endian_swap((short*)(header+72),1);
        cimg::endian_swap((float*)(header+76),4);
        cimg::endian_swap((float*)(header+112),1);
      }
      unsigned short *dim = (unsigned short*)(header+40), dimx=1, dimy=1, dimz=1, dimv=1;
      cimg::warn(!dim[0],"CImg<%s>::load_analyze() : Specified image has zero dimensions.",pixel_type());
      cimg::warn(dim[0]>4,"CImg<%s>::load_analyze() : Number of image dimension is %d, reading only the 4 first dimensions",
         pixel_type(),dim[0]);
      if (dim[0]>=1) dimx = dim[1];
      if (dim[0]>=2) dimy = dim[2];
      if (dim[0]>=3) dimz = dim[3];
      if (dim[0]>=4) dimv = dim[4];
      
      float scalefactor = *(float*)(header+112); if (scalefactor==0) scalefactor=1;
      const unsigned short datatype = *(short*)(header+70);
      if (voxsize) { const float *vsize = (float*)(header+76); voxsize[0] = vsize[1]; voxsize[1] = vsize[2]; voxsize[2] = vsize[3]; }
      delete[] header;

      // Read pixel data
      CImg dest(dimx,dimy,dimz,dimv);
      switch (datatype) {
      case 2: {
    unsigned char *buffer = new unsigned char[dimx*dimy*dimz*dimv];
    cimg::fread(buffer,sizeof(unsigned char),dimx*dimy*dimz*dimv,file);
    cimg_mapoff(dest,off) dest.data[off] = (T)(buffer[off]*scalefactor);
    delete[] buffer;
      } break;
      case 4: {
    short *buffer = new short[dimx*dimy*dimz*dimv];
    cimg::fread(buffer,sizeof(short),dimx*dimy*dimz*dimv,file);
    if (endian) cimg::endian_swap(buffer,dimx*dimy*dimz*dimv);
    cimg_mapoff(dest,off) dest.data[off] = (T)(buffer[off]*scalefactor);
    delete[] buffer;
      } break;
      case 8: {
    int *buffer = new int[dimx*dimy*dimz*dimv];
    cimg::fread(buffer,sizeof(int),dimx*dimy*dimz*dimv,file);
    if (endian) cimg::endian_swap(buffer,dimx*dimy*dimz*dimv);
    cimg_mapoff(dest,off) dest.data[off] = (T)(buffer[off]*scalefactor);
    delete[] buffer;
      } break;
      case 16: {
    float *buffer = new float[dimx*dimy*dimz*dimv];
    cimg::fread(buffer,sizeof(float),dimx*dimy*dimz*dimv,file);
    if (endian) cimg::endian_swap(buffer,dimx*dimy*dimz*dimv);
    cimg_mapoff(dest,off) dest.data[off] = (T)(buffer[off]*scalefactor);
    delete[] buffer;
      } break;
      case 64: {
    double *buffer = new double[dimx*dimy*dimz*dimv];
    cimg::fread(buffer,sizeof(double),dimx*dimy*dimz*dimv,file);
    if (endian) cimg::endian_swap(buffer,dimx*dimy*dimz*dimv);
    cimg_mapoff(dest,off) dest.data[off] = (T)(buffer[off]*scalefactor);
    delete[] buffer;
      } break;
      default: throw CImgIOException("CImg<%s>::load_analyze() : Cannot read images width 'datatype = %d'",pixel_type(),datatype);
      }
      cimg::fclose(file);
      return dest;
    }

    //! Load an image from a RAW file
    static CImg load_raw(const char *filename,const char axe='v',const char align='p') { 
      return CImgl<T>(filename).get_append(axe,align); 
    }

    //! Function that loads the image for other file formats that are not natively handled by CImg, using the tool 'convert' from the ImageMagick package.\n
    //! This is the case for all compressed image formats (GIF,PNG,JPG,TIF,...). You need to install the ImageMagick package in order to get
    //! this function working properly (see http://www.imagemagick.org ).
    static CImg load_convert(const char *filename) {
      static bool first_time = true;
      char command[512], filetmp[512];     
      if (first_time) { std::srand((unsigned int)::time(NULL)); first_time = false; }
      std::FILE *file = NULL;
      do { 
    if (file) std::fclose(file);
    std::sprintf(filetmp,"%s/CImg%.4d.ppm",cimg::temporary_path(),std::rand()%10000);
    file = std::fopen(filetmp,"rb");
      } while (file);
      std::sprintf(command,"\"%s\" \"%s\" %s",cimg::convert_path(),filename,filetmp);
      cimg::system(command);
      file = std::fopen(filetmp,"rb");
      if (!file) {
        std::fclose(cimg::fopen(filename,"r"));
        throw CImgIOException("CImg<%s>::load_convert() : Failed to open image '%s' with 'convert'.\n"
                  "Check that you have installed the ImageMagick package in a standart directory.",
                  pixel_type(),filename);
      } else cimg::fclose(file);
      const CImg dest(filetmp);
      std::remove(filetmp);
      return dest;
    }

    //! Save the image as a file. 
    /**
       The used file format is defined by the file extension in the filename \p filename.\n
       Parameter \p number can be used to add a 6-digit number to the filename before saving.\n
       If \p normalize is true, a normalized version of the image (between [0,255]) is saved.
    **/
    const CImg& save(const char *filename,const int number=-1) const {
      cimg_test(*this,"CImg<T>::save");
      if (!filename) throw CImgArgumentException("CImg<%s>::save() : Can't save with (null) filename",pixel_type());
      const char *ext = cimg::filename_split(filename);
      char nfilename[1024];
      if (number>=0) filename = cimg::filename_number(filename,number,6,nfilename);
      if (!cimg::strcasecmp(ext,"asc")) return save_ascii(filename);
      if (!cimg::strcasecmp(ext,"dlm")) return save_dlm(filename);
      if (!cimg::strcasecmp(ext,"inr")) return save_inr(filename);
      if (!cimg::strcasecmp(ext,"hdr")) return save_analyze(filename);
      if (!cimg::strcasecmp(ext,"pan")) return save_pandore(filename);
      if (!cimg::strcasecmp(ext,"bmp")) return save_bmp(filename);
      if (!cimg::strcasecmp(ext,"png")) return save_png(filename);
      if (!cimg::strcasecmp(ext,"jpg") ||
      !cimg::strcasecmp(ext,"jpeg")) return save_jpeg(filename);
      if (!cimg::strcasecmp(ext,"rgba")) return save_rgba(filename);
      if (!cimg::strcasecmp(ext,"rgb")) return save_rgb(filename);
      if (!cimg::strcasecmp(ext,"raw") || ext[0]=='\0') return save_raw(filename);
      if (!cimg::strcasecmp(ext,"pgm") || 
      !cimg::strcasecmp(ext,"ppm") || 
      !cimg::strcasecmp(ext,"pnm")) return save_pnm(filename);
      return save_convert(filename);
    }
  
    //! Save the image as an ASCII file.
    const CImg& save_ascii(const char *filename) const {
      cimg_test(*this,"CImg<T>::save_ascii");
      std::FILE *file = cimg::fopen(filename,"w");
      std::fprintf(file,"%u %u %u %u\n",width,height,depth,dim);
      const T* ptrs = data;
      cimg_mapYZV(*this,y,z,v) {
    cimg_mapX(*this,x) std::fprintf(file,"%g ",(double)*(ptrs++));
    std::fputc('\n',file);
      }
      cimg::fclose(file);
      return *this;
    }

    //! Save the image as a DLM file.
    const CImg& save_dlm(const char *filename) const {
      cimg_test(*this,"CImg<T>::save_dlm");
      std::FILE *file = cimg::fopen(filename,"w");
      const T* ptrs = data;
      cimg_mapYZV(*this,y,z,v) {
    cimg_mapX(*this,x) std::fprintf(file,"%g%s",(double)*(ptrs++),(x==(int)width-1)?"":",");
    std::fputc('\n',file);
      }
      cimg::fclose(file);
      return *this;
    }

    //! Save the image as a PNM file.
    const CImg& save_pnm(const char *filename) const {
      cimg_test(*this,"CImg<T>::save_pnm");
      const char *ext = cimg::filename_split(filename);
      const CImgStats st(*this,false);

      if (dim>1 && !cimg::strcasecmp(ext,"pgm")) {
            get_norm_pointwise().normalize(0.0f,(float)st.max).save_pnm(filename); 
            return *this; 
      }
      std::FILE *file = cimg::fopen(filename,"wb");
      const T 
    *ptrR = ptr(0,0,0,0),
    *ptrG = (dim>=2)?ptr(0,0,0,1):ptrR,
    *ptrB = (dim>=3)?ptr(0,0,0,2):ptrR;
      const unsigned int buf_size = width*height*(dim==1?1:3);

      std::fprintf(file,"P%c\n# CREATOR: CImg : Original size=%ux%ux%ux%u\n%u %u\n%u\n",
           (dim==1?'5':'6'),width,height,depth,dim,width,height,(st.max)<256?255:65535);
      
      switch(dim) {
      case 1: {
    if ((st.max)<256) { // Binary PGM 8 bits
      unsigned char *ptrd = new unsigned char[buf_size], *xptrd = ptrd;
      cimg_mapXY(*this,x,y) *(xptrd++) = (unsigned char)*(ptrR++);
      cimg::fwrite(ptrd,sizeof(unsigned char),buf_size,file);
      delete[] ptrd;
    } else {             // Binary PGM 16 bits
      unsigned short *ptrd = new unsigned short[buf_size], *xptrd = ptrd;
      cimg_mapXY(*this,x,y) *(xptrd++) = (unsigned short)*(ptrR++);
      cimg::fwrite(ptrd,sizeof(unsigned short),buf_size,file);
      delete[] ptrd;
    }
      } break;
      default: {
    if ((st.max)<256) { // Binary PPM 8 bits
      unsigned char *ptrd = new unsigned char[buf_size], *xptrd = ptrd;
      cimg_mapXY(*this,x,y) {
        *(xptrd++) = (unsigned char)*(ptrR++);
        *(xptrd++) = (unsigned char)*(ptrG++);
        *(xptrd++) = (unsigned char)*(ptrB++);
      }
      cimg::fwrite(ptrd,sizeof(unsigned char),buf_size,file);
      delete[] ptrd;
    } else {             // Binary PPM 16 bits
      unsigned short *ptrd = new unsigned short[buf_size], *xptrd = ptrd;
      cimg_mapXY(*this,x,y) {
        *(xptrd++) = (unsigned short)*(ptrR++);
        *(xptrd++) = (unsigned short)*(ptrG++);
        *(xptrd++) = (unsigned short)*(ptrB++);
      }
      cimg::fwrite(ptrd,sizeof(unsigned short),buf_size,file);
      delete[] ptrd;
    }
      } break;
      }
      cimg::fclose(file);
      
      return *this;
    }
    
    //! Save the image as an ANALYZE7.5 file.
    const CImg& save_analyze(const char *filename,const float *const voxsize=NULL) const {
      cimg_test(*this,"CImg<T>::save_analyze");
      std::FILE *file;
      char header[348],hname[1024],iname[1024];
      const char *ext = cimg::filename_split(filename);
      short datatype=-1;
      std::memset(header,0,348);
      if (!ext[0]) { std::sprintf(hname,"%s.hdr",filename); std::sprintf(iname,"%s.img",filename); }
      if (!cimg::strncasecmp(ext,"hdr",3)) { 
    std::strcpy(hname,filename); std::strcpy(iname,filename); std::sprintf(iname+cimg::strlen(iname)-3,"img"); 
      }
      if (!cimg::strncasecmp(ext,"img",3)) {
    std::strcpy(hname,filename); std::strcpy(iname,filename); std::sprintf(hname+cimg::strlen(iname)-3,"hdr"); 
      }
      ((int*)(header))[0] = 348;
      std::sprintf(header+4,"CImg");
      std::sprintf(header+14," ");
      ((short*)(header+36))[0] = 4096;
      ((char*)(header+38))[0] = 114;
      ((short*)(header+40))[0] = 4;
      ((short*)(header+40))[1] = width;
      ((short*)(header+40))[2] = height;
      ((short*)(header+40))[3] = depth;
      ((short*)(header+40))[4] = dim;
      if (!cimg::strcasecmp(pixel_type(),"unsigned char"))  datatype = 2;
      if (!cimg::strcasecmp(pixel_type(),"short"))          datatype = 4;
      if (!cimg::strcasecmp(pixel_type(),"int"))            datatype = 8;
      if (!cimg::strcasecmp(pixel_type(),"float"))          datatype = 16;
      if (!cimg::strcasecmp(pixel_type(),"double"))         datatype = 64;
      ((short*)(header+70))[0] = datatype;
      ((short*)(header+72))[0] = sizeof(T);
      ((float*)(header+112))[0] = 1;
      ((float*)(header+76))[0] = 0;
      if (voxsize) {
        ((float*)(header+76))[1] = voxsize[0];
        ((float*)(header+76))[2] = voxsize[1];
        ((float*)(header+76))[3] = voxsize[2];
      } else ((float*)(header+76))[1] = ((float*)(header+76))[2] = ((float*)(header+76))[3] = 1;
      file = cimg::fopen(hname,"wb");
      cimg::fwrite(header,sizeof(char),348,file);
      cimg::fclose(file);
      file = cimg::fopen(iname,"wb");
      cimg::fwrite(data,sizeof(T),size(),file);
      cimg::fclose(file);
      return *this;
    }

    //! Save the image as a RAW file
    const CImg& save_raw(const char *filename) const {
      cimg_test(*this,"CImg<T>::save_raw");      
      CImgl<T> shared(1);
      shared[0].width = width;
      shared[0].height = height;
      shared[0].depth = depth;
      shared[0].dim = dim;
      shared[0].data = data;
      shared.save_raw(filename);
      shared[0].width = shared[0].height = shared[0].depth = shared[0].dim = 0;
      shared[0].data = NULL;
      return *this;
    }
 
    //! Save the image using ImageMagick's convert.    
    /** Function that saves the image for other file formats that are not natively handled by CImg,
    using the tool 'convert' from the ImageMagick package.\n
    This is the case for all compressed image formats (GIF,PNG,JPG,TIF,...). You need to install 
    the ImageMagick package in order to get
    this function working properly (see http://www.imagemagick.org ).
    **/
    const CImg& save_convert(const char *filename) const {
      cimg_test(*this,"CImg<T>::save_convert");
      static bool first_time = true;
      char command[512],filetmp[512];
      if (first_time) { std::srand((unsigned int)::time(NULL)); first_time = false; }
      std::FILE *file = NULL;
      do {
    if (file) std::fclose(file);
    std::sprintf(filetmp,"%s/CImg%.4d.rgba",cimg::temporary_path(),std::rand()%10000);
    file = std::fopen(filetmp,"rb");
      } while (file);
      save_rgba(filetmp);
      std::sprintf(command,"\"%s\" -depth 8 -size %ux%u -quality 100%% \"%s\" %s",
           cimg::convert_path(),width,height,filetmp,filename);
      cimg::system(command);
      file = std::fopen(filename,"rb");
      if (!file) throw CImgIOException("CImg<%s>::save_convert() : Failed to save image '%s' with 'convert'.\n"
                       "Check that you have installed the ImageMagick package in a standart directory.",
                       pixel_type(),filename);
      if (file) cimg::fclose(file);
      std::remove(filetmp);
      return *this;
    }
    
    //! Save the image as an INRIMAGE-4 file.
    const CImg& save_inr(const char *filename,const float *const voxsize = NULL) const {
      cimg_test(*this,"CImg<T>::save_inr");
      int inrpixsize=-1;
      const char *inrtype = "unsigned fixed\nPIXSIZE=8 bits\nSCALE=2**0";
      if (!cimg::strcasecmp(pixel_type(),"unsigned char"))  { inrtype = "unsigned fixed\nPIXSIZE=8 bits\nSCALE=2**0"; inrpixsize = 1; }
      if (!cimg::strcasecmp(pixel_type(),"char"))           { inrtype = "fixed\nPIXSIZE=8 bits\nSCALE=2**0"; inrpixsize = 1; }
      if (!cimg::strcasecmp(pixel_type(),"unsigned short")) { inrtype = "unsigned fixed\nPIXSIZE=16 bits\nSCALE=2**0";inrpixsize = 2; }
      if (!cimg::strcasecmp(pixel_type(),"short"))          { inrtype = "fixed\nPIXSIZE=16 bits\nSCALE=2**0"; inrpixsize = 2; }
      if (!cimg::strcasecmp(pixel_type(),"unsigned int"))   { inrtype = "unsigned fixed\nPIXSIZE=32 bits\nSCALE=2**0";inrpixsize = 4; }
      if (!cimg::strcasecmp(pixel_type(),"int"))            { inrtype = "fixed\nPIXSIZE=32 bits\nSCALE=2**0"; inrpixsize = 4; }
      if (!cimg::strcasecmp(pixel_type(),"float"))          { inrtype = "float\nPIXSIZE=32 bits"; inrpixsize = 4; }
      if (!cimg::strcasecmp(pixel_type(),"double"))         { inrtype = "float\nPIXSIZE=64 bits"; inrpixsize = 8; }
      if (inrpixsize<=0) throw CImgIOException("CImg<%s>::save_inr() : Don't know how to save images of '%s'",pixel_type(),pixel_type());
      std::FILE *file = cimg::fopen(filename,"wb");
      char header[257];      
      int err = std::sprintf(header,"#INRIMAGE-4#{\nXDIM=%u\nYDIM=%u\nZDIM=%u\nVDIM=%u\n",width,height,depth,dim);
      if (voxsize) err += std::sprintf(header+err,"VX=%g\nVY=%g\nVZ=%g\n",voxsize[0],voxsize[1],voxsize[2]);
      err += std::sprintf(header+err,"TYPE=%s\nCPU=%s\n",inrtype,cimg::endian()?"sun":"decm");
      std::memset(header+err,'\n',252-err);
      std::memcpy(header+252,"##}\n",4);
      cimg::fwrite(header,sizeof(char),256,file);
      cimg_mapXYZ(*this,x,y,z) cimg_mapV(*this,k) cimg::fwrite(&((*this)(x,y,z,k)),inrpixsize,1,file);
      cimg::fclose(file);
      return *this;
    }

#define cimg_save_pandore_case(sy,sz,sv,stype,id)                           \
   if (!saved && (sy?(sy==height):true) && (sz?(sz==depth):true) && (sv?(sv==dim):true) && !strcmp(stype,pixel_type())) { \
      unsigned int *iheader = (unsigned int*)(header+12);                   \
      nbdims = _save_pandore_header_length((*iheader=id),dims);             \
      cimg::fwrite(header,sizeof(unsigned char),36,file);                   \
      cimg::fwrite(dims,sizeof(unsigned int),nbdims,file);                  \
      if (id==2 || id==5 || id==8 || id==16 || id==19 || id==22 || id==26 || id==30) { \
    unsigned char *buffer = new unsigned char[size()];                  \
    T *ptrs = ptr();                                                    \
    cimg_mapoff(*this,off) *(buffer++)=(unsigned char)(*(ptrs++));      \
    buffer-=size();                                                     \
    cimg::fwrite(buffer,sizeof(unsigned char),size(),file);             \
    delete[] buffer;                                                    \
      }                                                                     \
      if (id==3 || id==6 || id==9 || id==17 || id==20 || id==23 || id==27 || id==31) { \
    unsigned long *buffer = new unsigned long[size()];                  \
    T *ptrs = ptr();                                                    \
    cimg_mapoff(*this,off) *(buffer++)=(long)(*(ptrs++));               \
    buffer-=size();                                                     \
    cimg::fwrite(buffer,sizeof(long),size(),file);                      \
    delete[] buffer;                                                    \
      }                                                                     \
      if (id==4 || id==7 || id==10 || id==18 || id==21 || id==25 || id==29 || id==33) { \
    float *buffer = new float[size()];                                  \
    T *ptrs = ptr();                                                    \
    cimg_mapoff(*this,off) *(buffer++)=(float)(*(ptrs++));              \
    buffer-=size();                                                     \
    cimg::fwrite(buffer,sizeof(float),size(),file);                     \
    delete[] buffer;                                                    \
      }                                                                     \
      saved = true;                                                         \
    }

    unsigned int _save_pandore_header_length(unsigned int id,unsigned int *dims) const {
      unsigned int nbdims=0;
      if (id==2 || id==3 || id==4)    { dims[0]=1; dims[1]=width; nbdims=2; }
      if (id==5 || id==6 || id==7)    { dims[0]=1; dims[1]=height; dims[2]=width; nbdims=3; }
      if (id==8 || id==9 || id==10)   { dims[0]=dim; dims[1]=depth; dims[2]=height; dims[3]=width; nbdims=4; }
      if (id==16 || id==17 || id==18) { dims[0]=3; dims[1]=height; dims[2]=width; dims[3]=1; nbdims=4; }
      if (id==19 || id==20 || id==21) { dims[0]=3; dims[1]=depth; dims[2]=height; dims[3]=width; dims[4]=0; nbdims=5; }
      if (id==22 || id==23 || id==25) { dims[0]=dim; dims[1]=width; nbdims=2; }
      if (id==26 || id==27 || id==29) { dims[0]=dim; dims[1]=height; dims[2]=width; nbdims=3; }
      if (id==30 || id==31 || id==33) { dims[0]=dim; dims[1]=depth; dims[2]=height; dims[3]=width; nbdims=4; }
      return nbdims;
    }    

    //! Save the image as a PANDORE-5 file.
    const CImg& save_pandore(const char* filename) const {
      cimg_test(*this,"CImg<T>::save_pandore");
      std::FILE *file = cimg::fopen(filename,"wb");
      unsigned char header[36] = { 'P','A','N','D','O','R','E','0','4',0,0,0,
                   0,0,0,0,'C','I','m','g',0,0,0,0,0,
                   '2','0','0','0','/','0','1','/','0','1',
                   0 };
      unsigned int nbdims,dims[5];
      bool saved=false;
      cimg_save_pandore_case(1,1,1,"unsigned char",2);
      cimg_save_pandore_case(1,1,1,"char",3);
      cimg_save_pandore_case(1,1,1,"short",3);
      cimg_save_pandore_case(1,1,1,"unsigned short",3);
      cimg_save_pandore_case(1,1,1,"unsigned int",3);
      cimg_save_pandore_case(1,1,1,"int",3);
      cimg_save_pandore_case(1,1,1,"unsigned long",4);
      cimg_save_pandore_case(1,1,1,"long",3);
      cimg_save_pandore_case(1,1,1,"float",4);
      cimg_save_pandore_case(1,1,1,"double",4);
 
      cimg_save_pandore_case(0,1,1,"unsigned char",5);
      cimg_save_pandore_case(0,1,1,"char",6);
      cimg_save_pandore_case(0,1,1,"short",6);
      cimg_save_pandore_case(0,1,1,"unsigned short",6);
      cimg_save_pandore_case(0,1,1,"unsigned int",6);
      cimg_save_pandore_case(0,1,1,"int",6);
      cimg_save_pandore_case(0,1,1,"unsigned long",7);
      cimg_save_pandore_case(0,1,1,"long",6);
      cimg_save_pandore_case(0,1,1,"float",7);
      cimg_save_pandore_case(0,1,1,"double",7);

      cimg_save_pandore_case(0,0,1,"unsigned char",8);
      cimg_save_pandore_case(0,0,1,"char",9);
      cimg_save_pandore_case(0,0,1,"short",9);
      cimg_save_pandore_case(0,0,1,"unsigned short",9);
      cimg_save_pandore_case(0,0,1,"unsigned int",9);
      cimg_save_pandore_case(0,0,1,"int",9);
      cimg_save_pandore_case(0,0,1,"unsigned long",10);
      cimg_save_pandore_case(0,0,1,"long",9);
      cimg_save_pandore_case(0,0,1,"float",10);
      cimg_save_pandore_case(0,0,1,"double",10);
      
      cimg_save_pandore_case(0,1,3,"unsigned char",16);
      cimg_save_pandore_case(0,1,3,"char",17);
      cimg_save_pandore_case(0,1,3,"short",17);
      cimg_save_pandore_case(0,1,3,"unsigned short",17);
      cimg_save_pandore_case(0,1,3,"unsigned int",17);
      cimg_save_pandore_case(0,1,3,"int",17);
      cimg_save_pandore_case(0,1,3,"unsigned long",18);
      cimg_save_pandore_case(0,1,3,"long",17);
      cimg_save_pandore_case(0,1,3,"float",18);
      cimg_save_pandore_case(0,1,3,"double",18);

      cimg_save_pandore_case(0,0,3,"unsigned char",19);
      cimg_save_pandore_case(0,0,3,"char",20);
      cimg_save_pandore_case(0,0,3,"short",20);
      cimg_save_pandore_case(0,0,3,"unsigned short",20);
      cimg_save_pandore_case(0,0,3,"unsigned int",20);
      cimg_save_pandore_case(0,0,3,"int",20);
      cimg_save_pandore_case(0,0,3,"unsigned long",21);
      cimg_save_pandore_case(0,0,3,"long",20);
      cimg_save_pandore_case(0,0,3,"float",21);
      cimg_save_pandore_case(0,0,3,"double",21);
     
      cimg_save_pandore_case(1,1,0,"unsigned char",22);
      cimg_save_pandore_case(1,1,0,"char",23);
      cimg_save_pandore_case(1,1,0,"short",23);
      cimg_save_pandore_case(1,1,0,"unsigned short",23);
      cimg_save_pandore_case(1,1,0,"unsigned int",23);
      cimg_save_pandore_case(1,1,0,"int",23);
      cimg_save_pandore_case(1,1,0,"unsigned long",25);
      cimg_save_pandore_case(1,1,0,"long",23);
      cimg_save_pandore_case(1,1,0,"float",25);
      cimg_save_pandore_case(1,1,0,"double",25);
 
      cimg_save_pandore_case(0,1,0,"unsigned char",26);
      cimg_save_pandore_case(0,1,0,"char",27);
      cimg_save_pandore_case(0,1,0,"short",27);
      cimg_save_pandore_case(0,1,0,"unsigned short",27);
      cimg_save_pandore_case(0,1,0,"unsigned int",27);
      cimg_save_pandore_case(0,1,0,"int",27);
      cimg_save_pandore_case(0,1,0,"unsigned long",29);
      cimg_save_pandore_case(0,1,0,"long",27);
      cimg_save_pandore_case(0,1,0,"float",29);
      cimg_save_pandore_case(0,1,0,"double",29);

      cimg_save_pandore_case(0,0,0,"unsigned char",30);
      cimg_save_pandore_case(0,0,0,"char",31);
      cimg_save_pandore_case(0,0,0,"short",31);
      cimg_save_pandore_case(0,0,0,"unsigned short",31);
      cimg_save_pandore_case(0,0,0,"unsigned int",31);
      cimg_save_pandore_case(0,0,0,"int",31);
      cimg_save_pandore_case(0,0,0,"unsigned long",33);
      cimg_save_pandore_case(0,0,0,"long",31);
      cimg_save_pandore_case(0,0,0,"float",33);
      cimg_save_pandore_case(0,0,0,"double",33);

      cimg::fclose(file);
      return *this;
    }

    //! Save the image as a BMP file
    const CImg& save_bmp(const char* filename) const {
      cimg_test(*this,"CImg<T>::save_bmp");
      std::FILE *file = cimg::fopen(filename,"wb");
      
      unsigned char header[54]={0}, align_buf[4]={0};
      const unsigned int 
    align     = (4-(3*width)%4)%4,
    buf_size  = (3*width+align)*dimy(),
    file_size = 54+buf_size;
      header[0] = 'B'; header[1] = 'M';
      header[0x02]=file_size&0xFF; header[0x03]=(file_size>>8)&0xFF;
      header[0x04]=(file_size>>16)&0xFF; header[0x05]=(file_size>>24)&0xFF;
      header[0x0A]=0x36;
      header[0x0E]=0x28;
      header[0x12]=width&0xFF; header[0x13]=(width>>8)&0xFF;
      header[0x14]=(width>>16)&0xFF; header[0x15]=(width>>24)&0xFF;
      header[0x16]=height&0xFF; header[0x17]=(height>>8)&0xFF;
      header[0x18]=(height>>16)&0xFF; header[0x19]=(height>>24)&0xFF;
      header[0x1A]=1;  header[0x1B]=0;
      header[0x1C]=24; header[0x1D]=0;
      header[0x22]=buf_size&0xFF; header[0x23]=(buf_size>>8)&0xFF;
      header[0x24]=(buf_size>>16)&0xFF; header[0x25]=(buf_size>>24)&0xFF;
      header[0x27]=0x1; header[0x2B]=0x1;
      cimg::fwrite(header,sizeof(unsigned char),54,file);

      const T
    *pR = ptr(0,height-1,0,0),
    *pG = (dim>=2)?ptr(0,height-1,0,1):pR, 
    *pB = (dim>=3)?ptr(0,height-1,0,2):pR;

      cimg_mapY(*this,y) {
    cimg_mapX(*this,x) {
      std::fputc((unsigned char)(*(pB++)),file);
      std::fputc((unsigned char)(*(pG++)),file);
      std::fputc((unsigned char)(*(pR++)),file);
    }
    std::fwrite(align_buf,sizeof(unsigned char),align,file);
    pR-=2*width; pG-=2*width; pB-=2*width;    
      }      
      cimg::fclose(file);
      return *this;
    }
    
    //! Save an image to a PNG file.
    // Most of this function has been written by Eric Fausett
    /**
       \param filename = name of the png image file to load
       \param png16 = specifies wether or not to use 16 bit per channel png format for saving.
       \return *this
       \note The png format specifies a variety of possible data formats.  Grey scale, Grey
       scale with Alpha, RGB color, RGB color with Alpha, and Palletized color are supported.
       Per channel bit depths of 1, 2, 4, 8, and 16 are natively supported.  In this saving function
       only 8 and 16 bit channel depths are supported based upon the bool parameter /c png16.  The
       type of file saved depends on the number of channels in the CImg file.  If there is 4 or more
       channels, the image will be saved as an RGB color with Alpha image using the bottom 4 channels.
       If there are 3 channels, the saved image will be an RGB color image.  If 2 channels then the
       image saved will be Grey scale with Alpha, and if 1 channel will be saved as a Grey scale
       image.
    **/
    const CImg& save_png(const char* filename, const bool png16=false) const {
      cimg_test(*this,"CImg<T>::save_png");
#ifndef cimg_use_png
      return save_convert(filename);
#else
      std::FILE *file = cimg::fopen(filename,"wb");
      
      // Setup PNG structures for write
      png_voidp user_error_ptr=0;
      png_error_ptr user_error_fn=0, user_warning_fn=0;
      png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                            user_error_ptr, user_error_fn, user_warning_fn);
      if(!png_ptr){
    cimg::fclose(file);
    throw CImgIOException("CImg<%s>::save_png() : trouble initializing 'png_ptr' data structure",pixel_type());
      }
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if(!info_ptr){
    png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
    cimg::fclose(file);
    throw CImgIOException("CImg<%s>::save_png() : trouble initializing 'info_ptr' data structure",pixel_type());
      }
      if (setjmp(png_jmpbuf(png_ptr))){
    png_destroy_write_struct(&png_ptr, &info_ptr);
    cimg::fclose(file);
    throw CImgIOException("CImg<%s>::save_png() : unspecified error reading PNG file '%s'",pixel_type(),filename);
      }
      
      png_init_io(png_ptr, file);      
      png_uint_32 width = dimx();
      png_uint_32 height = dimy();
      const int bit_depth = png16?16:8;
      int color_type;
      switch (dimv()) {
      case 1: color_type = PNG_COLOR_TYPE_GRAY; break;
      case 2: color_type = PNG_COLOR_TYPE_GRAY_ALPHA; break;
      case 3: color_type = PNG_COLOR_TYPE_RGB; break;
      default: color_type = PNG_COLOR_TYPE_RGB_ALPHA;
      }
      const int interlace_type = PNG_INTERLACE_NONE;
      const int compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
      const int filter_method = PNG_FILTER_TYPE_DEFAULT;
      png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, interlace_type,
           compression_type, filter_method);
      png_write_info(png_ptr, info_ptr);
      const int byte_depth = bit_depth>>3;
      const int numChan = dimv()>4?4:dimv();
      const int pixel_bit_depth_flag = numChan * (bit_depth-1);
      
      // Allocate Memory for Image Save and Fill pixel data
      png_bytep *imgData = new png_byte*[height];
      for(unsigned int row=0; row<height; row++) imgData[row] = new png_byte[byte_depth * numChan * width];
      const T *pC0 = ptr(0,0,0,0);
      switch(pixel_bit_depth_flag) {
      case 7 :    { // Gray 8-bit
    cimg_mapY(*this,y) {
      unsigned char *ptrs = imgData[y];
      cimg_mapX(*this,x) *(ptrs++) = (unsigned char)*(pC0++);
    }
      } break;
      case 14: { // Gray w/ Alpha 8-bit
    const T *pC1 = ptr(0,0,0,1);
    cimg_mapY(*this,y) {
      unsigned char *ptrs = imgData[y];
      cimg_mapX(*this,x) {
        *(ptrs++) = (unsigned char)*(pC0++);
        *(ptrs++) = (unsigned char)*(pC1++);
      }
    }
      } break;
      case 21:    { // RGB 8-bit
    const T *pC1 = ptr(0,0,0,1);
    const T *pC2 = ptr(0,0,0,2);
    cimg_mapY(*this,y) {
      unsigned char *ptrs = imgData[y];
      cimg_mapX(*this,x) {
        *(ptrs++) = (unsigned char)*(pC0++);
        *(ptrs++) = (unsigned char)*(pC1++);
        *(ptrs++) = (unsigned char)*(pC2++);
      }
    }
      }    break;
      case 28: { // RGB x/ Alpha 8-bit
    const T *pC1 = ptr(0,0,0,1);
    const T *pC2 = ptr(0,0,0,2);
    const T *pC3 = ptr(0,0,0,3);
    cimg_mapY(*this,y){
      unsigned char *ptrs = imgData[y];
      cimg_mapX(*this,x){
        *(ptrs++) = (unsigned char)*(pC0++);
        *(ptrs++) = (unsigned char)*(pC1++);
        *(ptrs++) = (unsigned char)*(pC2++);
        *(ptrs++) = (unsigned char)*(pC3++);
      }
    }
      }    break;
      case 15: { // Gray 16-bit
    cimg_mapY(*this,y){
      unsigned short *ptrs = reinterpret_cast<unsigned short*>(imgData[y]);
      cimg_mapX(*this,x) *(ptrs++) = (unsigned short)*(pC0++);
    }
      }    break;
      case 30: { // Gray w/ Alpha 16-bit
    const T *pC1 = ptr(0,0,0,1);
    cimg_mapY(*this,y){
      unsigned short *ptrs = reinterpret_cast<unsigned short*>(imgData[y]);
      cimg_mapX(*this,x) {
        *(ptrs++) = (unsigned short)*(pC0++);
        *(ptrs++) = (unsigned short)*(pC1++);
      }
    }
      }    break;
      case 45: { // RGB 16-bit
    const T *pC1 = ptr(0,0,0,1);
    const T *pC2 = ptr(0,0,0,2);
    cimg_mapY(*this,y) {
      unsigned short *ptrs = reinterpret_cast<unsigned short*>(imgData[y]);
      cimg_mapX(*this,x) {
        *(ptrs++) = (unsigned short)*(pC0++);
        *(ptrs++) = (unsigned short)*(pC1++);
        *(ptrs++) = (unsigned short)*(pC2++);
      }
    }
      }    break;
      case 60: { // RGB x/ Alpha 16-bit
    const T *pC1 = ptr(0,0,0,1);
    const T *pC2 = ptr(0,0,0,2);
    const T *pC3 = ptr(0,0,0,3);
    cimg_mapY(*this,y) {
      unsigned short *ptrs = reinterpret_cast<unsigned short*>(imgData[y]);
      cimg_mapX(*this,x) {
        *(ptrs++) = (unsigned short)*(pC0++);
        *(ptrs++) = (unsigned short)*(pC1++);
        *(ptrs++) = (unsigned short)*(pC2++);
        *(ptrs++) = (unsigned short)*(pC3++);
      }
    }
      }    break;
      default:
    throw CImgIOException("CImg<%s>::save_png() : unspecified error reading PNG file '%s'",pixel_type(),filename);
    break;
      }
      png_write_image(png_ptr, imgData);      
      png_write_end(png_ptr, info_ptr);      
      png_destroy_write_struct(&png_ptr, &info_ptr);
      
      // Deallocate Image Write Memory
      for (unsigned int n=0; n<height; n++) delete[] imgData[n];
      delete[] imgData;      
      cimg::fclose(file);
      return *this;
#endif
    }

    //! Save a file in JPEG format.
    const CImg<T>& save_jpeg(const char *filename,const unsigned int quality=100) const {
      cimg_test(*this,"CImg<T>::save_jpeg");
#ifndef cimg_use_jpeg
      return save_convert(filename);
#else
      
      unsigned char *buf = new unsigned char[width*height*3], *buf2 = buf;
      const T *ptr_r = ptr(0,0,0,0),
    *ptr_g = ptr(0,0,0,dim>1?1:0),
    *ptr_b = ptr(0,0,0,dim>2?2:0);
      cimg_mapXY(*this,x,y) {
    *(buf2++) = (unsigned char)*(ptr_r++);
    *(buf2++) = (unsigned char)*(ptr_g++);
    *(buf2++) = (unsigned char)*(ptr_b++);
      }
      
      struct jpeg_compress_struct cinfo;
      struct jpeg_error_mgr jerr;
      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&cinfo);
      std::FILE *file = cimg::fopen(filename,"wb");
      jpeg_stdio_dest(&cinfo,file);
      cinfo.image_width = width;
      cinfo.image_height = height;
      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB;
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo,quality<100?quality:100,TRUE);
      jpeg_start_compress(&cinfo,TRUE);
      
      const unsigned int row_stride = width*3;
      JSAMPROW row_pointer[1];
      while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &buf[cinfo.next_scanline*row_stride];
    jpeg_write_scanlines(&cinfo,row_pointer,1);
      }
      jpeg_finish_compress(&cinfo);
      delete[] buf;
      cimg::fclose(file);
      jpeg_destroy_compress(&cinfo);
      return *this;
#endif
    }

    //! Save the image as a RGBA file
    const CImg& save_rgba(const char *filename) const {
      cimg_test(*this,"CImg<T>::save_rgba");
      std::FILE *file = cimg::fopen(filename,"wb");
      const unsigned int wh = width*height;
      unsigned char *buffer = new unsigned char[4*wh], *nbuffer=buffer;
      const T 
    *ptr1 = ptr(0,0,0,0),
    *ptr2 = dim>1?ptr(0,0,0,1):ptr1,
    *ptr3 = dim>2?ptr(0,0,0,2):ptr1,
    *ptr4 = dim>3?ptr(0,0,0,3):NULL;
      for (unsigned int k=0; k<wh; k++) {
    *(nbuffer++) = (unsigned char)(*(ptr1++));
    *(nbuffer++) = (unsigned char)(*(ptr2++));
    *(nbuffer++) = (unsigned char)(*(ptr3++));
    *(nbuffer++) = (unsigned char)(ptr4?(*(ptr4++)):255);
      }
      cimg::fwrite(buffer,sizeof(unsigned char),4*wh,file);
      cimg::fclose(file);
      delete[] buffer;
      return *this;
    }
    

    //! Save the image as a RGB file
    const CImg& save_rgb(const char *filename) const {
      cimg_test(*this,"CImg<T>::save_rgb");
      std::FILE *file = cimg::fopen(filename,"wb");
      const unsigned int wh = width*height;
      unsigned char *buffer = new unsigned char[3*wh], *nbuffer=buffer;
      const T 
    *ptr1 = ptr(0,0,0,0),
    *ptr2 = dim>1?ptr(0,0,0,1):ptr1,
    *ptr3 = dim>2?ptr(0,0,0,2):ptr1;
      for (unsigned int k=0; k<wh; k++) {
    *(nbuffer++) = (unsigned char)(*(ptr1++));
    *(nbuffer++) = (unsigned char)(*(ptr2++));
    *(nbuffer++) = (unsigned char)(*(ptr3++));
      }
      cimg::fwrite(buffer,sizeof(unsigned char),3*wh,file);
      cimg::fclose(file);
      delete[] buffer;
      return *this;
    }
    
    //! Get a 40x38 color logo of a 'danger' item
    static CImg get_logo40x38() { 
      static bool first_time = true;
      static CImg<T> res(40,38,1,3);
      if (first_time) {
    const unsigned char *ptrs = cimg::logo40x38;
    T *ptr1 = res.ptr(0,0,0,0), *ptr2 = res.ptr(0,0,0,1), *ptr3 = res.ptr(0,0,0,2);
    for (unsigned int off = 0; off<res.width*res.height;) {
      const unsigned char n = *(ptrs++), r = *(ptrs++), g = *(ptrs++), b = *(ptrs++);
      for (unsigned int l=0; l<n; off++,l++) { *(ptr1++) = (T)r; *(ptr2++) = (T)g; *(ptr3++) = (T)b; }
    }
    first_time = false;
      }      
      return res;
    }
       
    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name Other functions
    //@{
    //------------------------------------------
    //------------------------------------------
    CImg& swap(CImg& img) {
      cimg::swap(width,img.width);
      cimg::swap(height,img.height);
      cimg::swap(depth,img.depth);
      cimg::swap(dim,img.dim);
      cimg::swap(data,img.data);
      return img;
    }

#ifdef cimg_plugin
#include cimg_plugin
#endif
    
    //@}
  };


  /*-------------------------------------------------------
    



  Definition of the CImgl<> structure




  ------------------------------------------------------*/

  //! This class represents list of images CImg<T>.
  template<typename T> struct CImgl {       
    //! This variable represents the number of images in the image list.
    /**
       \note if \c size==0, the image list is empty.
    **/
    unsigned int size;
    
    //! This variable represents a pointer to the first \c CImg<T> image of the list.
    /**
       \note the images are stored continuously in memory.
       \note If the list is empty, \c data=NULL.
    **/
    CImg<T> *data;                      //!< Pointer to the first image of the image list.
    
    //------------------------------------------
    //------------------------------------------
    //
    //! \name Constructors - Destructor - Copy
    //@{
    //------------------------------------------
    //------------------------------------------
    
    //! Return a string describing the type of the image pixels in the list (template parameter \p T).
    static const char* pixel_type() { T val; return cimg::get_type(val); }
    
    //! Create a list of \p n new images, each having size (\p width,\p height,\p depth,\p dim).
    CImgl(const unsigned int n=0,const unsigned int width=0,const unsigned int height=1,
      const unsigned int depth=1, const unsigned int dim=1):size(n) {
      if (n) {
    data = new CImg<T>[(n/cimg::lblock+1)*cimg::lblock];
    cimgl_map(*this,l) data[l]=CImg<T>(width,height,depth,dim);
      } else data = NULL;
    }

    //! Create a list of \p n new images, each having size (\p width,\p height,\p depth,\p dim).
    CImgl(const unsigned int n,const unsigned int width,const unsigned int height,
      const unsigned int depth, const unsigned int dim,const T& val):size(n) {
      if (n) {
    data = new CImg<T>[(n/cimg::lblock+1)*cimg::lblock];
    cimgl_map(*this,l) data[l]=CImg<T>(width,height,depth,dim,val);
      } else data = NULL;
    }
    
    // ! Create a list of \p n copy of the input image.
    template<typename t> CImgl(const unsigned int n, const CImg<t>& img):size(n) {
      if (n) {
    data = new CImg<T>[(n/cimg::lblock+1)*cimg::lblock];
    cimgl_map(*this,l) data[l]=img;
      } else data = NULL;
    }
    
    //! Copy constructor.
    template<typename t> CImgl(const CImgl<t>& list):size(list.size) {
      if (size) {
    data = new CImg<T>[(size/cimg::lblock+1)*cimg::lblock];
    cimgl_map(*this,l) data[l] = list[l];
      } else data = NULL;
    }
    CImgl(const CImgl<T>& list):size(list.size) {
      if (size>0) {
    data = new CImg<T>[(size/cimg::lblock+1)*cimg::lblock];
    cimgl_map(*this,l) data[l] = list[l];
      } else data = NULL;
    }

    //! Create a list by loading a file.
    CImgl(const char* filename):size(0),data(NULL) { load(filename).swap(*this); }
    
    //! Create a list from a single image \p img.
    CImgl(const CImg<T>& img):size(0),data(NULL) { CImgl<T>(1,img).swap(*this); }

    //! Create a list from two images \p img1 and \p img2 (images are copied).
    CImgl(const CImg<T>& img1,const CImg<T>& img2):size(2) {
      data = new CImg<T>[cimg::lblock];
      data[0] = img1;
      data[1] = img2;
    }

    //! Create a list from three images \p img1,\p img2 and \p img3 (images are copied).
    CImgl(const CImg<T>& img1,const CImg<T>& img2,const CImg<T>& img3):size(3) {
      data = new CImg<T>[cimg::lblock];
      data[0] = img1;
      data[1] = img2;
      data[2] = img3;
    }

    //! Create a list from four images \p img1,\p img2,\p img3 and \p img4 (images are copied).
    CImgl(const CImg<T>& img1,const CImg<T>& img2,const CImg<T>& img3,const CImg<T>& img4):size(4) {
      data = new CImg<T>[cimg::lblock];
      data[0] = img1;
      data[1] = img2;
      data[2] = img3;
      data[3] = img4;
    }
    
    //! Copy a list into another one.
    template<typename t> CImgl& operator=(const CImgl<t>& list) { 
      if (list.size>size) return CImgl<T>(list).swap(*this); 
      size = list.size;
      cimgl_map(*this,l) data[l] = list[l];
      return *this;
    }

    CImgl& operator=(const CImgl<T>& list) { 
      if (&list==this) return *this; 
      if (list.size>size) return CImgl<T>(list).swap(*this);
      size = list.size;
      cimgl_map(*this,l) data[l] = list[l];
      return *this;
    }
    
    //! Destructor
    ~CImgl() { if (data) delete[] data; }
    
    //! Empty list
    CImgl& empty() { return CImgl<T>().swap(*this); }
    
    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name Arithmetics operators
    //@{
    //------------------------------------------
    //------------------------------------------
    
    //! Add each image of the current list with the corresponding image in the list \p list.
    template<typename t> CImgl& operator+=(const CImgl<t>& list) {
      const unsigned int sizemax = min(size,list.size);
      for (unsigned int l=0; l<sizemax; l++) (*this)[l]+=list[l];
      return *this;
    }
    
    //! Subtract each image of the current list with the corresponding image in the list \p list.
    template<typename t> CImgl& operator-=(const CImgl<t>& list) {
      const unsigned int sizemax = min(size,list.size);
      for (unsigned int l=0; l<sizemax; l++) (*this)[l]-=list[l];
      return *this;
    }
    
    //! Add each image of the current list with a value \p val.
    CImgl& operator+=(const T& val) { cimgl_map(*this,l) (*this)[l]+=val; return *this; }
    
    //! Substract each image of the current list with a value \p val.
    CImgl& operator-=(const T& val) { cimgl_map(*this,l) (*this)[l]-=val; return *this; }
    
    //! Multiply each image of the current list by a value \p val.
    CImgl& operator*=(const double val) { cimgl_map(*this,l) (*this)[l]*=val; return *this; }
    
    //! Divide each image of the current list by a value \p val.
    CImgl& operator/=(const double val) { cimgl_map(*this,l) (*this)[l]/=val; return *this; }
    
    //! Return a new image list corresponding to the addition of each image of the current list with a value \p val.
    CImgl operator+(const T& val) const { return CImgl<T>(*this)+=val;  }
    
    //! Return a new image list corresponding to the multiplication of each image of the current list by a value \p val.
    CImgl operator*(const double val) const { return CImgl<T>(*this)*=val;  }
    
    //! Return a new image list corresponding to the substraction of each image of the current list with a value \p val.
    CImgl operator-(const T& val) const { return CImgl<T>(*this)-=val;  }
    
    //! Return a new image list corresponding to the division of each image of the current list by a value \p val.
    CImgl operator/(const double val) const { return CImgl<T>(*this)/=val;  }
    
    //! Return a new image list corresponding to the addition of each image of the current list with the corresponding image in the list \p list.
    CImgl operator+(const CImgl& list) const { return CImgl<T>(*this)+=list; }

    //! Return a new image list corresponding to the substraction of each image of the current list with the corresponding image in the list \p list.
    CImgl operator-(const CImgl& list) const { return CImgl<T>(*this)-=list; }
    
    //! Return a new image list corresponding to the addition of each image of the current list with a value \p val;
    friend CImgl operator+(const T& val, const CImgl& list) { return CImgl<T>(list)+=val; }
    
    //! Return a new image list corresponding to the scalar multiplication of each image of the current list by a value \p val.
    friend CImgl operator*(const double val, const CImgl& list) { return CImgl<T>(list)*=val; }
  
    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name List operations
    //@{
    //------------------------------------------
    //------------------------------------------
    
    //! Return a reference to the i-th element of the image list.
    CImg<T>& operator[](const unsigned int pos) const {
#if cimg_debug>1
      if (pos>=size) {
    cimg::warn(true,"CImgl<%s>::operator[] : bad list position %u, in a list of %u images",pixel_type(),pos,size);
    return *data;
      }
#endif
      return data[pos];
    }
    
    //! Equivalent to CImgl<T>::operator[]
    CImg<T>& operator()(const unsigned int pos) const { return (*this)[pos]; }
    
    //! Insert a copy of the image \p img into the current image list, at position \p pos.
    CImgl& insert(const CImg<T>& img,const unsigned int pos) {
      if (pos>size) 
    throw CImgArgumentException("CImgl<%s>::insert() : Can't insert at position %u into a list with %u elements",
                    pixel_type(),pos,size);
      CImg<T> *new_data = (!((++size)%cimg::lblock) || !data)?new CImg<T>[(size/cimg::lblock+1)*cimg::lblock]:NULL;
      if (!data) { data=new_data; *data=img; }
      else {
    if (new_data) {
      std::memcpy(new_data,data,sizeof(CImg<T>)*pos);
      if (pos!=size-1) std::memcpy(new_data+pos+1,data+pos,sizeof(CImg<T>)*(size-1-pos));
      std::memset(data,0,sizeof(CImg<T>)*(size-1));
      delete[] data;
      data = new_data;
    }
    else if (pos!=size-1) memmove(data+pos+1,data+pos,sizeof(CImg<T>)*(size-1-pos));
    data[pos].data = NULL;
    data[pos] = img;
      }
      return *this;
    }
    
    //! Append a copy of the image \p img at the current image list.
    CImgl& insert(const CImg<T>& img) { return insert(img,size); }
    
    //! Insert a copy of the image list \p list into the current image list, starting from position \p pos.
    CImgl& insert(const CImgl<T>& list,const unsigned int pos) { 
      if (this!=&list) cimgl_map(list,l) insert(list[l],pos+l);
      else insert(CImgl<T>(list),pos);
      return *this; 
    }
    
    //! Append a copy of the image list \p list at the current image list.
    CImgl& insert(const CImgl<T>& list) { return insert(list,size); }
    
    //! Remove the image at position \p pos from the image list.
    CImgl& remove(const unsigned int pos) {
      if (pos>=size) { 
    cimg::warn(true,"CImgl<%s>::remove() : Can't remove an image from a list (%p,%u), at position %u",
           pixel_type(),data,size,pos);
    return *this;
      }
      CImg<T> tmp; tmp.swap(data[pos]); // the image to remove will be freed
      size--;
      if (pos!=size) { 
    memmove(data+pos,data+pos+1,sizeof(CImg<T>)*(size-pos));
    CImg<T> &tmp = data[size];
    tmp.width = tmp.height = tmp.depth = tmp.dim = 0;
    tmp.data = NULL;
      }
      return *this;
    }

    //! Remove the last image from the image list.
    CImgl& remove() { 
      if (size) return remove(size-1); 
      cimg::warn(true,"CImgl<%s>::remove() : List is empty",pixel_type());
      return *this;
    }

    //! Reverse list order
    CImgl& reverse() {
      for (unsigned int l=0; l<size/2; l++) (*this)[l].swap((*this)[size-1-l]);
      return *this;
    }
    
    //! Get reversed list
    CImgl& get_reverse() { return CImgl<T>(*this).reverse(); }

    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name Fourier transforms
    //@{
    //------------------------------------------
    //------------------------------------------
    
    //! Compute the Fast Fourier Transform (along the specified axis).
    CImgl& FFT(const char axe, const bool inverse=false) {
      if (size<2) throw CImgInstanceException("CImg<%s>::FFT() : Instance have less than 2 images",pixel_type());
      CImg<T> &Ir = data[0], &Ii = data[1];
      cimg_test(Ir,"CImg<T>::FFT"); cimg_test(Ii,"CImg<T>::FFT");
      if (Ir.width!=Ii.width || Ir.height!=Ii.height || Ir.depth!=Ii.depth || Ir.dim!=Ii.dim)
    throw CImgInstanceException("CImg<%s>::FFT() : Real and Imaginary parts of the instance have different dimensions",
                    pixel_type());
      switch (cimg::uncase(axe)) {
      case 'x': { // Fourier along X
    const unsigned int N = Ir.width, N2 = (N>>1);
    if (((N-1)&N) && N!=1) throw CImgInstanceException("CImg<%s>::FFT() : Dimension of instance image along 'x' is %d != 2^N",
                               pixel_type(),N);
    for (unsigned int i=0,j=0; i<N2; i++) {
      if (j>i) cimg_mapYZV(Ir,y,z,v) { cimg::swap(Ir(i,y,z,v),Ir(j,y,z,v)); cimg::swap(Ii(i,y,z,v),Ii(j,y,z,v));
      if (j<N2) { 
        const unsigned int ri = N-1-i, rj = N-1-j;
        cimg::swap(Ir(ri,y,z,v),Ir(rj,y,z,v)); cimg::swap(Ii(ri,y,z,v),Ii(rj,y,z,v)); 
      }}
      for (unsigned int m=N, n=N2; (j+=n)>=m; j-=m, m=n, n>>=1);
    }
    for (unsigned int delta=2; delta<=N; delta<<=1) {
      const unsigned int delta2 = (delta>>1);
      for (unsigned int i=0; i<N; i+=delta) {
        float wr = 1, wi = 0;
        const float angle = (float)((inverse?+1:-1)*2*cimg::PI/delta),
            ca = (float)std::cos(angle),
            sa = (float)std::sin(angle);
        for (unsigned int k=0; k<delta2; k++) {
          const unsigned int j = i + k, nj = j + delta2;
          cimg_mapYZV(Ir,y,z,k) {
        T &ir = Ir(j,y,z,k), &ii = Ii(j,y,z,k), &nir = Ir(nj,y,z,k), &nii = Ii(nj,y,z,k);           
        const T tmpr = wr*nir - wi*nii, tmpi = wr*nii + wi*nir;
        nir = ir - tmpr; nii = ii - tmpi;
        ir += tmpr; ii += tmpi;
          }
          const float nwr = wr*ca-wi*sa;
          wi = wi*ca + wr*sa;
          wr = nwr;          
        }
      }      
    }
    if (inverse) (*this)/=N;
      } break;

      case 'y': { // Fourier along Y
    const unsigned int N = Ir.height, N2 = (N>>1);
    if (((N-1)&N) && N!=1) throw CImgInstanceException("CImg<%s>::FFT() : Dimension of instance image(s) along 'y' is %d != 2^N",
                               pixel_type(),N);
    for (unsigned int i=0,j=0; i<N2; i++) {
      if (j>i) cimg_mapXZV(Ir,x,z,v) { cimg::swap(Ir(x,i,z,v),Ir(x,j,z,v)); cimg::swap(Ii(x,i,z,v),Ii(x,j,z,v));
      if (j<N2) { 
        const unsigned int ri = N-1-i, rj = N-1-j;
        cimg::swap(Ir(x,ri,z,v),Ir(x,rj,z,v)); cimg::swap(Ii(x,ri,z,v),Ii(x,rj,z,v)); 
      }}
      for (unsigned int m=N, n=N2; (j+=n)>=m; j-=m, m=n, n>>=1);
    }
    for (unsigned int delta=2; delta<=N; delta<<=1) {
      const unsigned int delta2 = (delta>>1);
      for (unsigned int i=0; i<N; i+=delta) {
        float wr = 1, wi = 0;
        const float angle = (float)((inverse?+1:-1)*2*cimg::PI/delta),
            ca = (float)std::cos(angle), sa = (float)std::sin(angle);
        for (unsigned int k=0; k<delta2; k++) {
          const unsigned int j = i + k, nj = j + delta2;
          cimg_mapXZV(Ir,x,z,k) {
        T &ir = Ir(x,j,z,k), &ii = Ii(x,j,z,k), &nir = Ir(x,nj,z,k), &nii = Ii(x,nj,z,k);           
        const T tmpr = wr*nir - wi*nii, tmpi = wr*nii + wi*nir;
        nir = ir - tmpr; nii = ii - tmpi;
        ir += tmpr; ii += tmpi;
          }
          const float nwr = wr*ca-wi*sa;
          wi = wi*ca + wr*sa;
          wr = nwr;          
        }
      }      
    }
    if (inverse) (*this)/=N;
      } break;

      case 'z': { // Fourier along Z
    const unsigned int N = Ir.depth, N2 = (N>>1);
    if (((N-1)&N) && N!=1) throw CImgInstanceException("CImg<%s>::FFT() : Dimension of instance image(s) along 'z' is %d != 2^N",
                               pixel_type(),N);
    for (unsigned int i=0,j=0; i<N2; i++) {
      if (j>i) cimg_mapXYV(Ir,x,y,v) { cimg::swap(Ir(x,y,i,v),Ir(x,y,j,v)); cimg::swap(Ii(x,y,i,v),Ii(x,y,j,v));
      if (j<N2) { 
        const unsigned int ri = N-1-i, rj = N-1-j;
        cimg::swap(Ir(x,y,ri,v),Ir(x,y,rj,v)); cimg::swap(Ii(x,y,ri,v),Ii(x,y,rj,v)); 
      }}
      for (unsigned int m=N, n=N2; (j+=n)>=m; j-=m, m=n, n>>=1);
    }
    for (unsigned int delta=2; delta<=N; delta<<=1) {
      const unsigned int delta2 = (delta>>1);
      for (unsigned int i=0; i<N; i+=delta) {
        float wr = 1, wi = 0;
        const float angle = (float)((inverse?+1:-1)*2*cimg::PI/delta),
            ca = (float)std::cos(angle), sa = (float)std::sin(angle);
        for (unsigned int k=0; k<delta2; k++) {
          const unsigned int j = i + k, nj = j + delta2;
          cimg_mapXYV(Ir,x,y,k) {
        T &ir = Ir(x,y,j,k), &ii = Ii(x,y,j,k), &nir = Ir(x,y,nj,k), &nii = Ii(x,y,nj,k);           
        const T tmpr = wr*nir - wi*nii, tmpi = wr*nii + wi*nir;
        nir = ir - tmpr; nii = ii - tmpi;
        ir += tmpr; ii += tmpi;
          }
          const float nwr = wr*ca-wi*sa;
          wi = wi*ca + wr*sa;
          wr = nwr;          
        }
      }      
    }
    if (inverse) (*this)/=N;
      } break;

      default: throw CImgArgumentException("CImg<%s>::FFT() : unknown axe '%c', must be 'x','y' or 'z'");
      }
      return *this; 
    }

    //! Compute the Fast Fourier Transform of a complex image.
    CImgl& FFT(const bool inverse=false) {
      if (size<2) throw CImgInstanceException("CImg<%s>::FFT() : Instance have less than 2 images",pixel_type());
      CImg<T> &Ir = data[0], &Ii = data[1];
      cimg_test(Ir,"CImg<T>::FFT"); cimg_test(Ii,"CImg<T>::FFT");
      if (Ir.width!=Ii.width || Ir.height!=Ii.height || Ir.depth!=Ii.depth || Ir.dim!=Ii.dim)
    throw CImgInstanceException("CImg<%s>::FFT() : Real and Imaginary parts of the instance have different dimensions",
                    pixel_type());
      if (Ir.depth>1)  FFT('z',inverse);
      if (Ir.height>1) FFT('y',inverse);
      if (Ir.width>1)  FFT('x',inverse);
      return *this;
    }
    
    //! Return the Fast Fourier Transform of a complex image
    CImgl get_FFT(const bool inverse=false) const { return CImgl<T>(*this).FFT(inverse); }

    //! Return the Fast Fourier Transform of a complex image (along a specified axis).
    CImgl get_FFT(const char axe,const bool inverse=false) const { return CImgl<T>(*this).FFT(axe,inverse); }
        
    //@}
    //------------------------------------------
    //------------------------------------------
    //
    //! \name IO and display functions
    //@{
    //------------------------------------------
    //------------------------------------------
    
    //! Print informations about the list on the standart error stream.
    const CImgl& print(const char* title=NULL,const int print_flag=1) const { 
      char tmp[1024];
      std::fprintf(stderr,"%-8s(this=%p) : { size=%u, data=%p }\n",title?title:"CImgl",(void*)this,size,(void*)data);
      if (print_flag>0)    cimgl_map(*this,l) {
    std::sprintf(tmp,"%s[%d]",title?title:"CImgl",l);
    data[l].print(tmp,print_flag);
      }
      return *this;
    }
    //! Load an image list from a file (.raw format).
#define cimg_load_raw_case(Ts,Tss)                          \
  if (!loaded && !cimg::strcasecmp(Ts,tmp2)) for (unsigned int l=0; l<n; l++) {  \
      const bool endian = cimg::endian();                               \
      j=0; while((i=std::fgetc(file))!='\n') tmp[j++]=(char)i; tmp[j]='\0';  \
      std::sscanf(tmp,"%u %u %u %u",&w,&h,&z,&k);                       \
      if (w*h*z*k>0) {                                                  \
        Tss *buf = new Tss[w*h*z*k]; cimg::fread(buf,sizeof(Tss),w*h*z*k,file);\
        if (endian) cimg::endian_swap(buf,w*h*z*k);                   \
        CImg<T> idest(w,h,z,k); cimg_mapoff(idest,off)                  \
                          idest[off] = (T)(buf[off]); idest.swap(res[l]); \
        delete[] buf;                                                   \
       }                                                                \
      loaded = true;                                                    \
    }

    static CImgl load_raw(const char *filename) {
      typedef unsigned char uchar;
      typedef unsigned short ushort;
      typedef unsigned int uint;  
      typedef unsigned long ulong; 
      std::FILE *file = cimg::fopen(filename,"rb");
      char tmp[256],tmp2[256];
      int i;
      bool loaded = false;
      unsigned int n,j,w,h,z,k,err;
      j=0; while((i=std::fgetc(file))!='\n' && i!=EOF) tmp[j++]=i; tmp[j]='\0';
      err=std::sscanf(tmp,"%u#%255[A-Za-z ]",&n,tmp2);
      if (err!=2) throw CImgIOException("CImgl<%s>::load_raw() : file '%s', Unknow .raw header",pixel_type(),filename);
      CImgl<T> res(n);
      cimg_load_raw_case("unsigned char",uchar);
      cimg_load_raw_case("uchar",uchar);
      cimg_load_raw_case("char",char);
      cimg_load_raw_case("unsigned short",ushort);
      cimg_load_raw_case("ushort",ushort);
      cimg_load_raw_case("short",short);
      cimg_load_raw_case("unsigned int",uint);
      cimg_load_raw_case("uint",uint);
      cimg_load_raw_case("int",int);
      cimg_load_raw_case("unsigned long",ulong);
      cimg_load_raw_case("ulong",ulong);
      cimg_load_raw_case("long",long);
      cimg_load_raw_case("float",float);
      cimg_load_raw_case("double",double);
      if (!loaded) throw CImgIOException("CImgl<%s>::load_raw() : file '%s', can't read images of %s",pixel_type(),filename,tmp2);
      cimg::fclose(file);
      return res;
    }

    //! Load an image list from a file. The file should be a '.raw' format, else only one image will be loaded into the list.
    static CImgl load(const char *filename) {
      CImgl res;
      const char *ext = cimg::filename_split(filename);
      if (!cimg::strcasecmp(ext,"raw") || !ext[0]) return load_raw(filename); else return CImg<T>(filename);
    }


    //! Save an image list into a file.
    /**
       Depending on the extension of the given filename, a file format is chosen for the output file.       
    **/    
    const CImgl& save(const char *filename) const {
      cimgl_test(*this,"CImgl<T>::save");
      const char *ext = cimg::filename_split(filename);
      if (!cimg::strcasecmp(ext,"raw") || !ext[0]) return save_raw(filename);
      else {
    if (size==1) data[0].save(filename,-1);
    else cimgl_map(*this,l) data[l].save(filename,l);
      }
      return *this;
    }

    //! Save an image list into a RAW file.
    /**
       A RAW file is a simple uncompressed binary file that may be used to save list of CImg<T> images.
       \param filename : name of the output file.
       \return A reference to the current CImgl instance is returned.
    **/
    const CImgl& save_raw(const char *filename) const {
      cimgl_test(*this,"CImgl<T>::save_raw");
      std::FILE *file = cimg::fopen(filename,"wb");
      std::fprintf(file,"%u#%s\n",size,pixel_type());
      cimgl_map(*this,l) {
    const CImg<T>& img = data[l];
    std::fprintf(file,"%u %u %u %u\n",img.width,img.height,img.depth,img.dim);
    if (img.data) {
      if (cimg::endian()) {
        CImg<T> tmp(img);
        cimg::endian_swap(tmp.data,tmp.size());
        cimg::fwrite(tmp.data,sizeof(T),img.width*img.height*img.depth*img.dim,file);
      } else cimg::fwrite(img.data,sizeof(T),img.width*img.height*img.depth*img.dim,file);
    }
      }
      cimg::fclose(file);
      return *this;
    }  

    //! Return a single image which is the concatenation of all images of the current CImgl instance.
    /**
       \param axe : specify the axe for image concatenation. Can be 'x','y','z' or 'v'.
       \param align : specify the alignment for image concatenation. Can be 'p' (top), 'c' (center) or 'n' (bottom).
       \return A CImg<T> image corresponding to the concatenation is returned.
    **/
    CImg<T> get_append(const char axe='x',const char align='c') const {
      cimgl_test(*this,"CImgl<T>::get_append");
      unsigned int dx=0,dy=0,dz=0,dv=0,pos=0;
      CImg<T> res;
      switch(cimg::uncase(axe)) {
      case 'x': {
    cimgl_map(*this,l) {
      const CImg<T>& img = (*this)[l];
      dx += img.width;
      dy = cimg::max(dy,img.height);
      dz = cimg::max(dz,img.depth);
      dv = cimg::max(dv,img.dim);
    }
    res = CImg<T>(dx,dy,dz,dv,0);
    switch (cimg::uncase(align)) {
    case 'p' : { cimgl_map(*this,ll) { res.draw_image((*this)[ll],pos,0,0,0); pos+=(*this)[ll].width; }} break;
    case 'n' : { cimgl_map(*this,ll) { 
          res.draw_image((*this)[ll],pos,dy-(*this)[ll].height,dz-(*this)[ll].depth,dv-(*this)[ll].dim); pos+=(*this)[ll].width;
        }} break;
    default  : { cimgl_map(*this,ll) {
          res.draw_image((*this)[ll],pos,(dy-(*this)[ll].height)/2,(dz-(*this)[ll].depth)/2,(dv-(*this)[ll].dim)/2);
          pos+=(*this)[ll].width; 
        }} break;
    }
      }    break;
      case 'y': {
    cimgl_map(*this,l) {
      const CImg<T>& img = (*this)[l];
      dx = cimg::max(dx,img.width);
      dy += img.height;
      dz = cimg::max(dz,img.depth);
      dv = cimg::max(dv,img.dim);
    }
    res = CImg<T>(dx,dy,dz,dv,0);
    switch (cimg::uncase(align)) {
    case 'p': { cimgl_map(*this,ll) { res.draw_image((*this)[ll],0,pos,0,0); pos+=(*this)[ll].height; }} break;
    case 'n': { cimgl_map(*this,ll) { 
          res.draw_image((*this)[ll],dx-(*this)[ll].width,pos,dz-(*this)[ll].depth,dv-(*this)[ll].dim); pos+=(*this)[ll].height;
        }} break;
    default : { cimgl_map(*this,ll) { 
          res.draw_image((*this)[ll],(dx-(*this)[ll].width)/2,pos,(dz-(*this)[ll].depth)/2,(dv-(*this)[ll].dim)/2);
          pos+=(*this)[ll].height; 
        }} break;
    }
      }    break;
      case 'z': {
    cimgl_map(*this,l) {
      const CImg<T>& img = (*this)[l];
      dx = cimg::max(dx,img.width);
      dy = cimg::max(dy,img.height);
      dz += img.depth;
      dv = cimg::max(dv,img.dim);
    }
    res = CImg<T>(dx,dy,dz,dv,0);
    switch (cimg::uncase(align)) {
    case 'p': { cimgl_map(*this,ll) { res.draw_image((*this)[ll],0,0,pos,0); pos+=(*this)[ll].depth; }} break;
    case 'n': { cimgl_map(*this,ll) { 
          res.draw_image((*this)[ll],dx-(*this)[ll].width,dy-(*this)[ll].height,pos,dv-(*this)[ll].dim); pos+=(*this)[ll].depth;
        }} break;
    case 'c': { cimgl_map(*this,ll) { 
          res.draw_image((*this)[ll],(dx-(*this)[ll].width)/2,(dy-(*this)[ll].height)/2,pos,(dv-(*this)[ll].dim)/2);
          pos+=(*this)[ll].depth; 
        }} break;
    }
      }    break;
      case 'v': {
    cimgl_map(*this,l) {
      const CImg<T>& img = (*this)[l];
      dx = cimg::max(dx,img.width);
      dy = cimg::max(dy,img.height);
      dz = cimg::max(dz,img.depth);
      dv += img.dim;
    }
    res = CImg<T>(dx,dy,dz,dv,0);
    switch (cimg::uncase(align)) {
    case 'p': { cimgl_map(*this,ll) { res.draw_image((*this)[ll],0,0,0,pos); pos+=(*this)[ll].dim; }} break;
    case 'n': { cimgl_map(*this,ll) { 
          res.draw_image((*this)[ll],dx-(*this)[ll].width,dy-(*this)[ll].height,dz-(*this)[ll].depth,pos); pos+=(*this)[ll].dim;
        }} break;
    case 'c': { cimgl_map(*this,ll) { 
          res.draw_image((*this)[ll],(dx-(*this)[ll].width)/2,(dy-(*this)[ll].height)/2,(dz-(*this)[ll].depth)/2,pos);
          pos+=(*this)[ll].dim; 
        }} break;
    }
      } break;
      default: throw CImgArgumentException("CImg<%s>::get_append() : unknow axe '%c', must be 'x','y','z' or 'v'",pixel_type(),axe);
      }
      return res;
    }

    // Create an auto-cropped font (along the X axis) from a input font \p font.
    CImgl<T> get_crop_font(const unsigned int padding=1) const {
      CImgl<T> res;
      cimgl_map(*this,l) {
    const CImg<T>& letter = (*this)[l];
        int xmin=letter.width, xmax = 0;
        cimg_mapXY(letter,x,y) if (letter(x,y)) { if (x<xmin) xmin=x; if (x>xmax) xmax=x; }
        if (xmin>xmax) res.insert(CImg<T>(4*padding,(*this)[' '].height,1,(*this)[' '].dim,0));
        else res.insert(letter.get_crop(xmin,0,xmax+padding,letter.height));
      }
      return res;
    }
    
    CImgl<T>& crop_font(const unsigned int padding=1) {
      return get_crop_font(padding).swap(*this);
    }

    //! Return a copy of the default 7x11 CImg font as a list of colors images and masks.
    /**
       \param fixed_size = compute fixed or variable character size
    **/

    static CImgl<T> get_font(const unsigned int *const font,const unsigned int w,const unsigned int h,
                 const int padding=1) {
      CImgl<T> res = CImgl<T>(256,w,h,1,3).insert(CImgl<T>(256,w,h,1,1));
      const unsigned int *ptr = font;
      unsigned long m = 0, val = 0;
      for (unsigned int y=0; y<h; y++)
    for (unsigned int x=0; x<256*w; x++) {
      m>>=1; if (!m) { m=0x80000000; val = *(ptr++); }
      CImg<T>& img = res[x/w], &mask = res[x/w+256];
      unsigned int xm = x%w;
      img(xm,y,0) = img(xm,y,1) = img(xm,y,2) = mask(xm,y,0) = (T)((val&m)?1:0);
    }
      if (padding>=0) return res.get_crop_font(padding);
      return res;
    }

    static CImgl<T> get_font10x13(const bool fixed_size = false) {
      static CImgl<T> nfixed, fixed;
      if (fixed_size) {
    if (!fixed.size) fixed = get_font(cimg::font10x13,10,13,-1);
    return fixed;
      } 
      if (!nfixed.size) nfixed = get_font(cimg::font10x13,10,13,1);
      return nfixed;
    }

    static CImgl<T> get_font7x11(const bool fixed_size = false) {
      static CImgl<T> nfixed, fixed;
      if (fixed_size) {
    if (!fixed.size) fixed = get_font(cimg::font7x11,7,11,-1);
    return fixed;
      } 
      if (!nfixed.size) nfixed = get_font(cimg::font7x11,7,11,1);
      return nfixed;
    }
    
    //! Display the current CImgl instance in an existing CImgDisplay window (by reference).
    /**
       This function displays the list images of the current CImgl instance into an existing CImgDisplay window.
       Images of the list are concatenated in a single temporarly image for visualization purposes.
       The function returns immediately.
       \param disp : reference to an existing CImgDisplay instance, where the current image list will be displayed.
       \param axe : specify the axe for image concatenation. Can be 'x','y','z' or 'v'.
       \param align : specify the alignment for image concatenation. Can be 'p' (top), 'c' (center) or 'n' (bottom).
       \return A reference to the current CImgl instance is returned.
    **/
    const CImgl& display(CImgDisplay& disp,const char axe='x',const char align='c') const { 
      get_append(axe,align).display(disp); return *this; 
    }

    //! Display the current CImgl instance in an existing CImgDisplay window (by pointer).
    /**
       This function displays the list images of the current CImgl instance into an existing CImgDisplay window.
       Images of the list are concatenated in a single temporarly image for visualization purposes.
       The function returns immediately.
       \param disp : pointer to an existing CImgDisplay instance, where the current image list will be displayed.
       \param axe : specify the axe for image concatenation. Can be 'x','y','z' or 'v'.
       \param align : specify the alignment for image concatenation. Can be 'p' (top), 'c' (center) or 'n' (bottom).
       \return A reference to the current CImgl instance is returned.
    **/
    const CImgl& display(CImgDisplay* disp,const char axe='x',const char align='c') const { 
      if (!disp) throw CImgArgumentException("CImgl<%s>::display() : given display pointer is (null)",pixel_type());
      else display(*disp,axe,align);
      return *this;
    }

    //! Display the current CImgl instance in a new display window.
    /**
       This function opens a new window with a specific title and displays the list images of the current CImgl instance into it.
       Images of the list are concatenated in a single temporarly image for visualization purposes.
       The function returns when a key is pressed or the display window is closed by the user.
       \param title : specify the title of the opening display window.
       \param axe : specify the axe for image concatenation. Can be 'x','y','z' or 'v'.
       \param align : specify the alignment for image concatenation. Can be 'p' (top), 'c' (center) or 'n' (bottom).
       \param min_size : specify the minimum size of the opening display window. Images having dimensions below this
       size will be upscaled.
       \param max_size : specify the maximum size of the opening display window. Images having dimensions above this
       size will be downscaled.
       \return A reference to the current CImgl instance is returned.
    **/
    const CImgl& display(const char* title,const char axe='x',const char align='c',
             const int min_size=128,const int max_size=1024) const {
      get_append(axe,align).display(title,min_size,max_size);
      return *this;
    }

    //! Display the current CImgl instance in a new display window.
    /**
       This function opens a new window and displays the list images of the current CImgl instance into it.
       Images of the list are concatenated in a single temporarly image for visualization purposes.
       The function returns when a key is pressed or the display window is closed by the user.
       \param axe : specify the axe for image concatenation. Can be 'x','y','z' or 'v'.
       \param align : specify the alignment for image concatenation. Can be 'p' (top), 'c' (center) or 'n' (bottom).
       \param min_size : specify the minimum size of the opening display window. Images having dimensions below this
       size will be upscaled.
       \param max_size : specify the maximum size of the opening display window. Images having dimensions above this
       size will be downscaled.
       \return A reference to the current CImgl instance is returned.
    **/
    const CImgl& display(const char axe='x',const char align='c',
             const int min_size=128,const int max_size=1024) const {
      return display("",axe,align,min_size,max_size); 
    }

    //! Same as \ref cimg::wait()
    /**
       \see cimg::wait().
    **/
    const CImgl& wait(const unsigned int milliseconds) const { cimg::wait(milliseconds); return *this;  }
    
    // Swap fields of two CImgl instances.
    CImgl& swap(CImgl& list) {
      cimg::swap(size,list.size);
      cimg::swap(data,list.data);
      return list;
    }

#ifdef cimgl_plugin
#include cimgl_plugin
#endif
   
    //@}
  };


  //! This class represents a region of interest of a CImg<T> image.
  /**
     When using the functions CImg<T>::ref_*(), an instance of a CImgROI<T> is returned, corresponding
     to a sub-image of the original image. Acting on the CImgROI<T> instance allows to modify only a
     part of the original image.
     CImgROI<T> instances should be handled with care.  
  **/
  template<typename T> struct CImgROI : public CImg<T> {
    CImgROI(const unsigned int dx,const unsigned int dy,const unsigned int dz,const unsigned int dv,T *const pdata) {
      CImg<T>::width = dx; CImg<T>::height = dy; CImg<T>::depth = dz; CImg<T>::dim = dv; CImg<T>::data = pdata;
    }
    CImgROI(const CImgROI& roi):CImg<T>() {
      CImg<T>::width = roi.width; CImg<T>::height = roi.height; CImg<T>::depth = roi.depth; CImg<T>::dim = roi.dim; 
      CImg<T>::data = roi.data;
    }
    ~CImgROI() { CImg<T>::width=CImg<T>::height=CImg<T>::depth=CImg<T>::dim=0; CImg<T>::data=NULL;}
    
    template<typename t> CImgROI<T>& operator=(const CImg<t>& img) { 
      if (img.width!=CImg<T>::width || img.height!=CImg<T>::height ||
      img.depth!=CImg<T>::depth || img.dim!=CImg<T>::dim)
    throw CImgArgumentException("CImgROI<%s>::operator=() : Affectation to CImgROI instances must supply "
                    "data with same dimensions",CImg<T>::pixel_type());
      const t* ptrs = img.data + CImg<T>::size();
      for (T *ptrd = CImg<T>::data+CImg<T>::size(); ptrd>CImg<T>::data; ) *(--ptrd) = (T)*(--ptrs);
      return *this;
    }

    CImgROI& operator=(const CImg<T>& img) {
      if (&img==this) return *this;
      if (img.width!=CImg<T>::width || img.height!=CImg<T>::height ||
      img.depth!=CImg<T>::depth || img.dim!=CImg<T>::dim)
    throw CImgArgumentException("CImgROI<%s>::operator=() : Affectation to CImgROI instances must supply "
                    "data with same dimensions",CImg<T>::pixel_type());
      std::memcpy(CImg<T>::data,img.data,sizeof(T)*CImg<T>::size());
      return *this;
    }
    
  };

namespace cimg {
  
  //! Display a dialog box, where a user can click standart buttons.
  /**
     Up to 6 buttons can be defined in the dialog window.
     This function returns when a user clicked one of the button or closed the dialog window.
     \param title = Title of the dialog window.
     \param msg = Main message displayed inside the dialog window.
     \param button1_txt = Label of the 1st button.
     \param button2_txt = Label of the 2nd button.
     \param button3_txt = Label of the 3rd button.
     \param button4_txt = Label of the 4th button.
     \param button5_txt = Label of the 5th button.
     \param button6_txt = Label of the 6th button.
     \param logo = Logo image displayed at the left of the main message. This parameter is optional.
     \return The button number (from 0 to 5), or -1 if the dialog window has been closed by the user.
     \note If a button text is set to NULL, then the corresponding button (and the followings) won't appear in
     the dialog box. At least one button is necessary.
  **/
  template<typename t>
  static int dialog(const char *title,const char *msg,
            const char *button1_txt,const char *button2_txt,
            const char *button3_txt,const char *button4_txt,
            const char *button5_txt,const char *button6_txt,
            const CImg<t>& logo) {
#if cimg_display_type!=0
    const unsigned char
      black[3]={0,0,0}, white[3]={255,255,255}, gray[3]={200,200,200}, gray2[3]={150,150,150};
      
      // Create buttons and canvas graphics
      CImgl<unsigned char> buttons, cbuttons, sbuttons;
      const CImgl<unsigned char> tahoma = CImgl<unsigned char>::get_font10x13(false);
      if (button1_txt) { buttons.insert(CImg<unsigned char>().draw_text(button1_txt,0,0,black,gray,tahoma));
      if (button2_txt) { buttons.insert(CImg<unsigned char>().draw_text(button2_txt,0,0,black,gray,tahoma));
      if (button3_txt) { buttons.insert(CImg<unsigned char>().draw_text(button3_txt,0,0,black,gray,tahoma));
      if (button4_txt) { buttons.insert(CImg<unsigned char>().draw_text(button4_txt,0,0,black,gray,tahoma));
      if (button5_txt) { buttons.insert(CImg<unsigned char>().draw_text(button5_txt,0,0,black,gray,tahoma));
      if (button6_txt) { buttons.insert(CImg<unsigned char>().draw_text(button6_txt,0,0,black,gray,tahoma));
      }}}}}}
      if (!buttons.size) throw CImgArgumentException("cimg::dialog() : No buttons have been defined. At least one is necessary");
      
      unsigned int bw=0, bh=0;
      cimgl_map(buttons,l) { bw = cimg::max(bw,buttons[l].width); bh = cimg::max(bh,buttons[l].height); }
      bw+=8; bh+=8;
      if (bw<64) bw=64;
      if (bw>128) bw=128;
      if (bh<24) bh=24;
      if (bh>48) bh=48;
      
      CImg<unsigned char> button = CImg<unsigned char>(bw,bh,1,3).
    draw_rectangle(0,0,bw-1,bh-1,gray).
    draw_line(0,0,bw-1,0,white).draw_line(0,bh-1,0,0,white).
    draw_line(bw-1,0,bw-1,bh-1,black).draw_line(bw-1,bh-1,0,bh-1,black).
    draw_line(1,bh-2,bw-2,bh-2,gray2).draw_line(bw-2,bh-2,bw-2,1,gray2);
      CImg<unsigned char> sbutton = CImg<unsigned char>(bw,bh,1,3).
    draw_rectangle(0,0,bw-1,bh-1,gray).
    draw_line(0,0,bw-1,0,black).draw_line(bw-1,0,bw-1,bh-1,black).
    draw_line(bw-1,bh-1,0,bh-1,black).draw_line(0,bh-1,0,0,black).
    draw_line(1,1,bw-2,1,white).draw_line(1,bh-2,1,1,white).
    draw_line(bw-2,1,bw-2,bh-2,black).draw_line(bw-2,bh-2,1,bh-2,black).
    draw_line(2,bh-3,bw-3,bh-3,gray2).draw_line(bw-3,bh-3,bw-3,2,gray2).
    draw_line(4,4,bw-5,4,black,0xAAAAAAAA).draw_line(bw-5,4,bw-5,bh-5,black,0xAAAAAAAA).
    draw_line(bw-5,bh-5,4,bh-5,black,0xAAAAAAAA).draw_line(4,bh-5,4,4,black,0xAAAAAAAA);
      CImg<unsigned char> cbutton = CImg<unsigned char>(bw,bh,1,3).
    draw_rectangle(0,0,bw-1,bh-1,black).draw_rectangle(1,1,bw-2,bh-2,gray2).draw_rectangle(2,2,bw-3,bh-3,gray).
    draw_line(4,4,bw-5,4,black,0xAAAAAAAA).draw_line(bw-5,4,bw-5,bh-5,black,0xAAAAAAAA).
    draw_line(bw-5,bh-5,4,bh-5,black,0xAAAAAAAA).draw_line(4,bh-5,4,4,black,0xAAAAAAAA);

    cimgl_map(buttons,ll) {
      cbuttons.insert(CImg<unsigned char>(cbutton).draw_image(buttons[ll],1+(bw-buttons[ll].dimx())/2,1+(bh-buttons[ll].dimy())/2));
      sbuttons.insert(CImg<unsigned char>(sbutton).draw_image(buttons[ll],(bw-buttons[ll].dimx())/2,(bh-buttons[ll].dimy())/2));
      buttons[ll] = CImg<unsigned char>(button).draw_image(buttons[ll],(bw-buttons[ll].dimx())/2,(bh-buttons[ll].dimy())/2);
    }
    
    CImg<unsigned char> canvas;
    if (msg) canvas = CImg<unsigned char>().draw_text(msg,0,0,black,gray,tahoma);
    const unsigned int 
      bwall = (buttons.size-1)*(12+bw) + bw,
      w = cimg::max(196U,36+logo.width+canvas.width, 24+bwall),
      h = cimg::max(96U,36+canvas.height+bh,36+logo.height+bh),
      lx = 12 + (canvas.data?0:((w-24-logo.width)/2)),
      ly = (h-12-bh-logo.height)/2,
      tx = lx+logo.width+12,
      ty = (h-12-bh-canvas.height)/2,
      bx = (w-bwall)/2,
      by = h-12-bh;
    
    if (canvas.data)
      canvas = CImg<unsigned char>(w,h,1,3).
        draw_rectangle(0,0,w-1,h-1,gray).
        draw_line(0,0,w-1,0,white).draw_line(0,h-1,0,0,white).
        draw_line(w-1,0,w-1,h-1,black).draw_line(w-1,h-1,0,h-1,black).
        draw_image(canvas,tx,ty);
    else 
      canvas = CImg<unsigned char>(w,h,1,3).
        draw_rectangle(0,0,w-1,h-1,gray).
        draw_line(0,0,w-1,0,white).draw_line(0,h-1,0,0,white).
        draw_line(w-1,0,w-1,h-1,black).draw_line(w-1,h-1,0,h-1,black);
    if (logo.data) canvas.draw_image(logo,lx,ly);
    
    unsigned int xbuttons[6];
    cimgl_map(buttons,lll) { xbuttons[lll] = bx+(bw+12)*lll; canvas.draw_image(buttons[lll],xbuttons[lll],by); }
    
    // Open window and enter events loop  
    CImgDisplay disp(canvas,title?title:"",0,3);
    bool stopflag = false, refresh = false;
    int oselected = -1, oclicked = -1, selected = -1, clicked = -1;
    while (!disp.closed && !stopflag) {
      if (refresh) {
        if (clicked>=0) CImg<unsigned char>(canvas).draw_image(cbuttons[clicked],xbuttons[clicked],by).display(disp);
        else {
          if (selected>=0) CImg<unsigned char>(canvas).draw_image(sbuttons[selected],xbuttons[selected],by).display(disp);
          else canvas.display(disp);
        }
        refresh = false;
      }
      disp.wait(40);
      if (disp.resized) disp.resize(disp);
      
      if (disp.button&1)  {
        oclicked = clicked;
        clicked = -1;
        cimgl_map(buttons,l)
          if (disp.mouse_y>=(int)by && disp.mouse_y<(int)(by+bh) &&
          disp.mouse_x>=(int)xbuttons[l] && disp.mouse_x<(int)(xbuttons[l]+bw)) {
        clicked = selected = l;
        refresh = true;
          }
        if (clicked!=oclicked) refresh = true;
      } else if (clicked>=0) stopflag = true;
      
      if (disp.key) {
        oselected = selected;
        switch (disp.key) {
        case cimg::keyESC: selected=-1; stopflag=true; break;
        case cimg::keyENTER: if (selected<0) selected=0; stopflag = true; break;
        case cimg::keyTAB:
        case cimg::keyARROWRIGHT:
        case cimg::keyARROWDOWN: selected = (selected+1)%buttons.size; break;
        case cimg::keyARROWLEFT:
        case cimg::keyARROWUP: selected = (selected+buttons.size-1)%buttons.size; break;
        }
        disp.key=0;
        if (selected!=oselected) refresh = true;
      }
    }
    if (disp.closed) selected = -1;
    return selected;
#else
    std::fprintf(stderr,"<%s>\n\n%s\n\n",title,msg);
    return -1;
#endif
  }
  
  static int dialog(const char *title,const char *msg,const char *button1_txt,const char *button2_txt,
            const char *button3_txt,const char *button4_txt,const char *button5_txt,const char *button6_txt) {
    return dialog(title,msg,button1_txt,button2_txt,button3_txt,button4_txt,button5_txt,button6_txt,
          CImg<unsigned char>::get_logo40x38());
  }
}

}

// Overcome VisualC++ 6.0 and DMC compilers namespace bug
#if ( defined(_MSC_VER) || defined(__DMC__) ) && defined(std)
#undef std
#endif

/*--------------------------------------------------------------------------------------



  Additional documentation for the generation of the reference page (using doxygen)



  -------------------------------------------------------------------------------------*/
/**
   \mainpage
   
   This is the reference documentation of <a href="http://cimg.sourceforge.net">the CImg Library</a>.
   This documentation has been automatically generated from the header file CImg.h, using the tool
   <a href="http://www.doxygen.org">doxygen</a>.
   It contains a detailed description of all classes and functions of <a href="http://cimg.sourceforge.net">the CImg Library</a>.
   If you have downloaded the CImg zip package, you have a local copy of these documentation pages in the directory
   \c CImg/documentation/reference/.

   If you don't know what the CImg Library is, you should look first at <a href="http://cimg.sourceforge.net">the project web page</a>.
   
   You can easily navigate through the documentation pages, using the menu above.

   To get started, you may first check the list of <a href="modules.html">available modules</a>.

*/

//---------------------------------------------------------------------------------------------------------------
/** \addtogroup cimg_structure Introduction to the CImg Library */
/*@{
  \page foo2

  The <b>CImg Library</b> is an image processing library, designed for C++ programmers.
  It provides useful classes and functions to load/save, display and process various types of images.
  
  \section s1 Library structure

  The CImg Library consists in a <b>single header file</b> \ref CImg.h providing a set of C++ template classes that
  can be used in your own sources, to load/save, process and display images or list of images.
  Very portable (Unix/X11,Windows, MacOS X, FreeBSD,..), efficient, simple to use, it's a pleasant toolkit
  for coding image processing stuffs in C++.

  The header file \ref CImg.h contains all the classes and functions that compose the library itself.
  This is one originality of the CImg Library. This particularly means that :
  - No pre-compilation of the library is needed, since the compilation of the CImg functions is done at the same time as
  the compilation of your own C++ code.
  - No complex dependencies have to be handled : Just include the CImg.h file, and you get a working C++ image processing toolkit.
  - The compilation is done on the fly : only CImg functionalities really used by your program are compiled and appear in the 
  compiled executable program. This leads to very compact code, without any unused stuffs.
  - Class members and functions are inlined, leading to better performance during the program execution.

  The CImg Library is structured as follows :
  
  - All library classes and functions are defined in the namespace \ref cimg_library. This namespace
  encapsulates the library functionalities and avoid any class name collision that could happen with
  other includes. Generally, one uses this namespace as a default namespace :
  \code
  #include "CImg.h"
  using namespace cimg_library;
  ...
  \endcode

  - The namespace \ref cimg_library::cimg defines a set of \e low-level functions and variables used by the library.
  Documented functions in this namespace can be safely used in your own program. But, \b never use the
  \ref cimg_library::cimg namespace as a default namespace, since it contains functions whose names are already
  defined in the standart C/C++ library.

  - The class \ref cimg_library::CImg<T> represents images up to 4-dimensions wide, containing pixels of type \c T
  (template parameter). This is actually the main class of the library.

  - The class \ref cimg_library::CImgl<T> represents lists of cimg_library::CImg<T> images. It can be used for instance
  to store different frames of an image sequence.

  - The class \ref cimg_library::CImgDisplay is able to display images or image lists into graphical display windows.
  As you may guess, the code of this class is highly system-dependent but this is transparent for the programmer,
  as environment variables are automatically set by the CImg library (see also \ref cimg_environment).

  - The class \ref cimg_library::CImgStats represents image statistics. Use it to compute the
  minimum, maximum, mean and variance of pixel values of images, as well as the corresponding min/max pixel location.

  - The class \ref cimg_library::CImgException (and its subclasses) are used by the library to throw exceptions
  when errors occur. Those exceptions can be catched with a bloc <tt>try { ..} catch (CImgException) { .. }</tt>.
  Subclasses define precisely the type of encountered errors.

  Knowing these five classes is \b enough to get benefit of the CImg Library functionalities.


  \section s2 CImg version of "Hello world".

  Below is a very simple code that creates a "Hello World" image. This shows you basically how a CImg program looks like.

  \code
  #include "CImg.h"
  using namespace cimg_library;

  int main() {
    CImg<unsigned char> img(640,400,1,3);        // Define a 640x400 color image with 8 bits per color component.
    img.fill(0);                                 // Set pixel values to 0 (color : black)
    unsigned char purple[3]={255,0,255};         // Define a purple color
    img.draw_text("Hello World",100,100,purple); // Draw a purple "Hello world" at coordinates (100,100).
    img.display("My first CImg code");           // Display the image in a display window.
    return 0;
  }
  \endcode

  Which can be also written in a more compact way as :

  \code
  #include "CImg.h"
  using namespace cimg_library;

  int main() {
    const unsigned char purple[3]={255,0,255};
    CImg<unsigned char>(640,400,1,3,0).draw_text("Hello World",100,100,purple).display("My first CImg code");
    return 0;
  }
  \endcode

  Generally, you can write very small code that performs complex image processing tasks. The CImg Library is very simple
  to use and provide a lot of interesting algorithms for image manipulation.
  
  \section s3 How to compile ?

  The CImg library is a very light and user-friendly library : only standart system libraries are used.
  It avoid to handle complex dependancies and problems with library compatibility.
  The only thing you need is a (quite modern) C++ compiler. Before each release, the CImg library
  is successfully compiled with the following compilers :
  
  - <b>Microsoft Visual C++ 6.0 and Visual Studio.NET</b> : Use project files and solution files provided in the 
  CImg Library package to see how it works.
  - <b>Intel ICL compiler</b> : Use the following command to compile a CImg-based program with ICL :
  \code
  icl /Ox hello_world.cpp user32.lib gdi32.lib
  \endcode
  - <b>Digital Mars Compiler</b> : Use the following command to compile a CImg-based program with DMC :
  \code
  dmc -Ae hello_world.cpp gdi32.lib
  \endcode
  - <b>g++ (MingW windows version)</b> : Use the following command to compile a CImg-based program with g++, on Windows :
  \code
  g++ -o hello_word.exe hello_word.cpp -O2 -lgdi32
  \endcode
  - <b>g++ (Linux version)</b> : Use the following command to compile a CImg-based program with g++, on Linux :
  \code
  g++ -o hello_word.exe hello_world.cpp -O2 -L/usr/X11R6/lib -lm -lpthread -lX11
  \endcode
  - <b>g++ (Solaris version)</b> : Use the following command to compile a CImg-based program with g++, on Solaris :
  \code
  g++ -o hello_word.exe hello_world.cpp -O2 -lm -lpthread -R/usr/X11R6/lib -lrt -lnsl -lsocket
  \endcode
  - <b>g++ (Mac OS X version)</b> : Use the following command to compile a CImg-based program with g++, on Mac OS X :
  \code
  g++ -o hello_word.exe hello_world.cpp -O2 -lm -lpthread -L/usr/X11R6/lib -lm -lpthread -lX11
  \endcode
  - <b>Dev-Cpp</b> : Use the project file provided in the CImg library package to see how it works.

  If you are using another compilers and encounter problems, please
  <a href="http://www.greyc.ensicaen.fr/~dtschump">write me</a> since maintaining compatibility is one
  of the priority of the CImg Library. Nevertheless, old compilers that does not respect the C++ norm will not
  support the CImg Library.

  \section s4 What's next ?

  If you are ready to get more, and to start writing more serious programs
  with CImg, you are invited to go to the \ref cimg_tutorial section.

**/
/*@}*/

//--------------------------------------------------------------------------------------------------------------------
/** \addtogroup cimg_environment Setting Environment Variables */
/*@{
  \page foo1
  
  The CImg library is a multiplatform library, working on a wide variety of systems.
  This implies the existence of some \e environment \e variables that must be correctly defined
  depending on your current system.
  Most of the time, the CImg Library defines these variables automatically
  (for popular systems). Anyway, if your system is not recognized, you will have to set the environment
  variables by hand. Here is a quick explanations of environment variables.\n
  
  Setting the environment variables is done with the <tt>#define</tt> keyword.
  This setting must be done <i>before including the file CImg.h</i> in your source code.
  For instance,
  defining the environment variable \c cimg_display_type would be done like this :
  \code
  #define cimg_display_type 0
  #include "CImg.h"
  ...
  \endcode

  Here are the different environment variables used by the CImg Library :
  
  - \b \c cimg_OS : This variable defines the type of your Operating System. It can be set to \b 1 (\e Unix),
  \b 2 (\e Windows), or \b 0 (\e Other \e configuration).
  It should be actually auto-detected by the CImg library. If this is not the case (<tt>cimg_OS=0</tt>), you
  will probably have to tune the environment variables described below.

  - \b \c cimg_display_type : This variable defines the type of graphical library used to
  display images in windows. It can be set to 0 (no display library available), \b 1 (X11-based display) or
  \b 2 (Windows-GDI display).
  If you are running on a system without X11 or Windows-GDI ability, please set this variable to \c 0.
  This will disable the display support, since the CImg Library doesn't contain the necessary code to display
  images on systems other than X11 or Windows GDI.

  - \b \c cimg_color_terminal : This variable tells the library if the system terminal has VT100 color capabilities.
  It can be \e defined or \e not \e defined. Define this variable to get colored output on your terminal, 
  when using the CImg Library.
  
  - \b \c cimg_debug : This variable defines the level of run-time debug messages that will be displayed by
  the CImg Library. It can be set to 0 (no debug messages), 1 (normal debug messages, which is
  the default value), or 2 (high debug messages). Note that setting this value to 2 may slow down your
  program since more debug tests are made by the library (particularly to check if pixel access is made outside
  image boundaries). See also \ref CImgException to better understand how debug messages are working.
  
  - \b \c cimg_convert_path : This variables tells the library where the ImageMagick's \e convert tool is located.
  Setting this variable should not be necessary if ImageMagick is installed on a standart directory, or
  if \e convert is in your system PATH variable. This macro should be defined only if the ImageMagick's 
  \e convert tool is not found automatically, when trying to read compressed image format (GIF,PNG,...). 
  See also cimg_library::CImg::load_convert() and cimg_library::CImg::save_convert() for more informations.

  - \b \c cimg_temporary_path : This variable tells the library where it can find a directory to store
  temporary files. Setting this variable should not be necessary if you are running on a standart system.
  This macro should be defined only when troubles are encountered when trying to read
  compressed image format (GIF,PNG,...).
  See also cimg_library::CImg::load_convert() and cimg_library::CImg::save_convert() for more informations.

  - \b \c cimg_plugin : This variable tells the library to use a plugin file to add features to the CImg<T> class.
  Define it with the path of your plugin file, if you want to add member functions to the CImg<T> class,
  without having to modify directly the \c "CImg.h" file. An include of the plugin file is performed in the CImg<T>
  class. If \c cimg_plugin if not specified (default), no include is done.
  
  - \b \c cimgl_plugin : Same as \c cimg_plugin, but to add features to the CImgl<T> class.
  
  - \b \c cimgdisplay_plugin : Same as \c cimg_plugin, but to add features to the CImgDisplay<T> class.

  - \b \c cimgstats_plugin : Same as \c cimg_plugin, but to add features to the CImgStats<T> class.

  All these compilation variables can be checked, using the function cimg_library::cimg::info(), which
  displays a list of the different configuration variables and their values on the standart error output.
**/
/*@}*/

//--------------------------------------------------------------------------------------------------------------------
/** \addtogroup cimg_tutorial Tutorial : Getting Started. */
/*@{
  \page foo3
  
  Let's start to write our first program to get the idea. This will demonstrate how to load and create images, as well as handle image 
  display and mouse events.
  Assume we want to load a color image <tt>lena.jpg</tt>, smooth it, display it in a windows, and enter an event loop so that clicking a
  point in the image with the mouse will draw the intensity profiles of (R,G,B) of the corresponding image line (in another window).
  Yes, that sounds quite complex for a first code, but don't worry, it will be very simple using the CImg library ! Well, just look
  at the code below, it does the task :

  \code
  #include "CImg.h"
  using namespace cimg_library;

  int main() {
    CImg<unsigned char> image("lena.jpg"), visu(500,400,1,3,0);
    const unsigned char red[3]={255,0,0}, green[3]={0,255,0}, blue[3]={0,0,255};
    image.blur(2.5);
    CImgDisplay main_disp(image,"Click a point"), draw_disp(visu,"Intensity profile");
    while (!main_disp.closed && !draw_disp.closed) {
      main_disp.wait();
      if (main_disp.button && main_disp.mouse_y>=0) {
        const int y = main_disp.mouse_y;
    visu.fill(0).draw_graph(image.get_crop(0,y,0,0,image.dimx()-1,y,0,0),red,0,256,0);
    visu.draw_graph(image.get_crop(0,y,0,1,image.dimx()-1,y,0,1),green,0,256,0);
    visu.draw_graph(image.get_crop(0,y,0,2,image.dimx()-1,y,0,2),blue,0,256,0).display(draw_disp);
    }
      }
    return 0;
  }
  \endcode
  
  Here is a screenshot of the resulting program :

  <img SRC="../img/tutorial.jpg">

  And here is the detailled explanation of the source, line by line :

  \code #include "CImg.h" \endcode
  Include the main and only header file of the CImg library.
  \code using namespace cimg_library; \endcode
  Use the library namespace to ease the declarations afterward.
  \code int main() { \endcode
  Definition of the main function.
  \code CImg<unsigned char> image("lena.jpg"), visu(500,400,1,3,0); \endcode
  Creation of two instances of images of \c unsigned \c char pixels.
  The first image \c image is initialized by reading an image file from the disk. 
  Here, <tt>lena.jpg</tt> must be in the same directory than the current program.
  Note that you must also have installed the \e ImageMagick package in order to be able to read JPG images.
  The second image \c visu is initialized as a black color image with dimension <tt>dx=500</tt>, <tt>dy=400</tt>, 
  <tt>dz=1</tt> (here, it is a 2D image, not a 3D one), and <tt>dv=3</tt> (each pixel has 3 'vector' channels R,G,B).
  The last argument in the constructor defines the default value of the pixel values
  (here \c 0, which means that \c visu will be initially black).
  \code const unsigned char red[3]={255,0,0}, green[3]={0,255,0}, blue[3]={0,0,255}; \endcode
  Definition of three different colors as array of unsigned char. This will be used to draw plots with different colors.
  \code image.blur(2.5); \endcode
  Blur the image, with a gaussian blur and a standart variation of 2.5. Note that most of the CImg functions have two versions :
  one that acts in-place (which is the case of blur), and one that returns the result as a new image (the name of the function 
  begins then with <tt>get_</tt>&nbsp;). In this case, one could have also written <tt>image = image.get_blur(2.5);</tt>
  (more expensive, since it needs an additional copy operation).
  \code CImgDisplay main_disp(image,"Click a point"), draw_disp(visu,"Intensity profile"); \endcode
  Creation of two display windows, one for the input image image, and one for the image visu which will be display intensity profiles.
  By default, CImg displays handles events (mouse,keyboard,..). On Windows, there is a way to create fullscreen displays.
  \code while (!main_disp.closed && !draw_disp.closed) { \endcode
  Enter the event loop, the code will exit when one of the two display windows is closed.
  \code main_disp.wait(); \endcode
  Wait for an event (mouse, keyboard,..) in the display window \c main_disp.
  \code if (main_disp.button && main_disp.mouse_y>=0) { \endcode
  Test if the mouse button has been clicked on the image area.
  One may distinguish between the 3 different mouse buttons,
  but in this case it is not necessary
  \code const int y = main_disp.mouse_y; \endcode
  Get the image line y-coordinate that has been clicked.
  \code visu.fill(0).draw_graph(image.get_crop(0,y,0,0,image.dimx()-1,y,0,0),red,0,256,0); \endcode
  This line illustrates the pipeline property of most of the CImg class functions. The first function <tt>fill(0)</tt> simply sets
  all pixel values with 0 (i.e. clear the image \c visu). The interesting thing is that it returns a reference to
  \c visu and then, can be pipelined with the function \c draw_graph() which draws a plot in the image \c visu.
  The plot data are given by another image (the first argument of \c draw_graph()). In this case, the given image is 
  the red-component of the line y of the original image, retrieved by the function \c get_crop() which returns a
  sub-image of the image \c image. Remember that images coordinates are 4D (x,y,z,v) and for color images,
  the R,G,B channels are respectively given by <tt>v=0, v=1</tt> and <tt>v=2</tt>.
  \code visu.draw_graph(image.get_crop(0,y,0,1,image.dimx()-1,y,0,1),green,0,256,0); \endcode
  Plot the intensity profile for the green channel of the clicked line.
  \code visu.draw_graph(image.get_crop(0,y,0,2,image.dimx()-1,y,0,2),blue,0,256,0).display(draw_disp); \endcode
  Same thing for the blue channel. Note how the function (which return a reference to \c visu) is pipelined with the function
  \c display() that just paints the image visu in the corresponding display window.
  \code ...till the end \endcode
  I don't think you need more explanations !

  As you have noticed, the CImg library allows to write very small and intuitive code. Note also that this source will perfectly 
  work on Unix and Windows systems. Take also a look to the examples provided in the CImg package (
  directory \c examples/ ). It will show you how CImg-based code can be surprisingly small. 
  Moreover, there is surely one example close to what you want to do.
  A good start will be to look at the file <tt>CImg_test.cpp</tt> which contains small and various examples of what you can do
  with the CImg Library. All CImg classes are used in this source, and the code can be easily modified to see what happens. 

**/
/*@}*/

//----------------------------------------------------------------------------------------------------
/** \addtogroup cimg_drawing Using Drawing Functions. */
/*@{
  \page foo5

  \section s5 Using Drawing Functions.

  This section tells more about drawing features in CImg images.
  Drawing functions list can be found in <a href="structCImg.html">the CImg functions list</a>
  (section \b Drawing Functions),
  and are all defined on a common basis. Here are the important points to understand before using
  drawing functions :
  
  - Drawing is performed on the instance image. Drawing functions parameters
  are defined as \e const variables and return a reference to the current instance <tt>(*this)</tt>,
  so that drawing functions can be pipelined (see examples below).
  Drawing is usually done in 2D color images but can be performed in 3D images with any vector-valued dimension,
  and with any possible pixel type.

  - A color parameter is always needed to draw features in an image. The color must be defined as a C-style array
  whose dimension is at least

*/
/*@}*/

//----------------------------------------------------------------------------------------------------
/** \addtogroup cimg_loops Using Image Loops. */
/*@{
  \page foo_lo
  The CImg Library provides different macros that define useful iterative loops over an image.
  Basically, it can be used to replace one or several <tt>for(..)</tt> instructions, but it also proposes
  interesting extensions to classical loops.
  Below is a list of all existing loop macros, classified in four different categories :
  - \ref lo1
  - \ref lo4
  - \ref lo5
  - \ref lo6

  \section lo1 Loops over the pixel buffer

  Loops over the pixel buffer are really basic loops that iterate a pointer on the pixel data buffer
  of a \c cimg_library::CImg image. Two macros are defined for this purpose :
  
  - \b cimg_map(img,ptr,T) :
  This macro loops over the pixel data buffer of the image \c img, using a pointer <tt>T* ptr</tt>,
  starting from the end of the buffer (last pixel) till the beginning of the buffer (first pixel).
      - \c img must be a (non empty) \c cimg_library::CImg image of pixels \c T.
      - \c ptr is a pointer of type \c T*.
  This kind of loop should not appear a lot in your own source code, since this is a low-level loop
  and many functions of the CImg class may be used instead. Here is an example of use :
  \code
  CImg<float> img(320,200);
  cimg_map(img,ptr,float) { *ptr=0; }      // Equivalent to 'img.fill(0);'
  \endcode

  - \b cimg_mapoff(img,off) :
  This macro loops over the pixel data buffer of the image \c img, using an offset \c ,
  starting from the beginning of the buffer (first pixel, \c off=0)
  till the end of the buffer (last pixel value, <tt>off = img.size()-1</tt>).
      - \c img must be a (non empty) cimg_library::CImg<T> image of pixels \c T.
      - \c off is an inner-loop variable, only defined inside the scope of the loop.

  Here is an example of use :
  \code
  CImg<float> img(320,200);
  cimg_mapoff(img,off) { img[off]=0; }  // Equivalent to 'img.fill(0);'
  \endcode

  \section lo4 Loops over image dimensions

  The following loops are probably the most used loops in image processing programs.
  They allow to loop over the image along one or several dimensions, along a raster scan course.
  Here is the list of such loop macros for a single dimension :
  - \b cimg_mapX(img,x) : equivalent to : <tt>for (int x=0; x<img.dimx(); x++)</tt>.
  - \b cimg_mapY(img,y) : equivalent to : <tt>for (int y=0; y<img.dimy(); y++)</tt>.
  - \b cimg_mapZ(img,z) : equivalent to : <tt>for (int z=0; z<img.dimz(); z++)</tt>.
  - \b cimg_mapV(img,v) : equivalent to : <tt>for (int v=0; v<img.dimv(); v++)</tt>.

  Combinations of these macros are also defined as other loop macros, allowing to loop directly over 2D, 3D or 4D images :
  - \b cimg_mapXY(img,x,y) : equivalent to : \c cimg_mapY(img,y) \c cimg_mapX(img,x).
  - \b cimg_mapXZ(img,x,z) : equivalent to : \c cimg_mapZ(img,z) \c cimg_mapX(img,x).
  - \b cimg_mapYZ(img,y,z) : equivalent to : \c cimg_mapZ(img,z) \c cimg_mapY(img,y).
  - \b cimg_mapXV(img,x,v) : equivalent to : \c cimg_mapV(img,v) \c cimg_mapX(img,x).
  - \b cimg_mapYV(img,y,v) : equivalent to : \c cimg_mapV(img,v) \c cimg_mapY(img,y).
  - \b cimg_mapZV(img,z,v) : equivalent to : \c cimg_mapV(img,v) \c cimg_mapZ(img,z).
  - \b cimg_mapXYZ(img,x,y,z) : equivalent to : \c cimg_mapZ(img,z) \c cimg_mapXY(img,x,y).
  - \b cimg_mapXYV(img,x,y,v) : equivalent to : \c cimg_mapV(img,v) \c cimg_mapXY(img,x,y).
  - \b cimg_mapXZV(img,x,z,v) : equivalent to : \c cimg_mapV(img,v) \c cimg_mapXZ(img,x,z).
  - \b cimg_mapYZV(img,y,z,v) : equivalent to : \c cimg_mapV(img,v) \c cimg_mapYZ(img,y,z).
  - \b cimg_mapXYZV(img,x,y,z,v) : equivalent to : \c cimg_mapV(img,v) \c cimg_mapXYZ(img,x,y,z).

  - For all these loops, \c x,\c y,\c z and \c v are inner-defined variables only visible inside the scope of the loop.
  They don't have to be defined before the call of the macro.
  - \c img must be a (non empty) cimg_library::CImg image.

  Here is an example of use that creates an image with a smooth color gradient :
  \code
  CImg<unsigned char> img(256,256,1,3);       // Define a 256x256 color image
  cimg_mapXYV(img,x,y,v) { img(x,y,v) = (x+y)*(v+1)/6; }
  img.display("Color gradient");
  \endcode

  \section lo5 Loops over interior regions and borders.

  Similar macros are also defined to loop only on the border of an image, or inside the image (excluding the border).
  The border may be several pixel wide :

  - \b cimg_imapX(img,x,n) : Loop along the x-axis, except for pixels inside a border of \p n pixels wide.
  - \b cimg_imapY(img,y,n) : Loop along the y-axis, except for pixels inside a border of \p n pixels wide.
  - \b cimg_imapZ(img,z,n) : Loop along the z-axis, except for pixels inside a border of \p n pixels wide.
  - \b cimg_imapV(img,v,n) : Loop along the v-axis, except for pixels inside a border of \p n pixels wide.
  - \b cimg_imapXY(img,x,y,n) : Loop along the (x,y)-axes, excepted for pixels inside a border of \p n pixels wide.
  - \b cimg_imapXYZ(img,x,y,z,n) : Loop along the (x,y,z)-axes, excepted for pixels inside a border of \p n pixels wide.

  And also :

  - \b cimg_bmapX(img,x,n) : Loop along the x-axis, only for pixels inside a border of \p n pixels wide.
  - \b cimg_bmapY(img,y,n) : Loop along the y-axis, only for pixels inside a border of \p n pixels wide.
  - \b cimg_bmapZ(img,z,n) : Loop along the z-axis, only for pixels inside a border of \p n pixels wide.
  - \b cimg_bmapV(img,v,n) : Loop along the z-axis, only for pixels inside a border of \p n pixels wide.
  - \b cimg_bmapXY(img,x,y,n) : Loop along the (x,y)-axes, only for pixels inside a border of \p n pixels wide.
  - \b cimg_bmapXYZ(img,x,y,z,n) : Loop along the (x,y,z)-axes, only for pixels inside a border of \p n pixels wide.

  - For all these loops, \c x,\c y,\c z and \c v are inner-defined variables only visible inside the scope of the loop.
  They don't have to be defined before the call of the macro.
  - \c img must be a (non empty) cimg_library::CImg image.
  - The constant \c n stands for the size of the border.

  Here is an example of use, to create a 2d grayscale image with two different intensity gradients :
  \code
  CImg<> img(256,256);
  cimg_imapXY(img,x,y,50) img(x,y) = x+y;
  cimg_bmapXY(img,x,y,50) img(x,y) = x-y;
  img.display();
  \endcode

  \section lo6 Loops using neighborhoods.
  
  Inside an image loop, it is often useful to get values of neighborhood pixels of the
  current pixel at the loop location.
  The CImg Library provides a very smart and fast mechanism for this purpose, with the definition
  of several loop macros that remember the neighborhood values of the pixels.
  The use of these macros can highly optimize your code, and also simplify your program.

  \subsection lo7 Neighborhood-based loops for 2D images

  For 2D images, the neighborhood-based loop macros are : 

  - \b cimg_map2x2(img,x,y,z,v,I) : Loop along the (x,y)-axes using a centered 2x2 neighborhood.
  - \b cimg_map3x3(img,x,y,z,v,I) : Loop along the (x,y)-axes using a centered 3x3 neighborhood.
  - \b cimg_map4x4(img,x,y,z,v,I) : Loop along the (x,y)-axes using a centered 4x4 neighborhood.
  - \b cimg_map5x5(img,x,y,z,v,I) : Loop along the (x,y)-axes using a centered 5x5 neighborhood.

  For all these loops, \c x and \c y are inner-defined variables only visible inside the scope of the loop.
  They don't have to be defined before the call of the macro.
  \c img is a non empty CImg<T> image. \c z and \c v are constants that define on which image slice and
  vector channel the loop must apply (usually both 0 for grayscale 2D images).
  Finally, \c I is the 2x2, 3x3, 4x4 or 5x5 neighborhood that will be updated with the correct pixel values
  during the loop (see \ref lo9).

  \subsection lo8 Neighborhood-based loops for 3D images

  For 3D images, the neighborhood-based loop macros are : 

  - \b cimg_map2x2x2(img,x,y,z,v,I) : Loop along the (x,y,z)-axes using a centered 2x2x2 neighborhood.
  - \b cimg_map3x3x3(img,x,y,z,v,I) : Loop along the (x,y,z)-axes using a centered 3x3x3 neighborhood.

  For all these loops, \c x, \c y and \c z are inner-defined variables only visible inside the scope of the loop.
  They don't have to be defined before the call of the macro.
  \c img is a non empty CImg<T> image. \c v is a constant that defines on which image channel
  the loop must apply (usually 0 for grayscale 3D images).
  Finally, \c I is the 2x2x2 or 3x3x3 neighborhood that will be updated with the correct pixel values
  during the loop (see \ref lo9).

  \subsection lo9 Defining neighborhoods

  The CImg library defines a neighborhood as a set of named \e variables or \e references, declared
  using specific CImg macros :

  - \b CImg_2x2(I,type) : Define a 2x2 neighborhood named \c I, of type \c type.
  - \b CImg_3x3(I,type) : Define a 3x3 neighborhood named \c I, of type \c type.
  - \b CImg_4x4(I,type) : Define a 4x4 neighborhood named \c I, of type \c type.
  - \b CImg_5x5(I,type) : Define a 5x5 neighborhood named \c I, of type \c type.
  - \b CImg_2x2x2(I,type) : Define a 2x2x2 neighborhood named \c I, of type \c type.
  - \b CImg_3x3x3(I,type) : Define a 3x3x3 neighborhood named \c I, of type \c type.

  Actually, \c I is a \e generic \e name for the neighborhood. In fact, these macros declare
  a \e set of new variables.
  For instance, defining a 3x3 neighborhood \c CImg_3x3(I,float) declares 9 different float variables
  \c Ipp,\c Icp,\c Inp,\c Ipc,\c Icc,\c Inc,\c Ipn,\c Icn,\c Inn which correspond to each pixel value of
  a 3x3 neighborhood.
  Variable indices are \c p,\c c or \c n, and stand respectively for \e 'previous', \e 'current' and \e 'next'.
  First indice denotes the \c x-axis, second indice denotes the \c y-axis.
  Then, the names of the variables are directly related to the position of the corresponding pixels
  in the neighborhood. For 3D neighborhoods, a third indice denotes the \c z-axis.
  Then, inside a neighborhood loop, you will have the following equivalence :
  - <tt>Ipp = img(x-1,y-1)</tt>
  - <tt>Icn = img(x,y+1)</tt>
  - <tt>Inp = img(x+1,y-1)</tt>
  - <tt>Inpc = img(x+1,y-1,z)</tt>
  - <tt>Ippn = img(x-1,y-1,z+1)</tt>
  - and so on...

  For bigger neighborhoods, such as 4x4 or 5x5 neighborhoods, two additionnal indices are introduced :
  \c a (stands for \e 'after') and \c b (stands for \e 'before'), so that :
  - <tt>Ibb = img(x-2,y-2)</tt>
  - <tt>Ina = img(x+1,y+2)</tt>
  - and so on...

  The value of a neighborhood pixel outside the image range (image border problem) is automatically set to the same
  values than the nearest valid pixel in the image (this is also called the \e Neumann \e border \e condition).

  \subsection lo10 Neighborhood as a reference
  It is also possible to define neighborhood variables as references to classical C-arrays or CImg<T> images, instead of
  allocating new variables. This is done by adding \c _ref to the macro names used for the neighborhood definition :

  - \b CImg_2x2_ref(I,type,tab) : Define a 2x2 neighborhood named \c I, of type \c type, as a reference to \c tab.
  - \b CImg_3x3_ref(I,type,tab) : Define a 3x3 neighborhood named \c I, of type \c type, as a reference to \c tab.
  - \b CImg_4x4_ref(I,type,tab) : Define a 4x4 neighborhood named \c I, of type \c type, as a reference to \c tab.
  - \b CImg_5x5_ref(I,type,tab) : Define a 5x5 neighborhood named \c I, of type \c type, as a reference to \c tab.
  - \b CImg_2x2x2_ref(I,type,tab) : Define a 2x2x2 neighborhood named \c I, of type \c type, as a reference to \c tab.
  - \b CImg_3x3x3_ref(I,type,tab) : Define a 3x3x3 neighborhood named \c I, of type \c type, as a reference to \c tab.

  \c tab can be a one-dimensionnal C-style array, or a non empty \c CImg<T> image. Both objects must have
  same sizes as the considered neighborhoods.

  \subsection lo11 Example codes
  More than a long discussion, the above example will demonstrate how to compute the gradient norm of a 3D volume
  using the \c cimg_map3x3x3() loop macro :

  \code
  CImg<float> volume("IRM.hdr");        // Load an IRM volume from an Analyze7.5 file
  CImg_3x3x3(I,float);                  // Define a 3x3x3 neighborhood
  CImg<float> gradnorm(volume,false);   // Create an image with same size as 'volume'
  cimg_map3x3x3(volume,x,y,z,0,I) {     // Loop over the volume, using the neighborhood I
    const float ix = 0.5f*(Incc-Ipcc);  // Compute the derivative along the x-axis.
    const float iy = 0.5f*(Icnc-Icpc);  // Compute the derivative along the y-axis.
    const float iz = 0.5f*(Iccn-Iccp);  // Compute the derivative along the z-axis.
    gradnorm(x,y,z) = std::sqrt(ix*ix+iy*iy+iz*iz);  // Set the gradient norm in the destination image
  }
  gradnorm.display("Gradient norm");
  \endcode
  
  And the following example shows how to deal with neighborhood references to blur a color image by averaging
  pixel values on a 5x5 neighborhood.

  \code
  CImg<unsigned char> src("image_color.jpg"), dest(src,false), neighbor(5,5);  // Image definitions.
  typedef unsigned char uchar;             // Avoid space in the second parameter of the macro CImg_5x5 below.
  CImg_5x5_ref(N,uchar,neighbor);          // Define a 5x5 neighborhood as a reference to the 5x5 image neighbor.
  cimg_mapV(src,k)                         // Standart loop on color channels
     cimg_map5x5(src,x,y,0,k,N)            // 5x5 neighborhood loop.
       dest(x,y,k) = neighbor.sum()/(5*5); // Averaging pixels to filter the color image.
  CImgl<unsigned char> visu(src,dest);
  visu.display("Original + Filtered");     // Display both original and filtered image.
  \endcode
  
  Note that in this example, we didn't use directly the variables Nbb,Nbp,..,Ncc,... since
  there are only references to the neighborhood image \c neighbor. We rather used a member function of \c neighbor.

  As you can see, explaining the use of the CImg neighborhood macros is actually more difficult than using them !
*/
/*@}*/

//----------------------------------------------------------------------------------------------------
/** \addtogroup cimg_displays Using Display Windows. */
/*@{
  \page foo_di

  When opening a display window, you can choose the way the pixel values will be normalized
  before being displayed on the screen. Screen displays only support color values between [0,255],
  and some
  
  When displaying an image into the display window using CImgDisplay::display(), values of
  the image pixels can be eventually linearly normalized between [0,255] for visualization purposes.
  This may be useful for instance when displaying \p CImg<double> images with pixel values
  between [0,1].
  The normalization behavior depends on the value of \p normalize which can be either \p 0,\p 1 or \p 2 :
  - \p 0 : No pixel normalization is performed when displaying an image. This is the fastest
  process, but you must be sure your displayed image have pixel values inside the range [0,255].
  - \p 1 : Pixel value normalization is done for each new image display. Image pixels are
  not modified themselves, only displayed pixels are normalized.
  - \p 2 : Pixel value normalization is done for the first image display, then the
  normalization parameters are kept and used for all the next image displays.
  
*/
/*@}*/


//----------------------------------------------------------------------------------------------------
/** \addtogroup cimg_storage How pixel data are stored with CImg. */
/*@{
  \page foo_store

  TODO
*/
/*@}*/




//----------------------------------------------------------------------------------------------------
/** \addtogroup cimg_files_io Files IO in CImg. */
/*@{
  \page foo_fi

  The CImg Library can NATIVELY handle the following file formats :
  - RAW : consists in a very simple header (in ascii), then the image data.
  - ASC (Ascii)
  - HDR (Analyze 7.5)
  - INR (Inrimage)
  - PPM/PGM (Portable Pixmap)
  - BMP (uncompressed)
  - PAN (Pandore-5)
  - DLM (Matlab ASCII)
 
  If ImageMagick is installed, The CImg Library can save image in formats handled by ImageMagick : JPG, GIF, PNG, TIF,...

*/
/*@}*/

//----------------------------------------------------------------------------------------------------
/** \addtogroup cimg_options Retrieving Command Line Arguments. */
/*@{
  \page foo_so

   The CImg library offers facilities to retrieve command line arguments in a console-based
   program, as it is a commonly needed operation.
   Two macros \c cimg_usage() and \c cimg_option() are defined for this purpose.
   Using these macros allows to easily retrieve options values from the command line.
   Moreover, invoking the corresponding executable with the option \c -h or \c --help will
   automatically display the program usage, followed by the list of requested options.

   \section so1 The cimg_usage() macro

   The macro \c cimg_usage(usage) may be used to describe the program goal and usage.
   It is generally inserted one time after the <tt>int main(int argc,char **argv)</tt> definition.
  
   \param usage : A string describing the program goal and usage.
   \pre The function where \c cimg_usage() is used must have correctly defined \c argc and \c argv variables.

   \section so2 The cimg_option() macro

   The macro \c cimg_option(name,default,usage) may be used to retrieve an option value from the command line.

   \param name    : The name of the option to be retrieved from the command line.
   \param default : The default value returned by the macro if no options \p name has been specified when running the program.
   \param usage   : A brief explanation of the option. If \c usage==NULL, the option won't appear on the option list
                    when invoking the executable with options \c -h or \c --help (hidden option).
                  
   \return \c cimg_option() returns an object that has the \e same \e type than the default value \c default.
   The return value is equal to the one specified on the command line. If no such option have been specified,
   the return value is equal to the default value \c default.
   Warning, this can be confusing in some situations (look at the end of the next section).
   \pre The function where \c cimg_option() is used must have correctly defined \c argc and \c argv variables.

   \section so3 Example of use

   The code below uses the macros \c cimg_usage() and \c cimg_option().
   It loads an image, smoothes it an quantifies it with a specified number of values.
   \code
   #include "CImg.h"
   using namespace cimg_library;
   int main(int argc,char **argv) {
     cimg_usage("Retrieve command line arguments");
     const char* filename = cimg_option("-i","image.gif","Input image file");
     const char* output   = cimg_option("-o",(const char*)NULL,"Output image file");
     const double sigma   = cimg_option("-s",1.0,"Standart variation of the gaussian smoothing");
     const  int nblevels  = cimg_option("-n",16,"Number of quantification levels");
     const bool hidden    = cimg_option("-hidden",false,NULL);      // This is a hidden option

     CImg<unsigned char> img(filename);
     img.blur(sigma).quantify(nblevels);
     if (output) img.save(output); else img.display("Output image");
     if (hidden) std::fprintf(stderr,"You found me !\n");
     return 0;
   }
   \endcode

   Invoking the corresponding executable with <tt>test -h -hidden -n 20 -i foo.jpg</tt> will display :
   \verbatim
$ ./test -h -hidden -n 20 -i foo.jpg

 test : Retrieve command line arguments (Oct 16 2004, 12:34:26)

    -i       = foo.jpg      : Input image file
    -o       = NULL         : Output image file
    -s       = 1            : Standart variation of the gaussian smoothing
    -n       = 20           : Number of quantification levels

   You found me !
\endverbatim

   \warning As the type of object returned by the macro \c cimg_option(option,default,usage) 
   is defined by the type of \c default, undesired casts may appear when writting code such as :
   \code
   const double sigma = cimg_option("-val",0,"A floating point value");
   \endcode
   In this case, \c sigma will always be equal to an integer (since the default value \c 0 is an integer).
   When passing a float value on the command line, a \e float \e to \e integer cast is then done,
   truncating the given parameter to an integer value (this is surely not a desired behavior).
   You must specify <tt>0.0</tt> as the default value in this case.

   \section so4 How to learn more about command line options ?
   You should take a look at the examples <tt>examples/inrcast.cpp</tt> provided in the CImg Library package.
   This is a command line based image converter which intensively uses the \c cimg_option() and \c cimg_usage()
   macros to retrieve command line parameters.
*/
/*@}*/
//----------------------------------------------------------------------------------------------------
#endif

// Local Variables:
// mode: c++
// End:
