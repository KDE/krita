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

std::vector<KisImageAlignment::Result> KisImageAlignment::align(QList<ImageInfo> images)
{
    std::vector<Result> result(images.size());
    std::vector<double> p(DoubleHomographySameDistortionFunction::SIZEINDEXES);
    int width = 1000; // TODO TMP variable
    int height = 1000; // TODO TMP variable
    kDebug(41006) <<"Creating panorama with" << images.size() <<" images";
    kDebug(41006) <<"Detecting interest points";
    const KoColorSpace* graycs = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
    if(not graycs)
    {
        kDebug(41006) <<"Gray 8bit is not installed."; // TODO: message box
        return std::vector<Result>();
    }
    for(QList<KisImageAlignment::ImageInfo>::iterator it = images.begin(); it != images.end(); ++it)
    {
        KisPaintDeviceSP graydevice = new KisPaintDevice(*(it->device));
        graydevice->convertTo(graycs);
        it->points = KisInterestPointsDetector::interestPointDetector()->computeInterestPoints(graydevice, it->rect);
        width = it->rect.width();
        height = it->rect.height();
        for(lInterestPoints::const_iterator itp = it->points.begin(); itp != it->points.end(); ++itp)
        {
          kDebug(41006) <<"ip =" << (*itp)->x() <<"" << (*itp)->y() <<"" << (*itp)->toString();
        }
    }
    lMatches mp = matching(images[0].points, images[1].points);
    Ransac<ImageMatchModel, void, KisMatch> ransac(2,100,0);
    std::list<ImageMatchModel*> models = ransac.findModels( mp );
    if(not models.empty())
    {
        kDebug(41006) <<"Best model:" << (*models.begin())->fittingErrorSum() <<" with" << (*models.begin())->matches().size() <<" points";
        Eigen::Matrix3d transfo = (*models.begin())->transfo();
        kDebug(41006) <<"Translation :" << transfo(0,2) <<"" << transfo(1,2);

        for(std::list<ImageMatchModel*>::iterator it = models.begin(); it != models.end(); it++)
        {
            kDebug(41006) <<" Error:" << (*it)->fittingErrorSum() <<" with" << (*it)->matches().size();
        }
#if 0
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
        double r = Optimization::Algorithms::levenbergMarquardt(&f, p, 100, 1e-12, 0.01, 10.0);
        kDebug(41006) <<"Remain =" << r;
        for(uint i = 0; i < p.size(); i++)
        {
            kDebug(41006) <<"p["<< i <<"]=" << p[i];
        }

        for(uint i = 0; i < p.size(); i++)
        {
            kDebug(41006) <<"p["<< i <<"]=" << p[i];
        }
#endif
        PanoptimFunction<DoubleHomographySameDistortionFunction, DoubleHomographySameDistortionFunction::SIZEINDEXES> f( (*models.begin())->matches(), width * 0.5, height * 0.5, width, height );
        p[DoubleHomographySameDistortionFunction::INDX_a] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_b] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_c] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_h11_1] = 1.0;
        p[DoubleHomographySameDistortionFunction::INDX_h21_1] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_h31_1] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_h12_1] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_h22_1] = 1.0;
        p[DoubleHomographySameDistortionFunction::INDX_h32_1] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_h13_1] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_h23_1] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_h11_2] = 1.0; // transfo(0,0);
        p[DoubleHomographySameDistortionFunction::INDX_h21_2] = 0.0; //transfo(0,1);
        p[DoubleHomographySameDistortionFunction::INDX_h31_2] = transfo(0,2);
        p[DoubleHomographySameDistortionFunction::INDX_h12_2] = 0.0; //transfo(1,0);
        p[DoubleHomographySameDistortionFunction::INDX_h22_2] = 1.0; // transfo(1,1);
        p[DoubleHomographySameDistortionFunction::INDX_h32_2] = transfo(1,2);
        p[DoubleHomographySameDistortionFunction::INDX_h13_2] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_h23_2] = 0.0;
        double r = Optimization::Algorithms::levenbergMarquardt(&f, p, 300, 1e-12, 0.01, 10.0 );
        kDebug(41006) <<"Remain =" << r;
        for(uint i = 0; i < p.size(); i++)
        {
            kDebug(41006) <<"p["<< i <<"]=" << p[i];
        }
    } else {
        kDebug(41006) <<"No models found";
        return std::vector<Result>();
    }
