/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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

#ifndef _KIS_IMAGE_ALIGNMENT_MODEL_P_H_
#define _KIS_IMAGE_ALIGNMENT_MODEL_P_H_

#include <vector>
#include <gmm/gmm_matrix.h>

#include "kis_image_alignment_model.h"

class KisImageAlignmentModel::OptimizationFunction {
  public:
    virtual ~OptimizationFunction();
    virtual std::vector<double> values(const std::vector<double>& parameters) =0;
    virtual gmm::row_matrix< gmm::rsvector<double> > jacobian(const std::vector<double>& parameters) = 0;
    
};

#endif
