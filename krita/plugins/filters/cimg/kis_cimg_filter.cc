/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *
 * Ported from the CImg Gimp plugin by Victor Stinner and uses CImg by David Tschumperlé.
 * See: http://www.girouette-stinner.com/castor/gimp.html?girouette=ad761bc2f4dcfda1cb44c587da17f86c
 */

#include <stdlib.h>
#include <vector>

#include <qpoint.h>
#include <qspinbox.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>

#include "kis_cimgconfig_widget.h"
#include "kis_cimg_filter.h"

#include "CImg.h"

using namespace cimg_library;
typedef unsigned char uchar;


KisCImgFilterConfiguration::KisCImgFilterConfiguration()
    : KisFilterConfiguration("cimg", 1)
{
    nb_iter = 1;
    dt = 20.0;
    sigma = 1.4;
    dlength = 0.8;
    dtheta = 45.0;
    onormalize = false;
    power1 = 0.1;
    power2 = 0.9;
    gauss_prec = 3.0;
    linear = true;
}

void KisCImgFilterConfiguration::fromXML(const QString & s)
{
    KisFilterConfiguration::fromXML( s );
    
    nb_iter = getInt("nb_iter", 1);
    dt = getDouble("dt", 20.0);
    sigma = getDouble("sigma", 1.4);
    dlength  = getDouble("dlength", 0.8);
    dtheta = getDouble("dtheta", 45.0);
    onormalize = getBool("onormalize", false);
    power1 = getDouble("power1", 0.1);
    power2 = getDouble("power2", 0.9);
    gauss_prec = getDouble("gauss_pref", 3.0);
    linear = getBool("linear", true);
}


QString KisCImgFilterConfiguration::toString()
{
    m_properties.clear();
    
    setProperty("nb_iter", nb_iter);
    setProperty("dt", dt);
    setProperty("sigma", sigma);
    setProperty("dlength", dlength);
    setProperty("dtheta", dtheta);
    setProperty("onormalize", onormalize);
    setProperty("power1", power1);
    setProperty("power2", power2);
    setProperty("gauss_prec", gauss_prec);
    setProperty("linear", linear);

    return KisFilterConfiguration::toString();
}

KisCImgFilter::KisCImgFilter()
    : KisFilter(id(), "enhance", i18n("&CImg Image Restoration...")),
      eigen(CImg<>(2,1), CImg<>(2,2))
{
    restore = true;
    inpaint = false;
    resize = false;
    visuflow = NULL;

    /* restore */
    nb_iter        = 1;
    dt             = 20.0f;
    sigma          = 0.8f;
    dlength        = 0.8;
    dtheta         = 45.0;
    onormalize     = false;
    power1         = 0.5;
    power2         = 0.9;

    /* inpainting *
       nb_iter                    = 100;
       dt                         = 50.0f;
       sigma                      = 2.0;
       power1                     = 0.1;
       power2                     = 100;
       dlength                    = 0.8;
       dtheta                     = 45.0;
    */

    /* resize *
       nb_iter           = 1;
       dt                = 30.0f;
       sigma             = 2.0;
       dlength           = 0.8;
       dtheta            = 45.0;
       power1            = 0.01;
       power2            = 100.0;
    */

    /* visualflow *
       nb_iter              = 1;
       dt                   = 30.0f;
       dlength              = 0.5;
       dtheta               = 20.0;
       onormalize = false;
    */

    gauss_prec  = 3.0f;
    linear = true;
}


void KisCImgFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{
    Q_UNUSED(dst);

    qint32 width = rect.width();
    qint32 height = rect.height();

     // Copy the src data into a CImg type image with three channels and no alpha.
    // XXX: This means that a CImg is always rgba; find the quickest way to get 8-bit rgb from any colorspace & find a way
    //      to warn in the gui of loss of precision. XXX: Add this to the ColorSpaceAPI doc.

    img = CImg<>(width, height, 1, 3);

    KisRectIteratorPixel it = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
    KisColorSpace * cs = src->colorSpace();

    while (!it.isDone()) {

        QColor color;
        cs->toQColor(it.rawData(), &color);

        qint32 x = it.x() - rect.x();
        qint32 y = it.y() - rect.y();

        img(x, y, 0) = color.red();
        img(x, y, 1) = color.green();
        img(x, y, 2) = color.blue();

        ++it;
    }

    // Copy the config data into local variables for easy cut & pasting from the original plugin

    KisCImgFilterConfiguration * cfg = (KisCImgFilterConfiguration*)configuration;

        nb_iter = cfg->nb_iter;
        dt = cfg->dt;
        dlength = cfg->dlength;
        dtheta = cfg->dtheta;
        sigma = cfg->sigma;
        power1 = cfg->power1;
        power2 = cfg->power2;
        gauss_prec = cfg->gauss_prec;
        onormalize = cfg->onormalize;
        linear = cfg->linear;

    if (process() && !cancelRequested()) {

        it = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true);

        while (!it.isDone()) {

            if (it.isSelected()) {

                qint32 x = it.x() - rect.x();
                qint32 y = it.y() - rect.y();

                QColor color((int)img(x, y, 0), (int)img(x, y, 1), (int)img(x, y, 2));

                cs->fromQColor(color, it.rawData());
            }

            ++it;
        }
    } else {
        // Everything went wrong; notify user and restore old state
    }

}

//----------------------------------------------------------------------------
//  Cut & Pasted code starts here....
//----------------------------------------------------------------------------

void get_geom(const char *geom, int &geom_w, int &geom_h)
{
    char tmp[16];
    std::sscanf(geom,"%d%7[^0-9]%d%7[^0-9]",&geom_w,tmp,&geom_h,tmp+1);
    if (tmp[0]=='%') geom_w=-geom_w;
    if (tmp[1]=='%') geom_h=-geom_h;
}


//----------------------------------------------------------------------------

void KisCImgFilter::cleanup()
{
    img0 = flow = G = dest = sum= W = CImg<>();
    mask = CImg<uchar> ();
}

//----------------------------------------------------------------------------

bool KisCImgFilter::prepare()
{
    if (!restore && !inpaint && !resize && !visuflow)
    {
        // XXX: Do KDE messagebox
        // g_message ("You must specify one of the restore, inpaint, resize or flow mode !");
        return false;
    }

    // Init algorithm parameters
    //---------------------------
    if (restore) if (!prepare_restore()) return false;
    if (inpaint) if (!prepare_inpaint()) return false;
    if (resize) if (!prepare_resize()) return false;
    if (visuflow) if (!prepare_visuflow()) return false;

    if (!check_args()) return false;

    // Init images
    //------------
    dest = CImg<>(img.width,img.height,1,img.dim);
    sum = CImg<>(img.width,img.height,1);
    W = CImg<>(img.width,img.height,1,2);

    return true;
}

//----------------------------------------------------------------------------

