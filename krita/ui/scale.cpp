// This file includes code for scaling images, in two versions.
// One ported from ImageMagick (slower, but can achieve better quality),
// and from Imlib2 ported by Mosfet (very fast).

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

#include "scale.h"

#include <QDebug>
#include <QImage>
#include <QRect>
#include <QSize>

#include <kcpuinfo.h>

#include <config-endian.h> // WORDS_BIGENDIAN
//#include <config-processor.h>

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
    MImageScaleInfo *mimageCalcScaleInfo(const QImage &img, int sw, int sh,
                                         int dw, int dh, char aa, int sow);
    void mimageScaleAARGBA(MImageScaleInfo *isi, unsigned int *dest, int dxx,
                           int dyy, int dx, int dy, int dw, int dh, int dow,
                           int sow);
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
    //qDebug() << "smoothScale " << image.rect() << ", " << image.format() << "," << image.hasAlphaChannel() << " (" << dw << "," << dh << ")\n";

    // We only do 32 bit images with alpha channel
    QImage img = image;

    int w = img.width();
    int h = img.height();

    int sow = img.bytesPerLine() / ( img.depth() / 8 );

    MImageScaleInfo *scaleinfo = mimageCalcScaleInfo(img, w, h, dw, dh, true, sow);
    if(!scaleinfo)
        return QImage();

    QImage buffer( dw, dh, QImage::Format_ARGB32 );

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
        mimageScaleAARGBA(scaleinfo, (unsigned int *)buffer.scanLine(0), 0, 0,
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
#ifdef WORDS_BIGENDIAN
#define A_VAL(p) ((unsigned char *)(p))[0]
#define R_VAL(p) ((unsigned char *)(p))[1]
#define G_VAL(p) ((unsigned char *)(p))[2]
#define B_VAL(p) ((unsigned char *)(p))[3]
#else
#define A_VAL(p) ((unsigned char *)(p))[3]
#define R_VAL(p) ((unsigned char *)(p))[2]
#define G_VAL(p) ((unsigned char *)(p))[1]
#define B_VAL(p) ((unsigned char *)(p))[0]
#endif

#define INV_XAP                   (256 - xapoints[x])
#define XAP                       (xapoints[x])
#define INV_YAP                   (256 - yapoints[dyy + y])
#define YAP                       (yapoints[dyy + y])

unsigned int** MImageScale::mimageCalcYPoints(unsigned int *src,
                                              int sow, int sh, int dh)
{
    //qDebug() << "mimageCalcYPoints. sow: " << sow << ", sh: " << sh << ", dh: " << dh << endl;
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
    //qDebug() << "mimageCalcXPoints. sw: " << sw << ", dw: " << dw << endl;
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
    //qDebug() << "mimageCalcAPoints. s: " << s << ", d: " << d << ", up: " << up << endl;
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

MImageScaleInfo* MImageScale::mimageCalcScaleInfo(const QImage &img, int sw, int sh,
                                                  int dw, int dh, char aa, int sow)
{
    //qDebug() << "mimageCalcScaleInfo. img " << img.rect() << ", sw " << sw << ", sh " << sh
    //         << ", dw " << dw << ", dh " << dh << ", aa " << aa << ", sow " << sow << endl;
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


/* FIXME: NEED to optimise ScaleAARGBA - currently its "ok" but needs work*/

/* scale by area sampling */
void MImageScale::mimageScaleAARGBA(MImageScaleInfo *isi, unsigned int *dest,
                                    int dxx, int dyy, int dx, int dy, int dw,
                                    int dh, int dow, int sow)
{
    //qDebug() << "mimageScaleAARGBA " << endl;
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

// public functions :
// ------------------


QImage ImageUtils::scale(const QImage& image, int width, int height, Qt::AspectRatioMode mode)
{
    if( image.isNull()) return image.copy();

    QSize newSize( image.size() );
    newSize.scale( QSize( width, height ), mode ); // ### remove cast in Qt 4.0
    newSize = newSize.expandedTo( QSize( 1, 1 )); // make sure it doesn't become null

    //qDebug() << "Scaling image " << image.rect() << " to " << newSize << endl;

    if ( newSize == image.size() ) return image.copy();

    return MImageScale::smoothScale( image, newSize.width(), newSize.height() );

}

