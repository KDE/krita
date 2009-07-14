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

#include "models/kis_image_alignment_projective_model.h"

#include "kis_image_alignment_model_p.h"
#include "kis_matched_point.h"
#include "imagoptim_p.h"
#include "imagoptim_functions.h"

KisImageAlignmentProjectiveModel::~KisImageAlignmentProjectiveModel()
{
}

KisImageAlignmentModel::OptimizationFunction* KisImageAlignmentProjectiveModel::createOptimizationFunction(const KisControlPoints& controlPoints, double xc, double yc, int width, int height) const
{

    return new PanoptimFunction<DoubleHomographySameDistortionFunction>(controlPoints, xc, yc, width, height);
}

