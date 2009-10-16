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


#include <Eigen/Core>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_paint_device.h>

#include "kis_control_point.h"
#include "kis_control_points.h"
#include "harris_detector.h"
#include "matching.h"
#include "ransac.h"
#include "imagoptim_p.h"
#include "imagematchmodel_p.h"
// #include "homographyimagematchmodel_p.h"
#include "kis_image_alignment_model_p.h"

struct KisImageAlignment::Private {
    KisInterestPointsDetector* interestPointsDetector;
    const KisImageAlignmentModel* model;
};

KisImageAlignment::KisImageAlignment(const KisImageAlignmentModel* model, KisInterestPointsDetector* ipd) : d(new Private)
{
    d->interestPointsDetector = ipd;
    d->model = model;
}
KisImageAlignment::~KisImageAlignment()
{
    delete d;
}

Eigen::Matrix3d recComputeTransfo(const Eigen::Matrix3d& transfoToFrame, KisImageAlignment::ImageInfo* referenceFrame)
{
    if (referenceFrame == 0) {
        return transfoToFrame;
    } else {
        /*        Eigen::Matrix3d m = recComputeTransfo( referenceFrame->transfoToFrame, referenceFrame->referenceFrame ) + transfoToFrame;
                m(2,2) = 1.0;
                return m;*/
        return recComputeTransfo(referenceFrame->transfoToFrame, referenceFrame->referenceFrame) * transfoToFrame;
    }
}

