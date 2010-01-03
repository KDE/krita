/*
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */


#ifndef _KIS_IMAGES_BLENDER_H_
#define _KIS_IMAGES_BLENDER_H_

#include "kis_types.h"
#include <Eigen/Core>
#include <QRect>
#include <QRegion>
#include <QList>

class KisRandomSubAccessorPixel;
class KisImagesBlender
{
public:
    struct LayerSource {
        KisPaintDeviceSP layer;
        double a, b, c; ///< distortion parameters
        double xc1, yc1, xc2, yc2;
        double norm;
        Eigen::Matrix3d homography; ///< homography parameters
        Eigen::Matrix3d invHomography; ///< homography parameters
        QRect rect; ///< size of the layer
        QRegion boundingBox; ///<contain the bounding box of the image (will be computed by the blend algorithm)
        KisRandomSubAccessorPixel* accessor;
    };
    static void blend(QList<LayerSource> sources, KisPaintDeviceSP dst);
private:
    static inline double poly(double a, double b, double c, double u) {
        return 1.0 + a*u + b*u*u + c*u*u*u;
    }
};

#endif