bool KisCImgFilter::check_args()
{
    if (power2 < power1)
    {
        // XXX: Do KDE messagebox
        // g_message ("Error : p2<p1 !");
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------

bool KisCImgFilter::prepare_restore()
{
    CImgStats stats(img, false);
    img.normalize((float)stats.min, (float)stats.max);
    img0 = img;
    G = CImg<>(img.width,img.height,1,3);
    return true;
}

//----------------------------------------------------------------------------

bool KisCImgFilter::prepare_inpaint()
{
    const char *file_m         = NULL; //cimg_option("-m",(const char*)NULL,"Input inpainting mask");
    if (!file_m)
    {
        // XXX: Do KDE messagebox
        // g_message ("You need to specify an inpainting mask (option '-m') !");
        return false;
    }

    const unsigned int dilate  = 0; //cimg_option("-dilate",0,"Inpainting mask dilatation");
    const unsigned int ip_init = 3; //cimg_option("-init",3,"Inpainting init (0=black, 1=white, 2=noise, 3=unchanged, 4=interpol)");
    if (cimg::strncasecmp("block",file_m,5))
        mask = CImg<uchar>(file_m);
    else {
        int l=16; std::sscanf(file_m,"block%d",&l);
        mask = CImg<uchar>(img.width/l,img.height/l);
        cimg_mapXY(mask,x,y) mask(x,y)=(x+y)%2;
    }
    mask.resize(img.width,img.height,1,1);
    if (dilate) mask.dilate(dilate);
    switch (ip_init) {
    case 0 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = 0; } break;
    case 1 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = 255; } break;
    case 2 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = (float)(255*cimg::rand()); } break;
    case 3 : break;
    case 4 : {
        CImg<uchar> tmask(mask),ntmask(tmask);
        CImg_3x3(M,uchar);
        CImg_3x3(I,float);
        while (CImgStats(ntmask,false).max>0) {
            cimg_map3x3(tmask,x,y,0,0,M) if (Mcc && (!Mpc || !Mnc || !Mcp || !Mcn)) {
                const float ccp = Mcp?0.0f:1.0f, cpc = Mpc?0.0f:1.0f,
                    cnc = Mnc?0.0f:1.0f, ccn = Mcn?0.0f:1.0f, csum = ccp + cpc + cnc + ccn;
                cimg_mapV(img,k) {
                    cimg_get3x3(img,x,y,0,k,I);
                    img(x,y,k) = (ccp*Icp + cpc*Ipc + cnc*Inc + ccn*Icn)/csum;
                }
                ntmask(x,y) = 0;
            }
            tmask = ntmask;
        }
    } break;
    default: break;
    }
    img0=img;
    G = CImg<>(img.width,img.height,1,3,0);
    CImg_3x3(g,uchar);
    CImg_3x3(I,float);
    cimg_map3x3(mask,x,y,0,0,g) if (!gcc && !(gnc-gcc) && !(gcc-gpc) && !(gcn-gcc) && !(gcc-gcp)) cimg_mapV(img,k) {
        cimg_get3x3(img,x,y,0,k,I);
        const float ix = 0.5f*(Inc-Ipc), iy = 0.5f*(Icn-Icp);
        G(x,y,0)+= ix*ix; G(x,y,1)+= ix*iy; G(x,y,2)+= iy*iy;
    }
    G.blur(sigma);
    { cimg_mapXY(G,x,y)
        {
            G.get_tensor(x,y).symeigen(eigen(0),eigen(1));
            const float
                l1 = eigen(0)[0],
                l2 = eigen(0)[1],
                u = eigen(1)[0],
                v = eigen(1)[1],
                ng = (float)std::sqrt(l1+l2),
                n1 = (float)(1.0/std::pow(1+ng,power1)),
                n2 = (float)(1.0/std::pow(1+ng,power2)),
                sr1 = (float)std::sqrt(n1),
                sr2 = (float)std::sqrt(n2);
            G(x,y,0) = sr1*u*u + sr2*v*v;
            G(x,y,1) = u*v*(sr1-sr2);
            G(x,y,2) = sr1*v*v + sr2*u*u;
        }
    }
    return true;
}

//----------------------------------------------------------------------------

bool KisCImgFilter::prepare_resize()
{
    const char *geom  = NULL; //cimg_option("-g",(const char*)NULL,"Output image geometry");
    const bool anchor = true; //cimg_option("-anchor",true,"Anchor original pixels");
    if (!geom) throw CImgArgumentException("You need to specify an output geomety (option -g)");
    int w,h; get_geom(geom,w,h);
    mask = CImg<uchar>(img.width,img.height,1,1,255);
    if (!anchor) mask.resize(w,h,1,1,1); else mask = ~mask.resize(w,h,1,1,4);
    img0 = img.get_resize(w,h,1,-100,1);
    img.resize(w,h,1,-100,3);
    G = CImg<>(img.width,img.height,1,3);
    return true;
}

//----------------------------------------------------------------------------