std::vector<KisImageAlignment::Result> KisImageAlignment::align(QList<ImageInfo> images)
{
//     int frame1start = 0 * DoubleHomographySameDistortionFunction::SIZEHOMOGRAPHYINDEXES + DoubleHomographySameDistortionFunction::SIZEINDEXES;
//     int frame2start = 1 * DoubleHomographySameDistortionFunction::SIZEHOMOGRAPHYINDEXES + DoubleHomographySameDistortionFunction::SIZEINDEXES;
    std::vector<Result> result(images.size());
    std::vector<double> p(images.size() * DoubleHomographySameDistortionFunction::SIZEHOMOGRAPHYINDEXES + DoubleHomographySameDistortionFunction::SIZEINDEXES);
    int width = 1000; // TODO TMP variable
    int height = 1000; // TODO TMP variable
    dbgPlugins << "Creating panorama with" << images.size() << " images";
    dbgPlugins << "Detecting interest points";
    const KoColorSpace* graycs = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
    if (!graycs) {
        dbgPlugins << "Gray 8bit is not installed."; // TODO: message box
        return std::vector<Result>();
    }
    // Interest point detection
    for (QList<KisImageAlignment::ImageInfo>::iterator it = images.begin(); it != images.end(); ++it) {
        dbgPlugins << "Detection on rectangle : " << it->smallRect;
        KisPaintDeviceSP graydevice = new KisPaintDevice(*(it->smallDevice));
        QUndoCommand* cmd = graydevice->convertTo(graycs);
        delete cmd;
        it->points = KisInterestPointsDetector::interestPointDetector()->computeInterestPoints(graydevice, it->smallRect);
        width = it->smallRect.width();
        height = it->smallRect.height();
        prepareGroups(it->points, 20.0);
#if 0
        for (lInterestPoints::const_iterator itp = it->points.begin(); itp != it->points.end(); ++itp) {
            dbgPlugins << "ip =" << (*itp)->x() << "" << (*itp)->y() /* <<"" << (*itp)->toString() */;
        }
#endif
    }
    // Matching
    KisControlPoints controlPoints(images.size());
    ImageMatchModel::Params* ransacparams = new ImageMatchModel::Params;
    ransacparams->threshold = 20.0;
    Ransac<ImageMatchModel, ImageMatchModel::Params, KisMatch> ransac(10, 100, ransacparams);
    int frameNbRef = 0;
    for (QList<KisImageAlignment::ImageInfo>::iterator it = images.begin(); it != images.end(); ++it, ++frameNbRef) {
        QList<KisImageAlignment::ImageInfo>::iterator it2 = it;
        ++it2;
        int frameNbMatch = frameNbRef + 1;
        for (; it2 != images.end(); ++it2, ++frameNbMatch) {
            lMatches mp = matching(it->points, it2->points);
#if 1
            for (lMatches::const_iterator itp = mp.begin(); itp != mp.end(); ++itp) {
                dbgPlugins << "match = " << itp->ref->x() << " " << itp->ref->y() << " " << itp->match->x() << " " << itp->match->y();
            }
#endif
            std::list<ImageMatchModel*> models = ransac.findModels(mp);
            if (models.empty()) {
                dbgPlugins << "No model found";
            } else {
                dbgPlugins << "Best model:" << (*models.begin())->fittingErrorSum() << " with" << (*models.begin())->matches().size() << " points";
                it2->transfoToFrame = (*models.begin())->transfo();
                it2->referenceFrame = &(*it);
                controlPoints.addMatches((*models.begin())->matches(), frameNbRef, frameNbMatch);
            }
        }
    }
    if (true) {
        KisImageAlignmentModel::OptimizationFunction* f = d->model->createOptimizationFunction(controlPoints, width * 0.5, height * 0.5, width, height);
        p[DoubleHomographySameDistortionFunction::INDX_a] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_b] = 0.0;
        p[DoubleHomographySameDistortionFunction::INDX_c] = 0.0;
        for (int i = 0; i < images.size(); i++) {
            Eigen::Matrix3d transfo = recComputeTransfo(images[i].transfoToFrame,  images[i].referenceFrame);
            Eigen::Matrix3d transfo2ref = images[i].transfoToFrame;
            dbgPlugins << i << " " << &(images[i]) << " Translation : " << transfo(0, 2) << " " << transfo(1, 2) << " Reference translation : " << transfo2ref(0, 2) << " " << transfo2ref(1, 2);
            int frameStart = i * DoubleHomographySameDistortionFunction::SIZEHOMOGRAPHYINDEXES + DoubleHomographySameDistortionFunction::SIZEINDEXES;
            p[DoubleHomographySameDistortionFunction::INDX_h11 + frameStart] = transfo(0, 0);
            p[DoubleHomographySameDistortionFunction::INDX_h21 + frameStart] = transfo(0, 1);
            p[DoubleHomographySameDistortionFunction::INDX_h31 + frameStart] = transfo(0, 2);
            p[DoubleHomographySameDistortionFunction::INDX_h12 + frameStart] = transfo(1, 0);
            p[DoubleHomographySameDistortionFunction::INDX_h22 + frameStart] = transfo(1, 1);
            p[DoubleHomographySameDistortionFunction::INDX_h32 + frameStart] = transfo(1, 2);
            p[DoubleHomographySameDistortionFunction::INDX_h13 + frameStart] = 0.0;
            p[DoubleHomographySameDistortionFunction::INDX_h23 + frameStart] = 0.0;
        }
        double r;
        {
            r = Optimization::Algorithms::levenbergMarquardt(f, p, 300, 1e-12, 0.01, 10.0);
            dbgPlugins << "Remain =" << r;
            for (uint i = 0; i < p.size(); i++) {
                dbgPlugins << "p[" << i << "]=" << p[i];
            }
            std::vector<double> remains = f->values(p);
            std::cout << "Remains = " << remains << std::endl;
        }
#if 1
        // Remove outliers and reoptimize
        {
            f->removeOutlier(p, r * 1.1);
            r = Optimization::Algorithms::levenbergMarquardt(f, p, 300, 1e-12, 0.01, 10.0);
            dbgPlugins << "Remain =" << r;
            for (uint i = 0; i < p.size(); i++) {
                dbgPlugins << "p[" << i << "]=" << p[i];
            }
            std::vector<double> remains = f->values(p);
            std::cout << "Remains = " << remains << std::endl;
        }
#endif
    } else {
        dbgPlugins << "No models found";
        return std::vector<Result>();
    }

    // Scale the results of the first optimization
    for (int i = 0; i < images.size(); i++) {
        int frameStart = i * DoubleHomographySameDistortionFunction::SIZEHOMOGRAPHYINDEXES + DoubleHomographySameDistortionFunction::SIZEINDEXES;

        Eigen::Matrix3d homography;
        homography(0, 0) = p[DoubleHomographySameDistortionFunction::INDX_h11 + frameStart];
        homography(0, 1) = p[DoubleHomographySameDistortionFunction::INDX_h21 + frameStart];
        homography(0, 2) = p[DoubleHomographySameDistortionFunction::INDX_h31 + frameStart];
        homography(1, 0) = p[DoubleHomographySameDistortionFunction::INDX_h12 + frameStart];
        homography(1, 1) = p[DoubleHomographySameDistortionFunction::INDX_h22 + frameStart];
        homography(1, 2) = p[DoubleHomographySameDistortionFunction::INDX_h32 + frameStart];
        homography(2, 0) = p[DoubleHomographySameDistortionFunction::INDX_h13 + frameStart];
        homography(2, 1) = p[DoubleHomographySameDistortionFunction::INDX_h23 + frameStart];
        homography(2, 2) = 1.0;
        std::cout << " homography [ " << i << " ] = " << homography << std::endl;

        double scale = images[i].bigRect.width() / (double)images[i].smallRect.width();
//         p[DoubleHomographySameDistortionFunction::INDX_h11 + frameStart] /= scale;
//         p[DoubleHomographySameDistortionFunction::INDX_h21 + frameStart] /= scale;
        p[DoubleHomographySameDistortionFunction::INDX_h31 + frameStart] *= scale;
//         p[DoubleHomographySameDistortionFunction::INDX_h12 + frameStart] /= scale;
//         p[DoubleHomographySameDistortionFunction::INDX_h22 + frameStart] /= scale;
        p[DoubleHomographySameDistortionFunction::INDX_h32 + frameStart] *= scale;
        p[DoubleHomographySameDistortionFunction::INDX_h13 + frameStart] /= scale;
        p[DoubleHomographySameDistortionFunction::INDX_h23 + frameStart] /= scale;
    }
