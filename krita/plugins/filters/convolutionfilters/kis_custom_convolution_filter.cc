/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 */

#include <kdebug.h>

#include "kis_custom_convolution_filter.h"

#include <QSpinBox>

#include "kis_convolution_painter.h"
#include "kis_custom_convolution_filter_configuration_widget.h"
#include "kis_matrix_widget.h"


KisFilterConfigWidget * KisCustomConvolutionFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP)
{
    KisCustomConvolutionFilterConfigurationWidget* ccfcw = new KisCustomConvolutionFilterConfigurationWidget(this,parent);
    Q_CHECK_PTR(ccfcw);
    ccfcw->setName( "custom convolution config widget");
    return ccfcw;
}


 KisFilterConfiguration * KisCustomConvolutionFilter::configuration()
{

    // Create the identity matrix:
    KisKernelSP kernel = KisKernelSP(new KisKernel());
    kernel->width = 3;
    kernel->height = 3;

    kernel->factor = 1;
    kernel->offset = 127;

    kernel->data = new qint32[9];
    kernel->data[0] = 0;
    kernel->data[1] = 0;
    kernel->data[2] = 0;
    kernel->data[3] = 0;
    kernel->data[4] = 1;
    kernel->data[5] = 0;
    kernel->data[6] = 0;
    kernel->data[7] = 0;
    kernel->data[8] = 0;

    return new KisConvolutionConfiguration( "custom convolution", kernel.data() );
}
