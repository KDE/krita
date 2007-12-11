/*
 * kis_image_alignment.cpp -- Part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_image_alignment.h"

#include <eigen/vector.h>
#include <eigen/matrix.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_paint_device.h>

#include "harris_detector.h"
#include "matching.h"
#include "ransac.h"
#include "imagoptim_p.h"
#include "imagematchmodel_p.h"
#include "homographyimagematchmodel_p.h"

struct KisImageAlignment::Private
{
    KisInterestPointsDetector* interestPointsDetector;
};

KisImageAlignment::KisImageAlignment(KisInterestPointsDetector* ipd) : d(new Private)
{
    d->interestPointsDetector = ipd;
}
KisImageAlignment::~KisImageAlignment()
{
    delete d;
}

std::vector<double> KisImageAlignment::align(QList<ImageInfo> images)
{
    std::vector<double> p(HomographySameDistortionFunction::SIZEINDEXES);
    int width = 1000; // TODO TMP variable
    int height = 1000; // TODO TMP variable
    kDebug(41006) <<"Creating panorama with" << images.size() <<" images";
    kDebug(41006) <<"Detecting interest points";
    const KoColorSpace* graycs = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
    if(not graycs)
    {
        kDebug(41006) <<"Gray 8bit is not installed."; // TODO: message box
        return std::vector<double>();
    }
    for(QList<KisImageAlignment::ImageInfo>::iterator it = images.begin(); it != images.end(); ++it)
    {
        KisPaintDeviceSP graydevice = new KisPaintDevice(*(it->device));
        graydevice->convertTo(graycs);
/*        QImage img = graydevice->convertToQImage(0,0.0);
        img.save("test.png", "PNG");*/
        it->points = KisInterestPointsDetector::interestPointDetector()->computeInterestPoints(graydevice, it->rect);
        width = it->rect.width();
        height = it->rect.height();
        for(lInterestPoints::const_iterator itp = it->points.begin(); itp != it->points.end(); ++itp)
        {
          kDebug(41006) <<"ip =" << (*itp)->x() <<"" << (*itp)->y() <<"" << (*itp)->toString();
        }
    }
    lMatches mp = matching(images[0].points, images[1].points);
/*    for(lMatches::const_iterator it = mp.begin(); it != mp.end(); ++it)
    {
        kDebug(41006) <<"init match =" << it->ref->x() <<"" << it->ref->y() <<"" << it->match->x() <<"" <<  it->match->y();
    }*/
    Ransac<ImageMatchModel, void, KisMatch> ransac(2,100,0);
    std::list<ImageMatchModel*> models = ransac.findModels( mp );