#if 1
    p[DoubleHomographySameDistortionFunction::INDX_a] = 0.0;
    p[DoubleHomographySameDistortionFunction::INDX_b] = 0.0;
    p[DoubleHomographySameDistortionFunction::INDX_c] = 0.0;
#endif

    // Fill the result and the homographie
    for (int i = 0; i < images.size(); i++) {
        int frameStart = i * DoubleHomographySameDistortionFunction::SIZEHOMOGRAPHYINDEXES + DoubleHomographySameDistortionFunction::SIZEINDEXES;
        result[i].a = p[DoubleHomographySameDistortionFunction::INDX_a];
        result[i].b = p[DoubleHomographySameDistortionFunction::INDX_b];
        result[i].c = p[DoubleHomographySameDistortionFunction::INDX_c];
        result[i].homography(0, 0) = p[DoubleHomographySameDistortionFunction::INDX_h11 + frameStart];
        result[i].homography(0, 1) = p[DoubleHomographySameDistortionFunction::INDX_h21 + frameStart];
        result[i].homography(0, 2) = p[DoubleHomographySameDistortionFunction::INDX_h31 + frameStart];
        result[i].homography(1, 0) = p[DoubleHomographySameDistortionFunction::INDX_h12 + frameStart];
        result[i].homography(1, 1) = p[DoubleHomographySameDistortionFunction::INDX_h22 + frameStart];
        result[i].homography(1, 2) = p[DoubleHomographySameDistortionFunction::INDX_h32 + frameStart];
        result[i].homography(2, 0) = p[DoubleHomographySameDistortionFunction::INDX_h13 + frameStart];
        result[i].homography(2, 1) = p[DoubleHomographySameDistortionFunction::INDX_h23 + frameStart];
        result[i].homography(2, 2) = 1.0;
        std::cout << " homography [ " << i << " ] = " << result[i].homography << std::endl;
    }
    return result;
}