bool KisCImgFilter::prepare_visuflow()
{
    const char *geom     = "100%x100%"; //cimg_option("-g","100%x100%","Output geometry");
    //const char *file_i   = (const char *)NULL; //cimg_option("-i",(const char*)NULL,"Input init image");
    const bool normalize = false; //cimg_option("-norm",false,"Normalize input flow");

    int w,h; get_geom(geom,w,h);
    if (!cimg::strcasecmp(visuflow,"circle")) { // Create a circular vector flow
        flow = CImg<>(400,400,1,2);
        cimg_mapXY(flow,x,y) {
            const float ang = (float)(std::atan2(y-0.5*flow.dimy(),x-0.5*flow.dimx()));
            flow(x,y,0) = -(float)std::sin(ang);
            flow(x,y,1) = (float)std::cos(ang);
        }
    }
    if (!cimg::strcasecmp(visuflow,"radial")) { // Create a radial vector flow
        flow = CImg<>(400,400,1,2);
        cimg_mapXY(flow,x,y) {
            const float ang = (float)(std::atan2(y-0.5*flow.dimy(),x-0.5*flow.dimx()));
            flow(x,y,0) = (float)std::cos(ang);
            flow(x,y,1) = (float)std::sin(ang);
        }
    }
    if (!flow.data) flow = CImg<>(visuflow);
    flow.resize(w,h,1,2,3);
    if (normalize) flow.orientation_pointwise();
    /*    if (file_i) img = CImg<>(file_i);
          else img = CImg<>(flow.width,flow.height,1,1,0).noise(100,2); */
    img0=img;
    img0.fill(0);
    float color[3]={255,255,255};
    img0.draw_quiver(flow,color,15,-10);
    G = CImg<>(img.width,img.height,1,3);
    return true;
}

//----------------------------------------------------------------------------

void KisCImgFilter::compute_smoothed_tensor()
{
    if (visuflow || inpaint) return;
    CImg_3x3(I,float);
    G.fill(0);
    cimg_mapV(img,k) cimg_map3x3(img,x,y,0,k,I) {
        const float ix = 0.5f*(Inc-Ipc), iy = 0.5f*(Icn-Icp);
        G(x,y,0)+= ix*ix; G(x,y,1)+= ix*iy; G(x,y,2)+= iy*iy;
    }
    G.blur(sigma);
}

//----------------------------------------------------------------------------

void KisCImgFilter::compute_normalized_tensor()
{
    if (restore || resize) cimg_mapXY(G,x,y) {
        G.get_tensor(x,y).symeigen(eigen(0),eigen(1));
        const float
            l1 = eigen(0)[0],
            l2 = eigen(0)[1],
            u = eigen(1)[0],
            v = eigen(1)[1],
            n1 = (float)(1.0/std::pow(1.0f+l1+l2,0.5f*power1)),
            n2 = (float)(1.0/std::pow(1.0f+l1+l2,0.5f*power2));
        G(x,y,0) = n1*u*u + n2*v*v;
        G(x,y,1) = u*v*(n1-n2);
        G(x,y,2) = n1*v*v + n2*u*u;
    }
    if (visuflow) cimg_mapXY(G,x,y) {
        const float
            u = flow(x,y,0),
            v = flow(x,y,1),
            n = (float)std::pow(u*u+v*v,0.25f),
            nn = n<1e-5?1:nn;
        G(x,y,0) = u*u/nn;
        G(x,y,1) = u*v/nn;
        G(x,y,2) = v*v/nn;
    }

    const CImgStats stats(G,false);
    G /= cimg::max(std::fabs(stats.max), std::fabs(stats.min));
}

//----------------------------------------------------------------------------

void KisCImgFilter::compute_W(float cost, float sint)
{
    cimg_mapXY(W,x,y) {
        const float
            a = G(x,y,0),
            b = G(x,y,1),
            c = G(x,y,2),
            u = a*cost + b*sint,
            v = b*cost + c*sint;
        W(x,y,0) = u;
        W(x,y,1) = v;
    }
}

//----------------------------------------------------------------------------