#if 0
    result[0].a = p[HomographySameDistortionFunction::INDX_a];
    result[0].b = p[HomographySameDistortionFunction::INDX_b];
    result[0].c = p[HomographySameDistortionFunction::INDX_c];
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            result[0].homography(i,j) = 0.0;
        }
        result[0].homography(i,i) = 1.0;
    }
    
    
    result[1].a = p[HomographySameDistortionFunction::INDX_a];
    result[1].b = p[HomographySameDistortionFunction::INDX_b];
    result[1].c = p[HomographySameDistortionFunction::INDX_c];
    result[1].homography(0,0) = p[HomographySameDistortionFunction::INDX_h11];
    result[1].homography(0,1) = p[HomographySameDistortionFunction::INDX_h21];
    result[1].homography(0,2) = p[HomographySameDistortionFunction::INDX_h31];
    result[1].homography(1,0) = p[HomographySameDistortionFunction::INDX_h12];
    result[1].homography(1,1) = p[HomographySameDistortionFunction::INDX_h22];
    result[1].homography(1,2) = p[HomographySameDistortionFunction::INDX_h32];
    result[1].homography(2,0) = p[HomographySameDistortionFunction::INDX_h13];
    result[1].homography(2,1) = p[HomographySameDistortionFunction::INDX_h23];
    result[1].homography(2,2) = 1.0;
#endif
    result[0].a = p[DoubleHomographySameDistortionFunction::INDX_a];
    result[0].b = p[DoubleHomographySameDistortionFunction::INDX_b];
    result[0].c = p[DoubleHomographySameDistortionFunction::INDX_c];
    result[0].homography(0,0) = p[DoubleHomographySameDistortionFunction::INDX_h11_1];
    result[0].homography(0,1) = p[DoubleHomographySameDistortionFunction::INDX_h21_1];
    result[0].homography(0,2) = p[DoubleHomographySameDistortionFunction::INDX_h31_1];
    result[0].homography(1,0) = p[DoubleHomographySameDistortionFunction::INDX_h12_1];
    result[0].homography(1,1) = p[DoubleHomographySameDistortionFunction::INDX_h22_1];
    result[0].homography(1,2) = p[DoubleHomographySameDistortionFunction::INDX_h32_1];
    result[0].homography(2,0) = p[DoubleHomographySameDistortionFunction::INDX_h13_1];
    result[0].homography(2,1) = p[DoubleHomographySameDistortionFunction::INDX_h23_1];
    result[0].homography(2,2) = 1.0;
    
    result[1].a = p[DoubleHomographySameDistortionFunction::INDX_a];
    result[1].b = p[DoubleHomographySameDistortionFunction::INDX_b];
    result[1].c = p[DoubleHomographySameDistortionFunction::INDX_c];
    result[1].homography(0,0) = p[DoubleHomographySameDistortionFunction::INDX_h11_2];
    result[1].homography(0,1) = p[DoubleHomographySameDistortionFunction::INDX_h21_2];
    result[1].homography(0,2) = p[DoubleHomographySameDistortionFunction::INDX_h31_2];
    result[1].homography(1,0) = p[DoubleHomographySameDistortionFunction::INDX_h12_2];
    result[1].homography(1,1) = p[DoubleHomographySameDistortionFunction::INDX_h22_2];
    result[1].homography(1,2) = p[DoubleHomographySameDistortionFunction::INDX_h32_2];
    result[1].homography(2,0) = p[DoubleHomographySameDistortionFunction::INDX_h13_2];
    result[1].homography(2,1) = p[DoubleHomographySameDistortionFunction::INDX_h23_2];
    result[1].homography(2,2) = 1.0;
    return result;
}
