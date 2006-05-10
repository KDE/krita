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
    KisCustomConvolutionFilterConfigurationWidget* ccfcw = new KisCustomConvolutionFilterConfigurationWidget(this,parent, "custom convolution config widget");
    Q_CHECK_PTR(ccfcw);
    return ccfcw;
}

KisFilterConfiguration * KisCustomConvolutionFilter::configuration(QWidget* nwidget)
{
    KisCustomConvolutionFilterConfigurationWidget* widget = (KisCustomConvolutionFilterConfigurationWidget*) nwidget;

    if ( widget == 0 )
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

    } else {

        // Create the identity matrices:
        KisKernelSP kernel = KisKernelSP(new KisKernel());
        kernel->width = 3;
        kernel->height = 3;

        kernel->data = new qint32[9];

        KisCustomConvolutionFilterConfigurationBaseWidget* mw = widget->matrixWidget();

        kernel->data[0] = mw->matrixWidget->m11->value();
        kernel->data[1] = mw->matrixWidget->m21->value();
        kernel->data[2] = mw->matrixWidget->m31->value();
        kernel->data[3] = mw->matrixWidget->m12->value();
        kernel->data[4] = mw->matrixWidget->m22->value();
        kernel->data[5] = mw->matrixWidget->m32->value();
        kernel->data[6] = mw->matrixWidget->m13->value();
        kernel->data[7] = mw->matrixWidget->m23->value();
        kernel->data[8] = mw->matrixWidget->m33->value();

        kernel->factor = mw->spinBoxFactor->value();
        kernel->offset = mw->spinBoxOffset->value();

        return new KisConvolutionConfiguration( "custom convolution",  kernel.data() );
    }
}