void KisCImgFilter::compute_LIC_back_forward(int x, int y)
{
    float l, X,Y, cu, cv, lsum=0;
    const float
        fsigma2 = 2*dt*(W(x,y,0)*W(x,y,0) + W(x,y,1)*W(x,y,1)),
        length = gauss_prec*(float)std::sqrt(fsigma2);

    if (linear) {

        // Integrate with linear interpolation
        cu = W(x,y,0); cv = W(x,y,1); X=(float)x; Y=(float)y;
        for (l=0; l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=dlength) {
            float u = (float)(W.linear_pix2d(X,Y,0)), v = (float)(W.linear_pix2d(X,Y,1));
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X+=dlength*u; Y+=dlength*v; cu=u; cv=v; lsum+=coef;
        }
        cu = W(x,y,0); cv = W(x,y,1); X=x-dlength*cu; Y=y-dlength*cv;
        for (l=dlength; l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=dlength) {
            float u = (float)(W.linear_pix2d(X,Y,0)), v = (float)(W.linear_pix2d(X,Y,1));
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X-=dlength*u; Y-=dlength*v; cu=u; cv=v; lsum+=coef;
        }
    } else {

        // Integrate with non linear interpolation
        cu = W(x,y,0); cv = W(x,y,1); X=(float)x; Y=(float)y;
        for (l=0; l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=dlength) {
            float u = W((int)X,(int)Y,0), v = W((int)X,(int)Y,1);
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X+=dlength*u; Y+=dlength*v; cu=u; cv=v; lsum+=coef;
        }
        cu = W(x,y,0); cv = W(x,y,1); X=x-dlength*cu; Y=y-dlength*cv;
        for (l=dlength; l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=dlength) {
            float u = W((int)X,(int)Y,0), v = W((int)X,(int)Y,1);
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X-=dlength*u; Y-=dlength*v; cu=u; cv=v; lsum+=coef;
        }
    }
    sum(x,y)+=lsum;
}

//----------------------------------------------------------------------------

void KisCImgFilter::compute_LIC(int &progressSteps)
{
    dest.fill(0);
    sum.fill(0);
    for (float theta=(180%(int)dtheta)/2.0f; theta<180; theta+=dtheta)
    {
        const float
            rad = (float)(theta*cimg::PI/180.0),
            cost = (float)std::cos(rad),
            sint = (float)std::sin(rad);

        // Compute vector field w = sqrt(T)*a_alpha
        compute_W(cost, sint);

        // Compute the LIC along w in backward and forward directions
        cimg_mapXY(dest,x,y)
        {
            setProgress(progressSteps);
            progressSteps++;

            if (cancelRequested()) {
                return;
            }

            if (!mask.data || mask(x,y)) compute_LIC_back_forward(x,y);
        }
    }

}

//----------------------------------------------------------------------------

void KisCImgFilter::compute_average_LIC()
{
    cimg_mapXY(dest,x,y)
    {
        if (sum(x,y)>0)
            cimg_mapV(dest,k) dest(x,y,k) /= sum(x,y);
        else
            cimg_mapV(dest,k) dest(x,y,k) = img(x,y,k);
    }
}


bool KisCImgFilter::process()
{
        if (!prepare()) return false;

    setProgressTotalSteps(dest.width * dest.height * nb_iter * (int)ceil(180 / dtheta));
    setProgressStage(i18n("Applying image restoration filter..."), 0);

        //-------------------------------------
        // Begin regularization PDE iterations
        //-------------------------------------
        int progressSteps = 0;
        for (unsigned int iter=0; iter<nb_iter; iter++)
        {
                // Compute smoothed structure tensor field G
                compute_smoothed_tensor();

                // Compute normalized tensor field sqrt(T) in G
                compute_normalized_tensor();

                // Compute LIC's along different angle projections a_\alpha
                compute_LIC(progressSteps);

        if (cancelRequested()) {
            break;
        }

                // Average all the LIC's
                compute_average_LIC();

                // Next step
                img = dest;
        }

    setProgressDone();
        // Save result and end program
        //-----------------------------
        if (visuflow) dest.mul(flow.get_norm_pointwise()).normalize(0,255);
        if (onormalize) dest.normalize(0,255);
        cleanup();
        return true;
}

KisFilterConfigWidget * KisCImgFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP /*dev*/)
{
    return new KisCImgconfigWidget(this, parent);
}

KisFilterConfiguration* KisCImgFilter::configuration(QWidget* nwidget)
{
    KisCImgconfigWidget * widget = (KisCImgconfigWidget *) nwidget;
    if( widget == 0 )
    {
        KisCImgFilterConfiguration * cfg = new KisCImgFilterConfiguration();
        Q_CHECK_PTR(cfg);
        return cfg;

    } else {
        return widget->config();
    }
}