//     Ransac<HomographyImageMatchModel, HomographyImageMatchModelStaticParams, KisMatch> ransac(2,100, new HomographyImageMatchModelStaticParams(width, height));
//     std::list<HomographyImageMatchModel*> models = ransac.findModels( mp );
    if(not models.empty())
    {
        kDebug(41006) <<"Best model:" << (*models.begin())->fittingErrorSum() <<" with" << (*models.begin())->matches().size() <<" points";
#if 0
        std::vector<double> p = (*models.begin())->parameters();
#endif
#if 1
        Eigen::Matrix3d transfo = (*models.begin())->transfo();
        kDebug(41006) <<"Translation :" << transfo(0,2) <<"" << transfo(1,2);

        for(std::list<ImageMatchModel*>::iterator it = models.begin(); it != models.end(); it++)
        {
            kDebug(41006) <<" Error:" << (*it)->fittingErrorSum() <<" with" << (*it)->matches().size();
        }
//
//         for(lMatches::const_iterator it = (*models.begin())->matches().begin(); it != (*models.begin())->matches().end(); ++it)
//         {
//             kDebug(41006) <<"selected match =" << it->ref->x() <<"" << it->ref->y() <<"" << it->match->x() <<"" <<  it->match->y();
//         }

        PanoptimFunction<HomographySameDistortionFunction, HomographySameDistortionFunction::SIZEINDEXES> f( (*models.begin())->matches(), width * 0.5, height * 0.5, width, height );


        p[HomographySameDistortionFunction::INDX_a] = 0.0;
        p[HomographySameDistortionFunction::INDX_b] = 0.0;
        p[HomographySameDistortionFunction::INDX_c] = 0.0;
        p[HomographySameDistortionFunction::INDX_h11] = 1.0; // << TODO we compute a rotation in the ransac model, use that
        p[HomographySameDistortionFunction::INDX_h21] = 0.0;
        p[HomographySameDistortionFunction::INDX_h31] = transfo(0,2);
        p[HomographySameDistortionFunction::INDX_h12] = 0.0;
        p[HomographySameDistortionFunction::INDX_h22] = 1.0; // << TODO we compute a rotation in the ransac model, use that
        p[HomographySameDistortionFunction::INDX_h32] = transfo(1,2);
        p[HomographySameDistortionFunction::INDX_h13] = 0.0;
        p[HomographySameDistortionFunction::INDX_h23] = 0.0;
    //       double r = Optimization::Algorithms::gaussNewton< Optimization::Methods::GaussNewton< PanoptimFunction, double> >(&f, p, 100, 1e-12);
        double r = Optimization::Algorithms::levenbergMarquardt(&f, p, 100, 1e-12, 0.01, 10.0);
        kDebug(41006) <<"Remain =" << r;
    //       kDebug(41006) <<"Remain =" << Optimization::gradientDescent(&f, p, 10000, 1e-3, 1e-6);
        for(uint i = 0; i < p.size(); i++)
        {
            kDebug(41006) <<"p["<< i <<"]=" << p[i];
        }
        #if 0
        // Attempt a second optimization by completing the match base
        kDebug(41006) <<"Complete match base";
        lMatches sndmatches;
        for(lMatches::const_iterator it = mp.begin(); it != mp.end(); ++it)
        {

          int indx[HomographySameDistortionFunction::SIZEINDEXES];
          for(int i = 0; i < HomographySameDistortionFunction::SIZEINDEXES; i++)
          {
            indx[i] = i;
          }
          double norm(4.0 / ( width * width + height * height ) );
          HomographySameDistortionFunction hsdf(indx, width * 0.5, height * 0.5, norm, it->ref->x(), it->ref->y(), it->match->x(), it->match->y());
          double f1, f2;
          hsdf.f(p, f1, f2);
          if( fabs(f1) < 10.0 and fabs(f2) < 10.0)
          {
            kDebug(41006) <<"Match accepted" << it->ref->x() <<"" << it->ref->y() <<"" << it->match->x() <<"" <<  it->match->y() <<" f1 =" << f1 <<" f2 =" << f2;
            sndmatches.push_back(*it);
          } else {
            kDebug(41006) <<"Match rejected" << it->ref->x() <<"" << it->ref->y() <<"" << it->match->x() <<"" <<  it->match->y() <<" f1 =" << f1 <<" f2 =" << f2;
          }
        }

        kDebug(41006) <<"Start second optimization with" << sndmatches.size() <<" matches";
        PanoptimFunction<HomographySameDistortionFunction, HomographySameDistortionFunction::SIZEINDEXES> f2( sndmatches, width * 0.5, height * 0.5, width, height );
        r = Optimization::Algorithms::levenbergMarquardt(&f2, p, 1000, 1e-12, 0.01, 10.0);
        kDebug(41006) <<"Remain =" << r;
    //       kDebug(41006) <<"Remain =" << Optimization::gradientDescent(&f, p, 10000, 1e-3, 1e-6);
        #endif

    //       kDebug(41006) <<"a1 =" << p[0] <<" b1 =" << p[1] <<" a2 =" << p[2] <<" b2 =" << p[3] <<" rotation =" << p[4] <<" tx21 =" << p[5] <<" ty21 =" << p[6];
//         kDebug(41006) << transfo(0,0) <<"" << transfo(1,1) <<"" << cos(p[4]) <<"" << cos(p[5]);
#endif

        for(uint i = 0; i < p.size(); i++)
        {
            kDebug(41006) <<"p["<< i <<"]=" << p[i];
        }
    } else {
        kDebug(41006) <<"No models found";
    }
    return p;
}
