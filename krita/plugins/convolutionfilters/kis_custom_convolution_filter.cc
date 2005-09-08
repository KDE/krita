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

#include <qspinbox.h>

#include "kis_custom_convolution_filter_configuration_widget.h"
#include "kis_custom_convolution_filter_configuration_base_widget.h"
#include "kis_matrix_widget.h"


KisCustomConvolutionFilter::KisCustomConvolutionFilter() : KisConvolutionFilter(id(), "enhance", "&Custom Convolution...")
{

}
KisFilterConfigWidget * KisCustomConvolutionFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceImplSP)
{
    KisCustomConvolutionFilterConfigurationWidget* ccfcw = new KisCustomConvolutionFilterConfigurationWidget(this,parent, "custom convolution config widget");
    Q_CHECK_PTR(ccfcw);
    return ccfcw;
}

KisFilterConfiguration * KisCustomConvolutionFilter::configuration(QWidget* nwidget, KisPaintDeviceImplSP dev)
{
    KisCustomConvolutionFilterConfigurationWidget* widget = (KisCustomConvolutionFilterConfigurationWidget*) nwidget;
    Q_INT32 imgdepth = dev->colorSpace()->nChannels();
    if ( widget == 0 )
    {
        // Create the identity matrices:
        KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
        Q_CHECK_PTR(amatrixes);

        int mat[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
        for(int i = 0; i < imgdepth - 1; i ++)
        {
            amatrixes[i] = KisMatrix3x3(mat, 1, 127);
        }
         int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
        amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
    
        return new KisCustomConvolutionConfiguration( amatrixes );
    } else {

        KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
        Q_CHECK_PTR(amatrixes);

        KisCustomConvolutionFilterConfigurationBaseWidget* mw = widget->matrixWidget();
        for(int i = 0; i < imgdepth - 1; i ++)
        {
            amatrixes[i][0][0] = mw->matrixWidget->m11->value();
            amatrixes[i][1][0] = mw->matrixWidget->m21->value();
            amatrixes[i][2][0] = mw->matrixWidget->m31->value();
            amatrixes[i][0][1] = mw->matrixWidget->m12->value();
            amatrixes[i][1][1] = mw->matrixWidget->m22->value();
            amatrixes[i][2][1] = mw->matrixWidget->m32->value();
            amatrixes[i][0][2] = mw->matrixWidget->m13->value();
            amatrixes[i][1][2] = mw->matrixWidget->m23->value();
            amatrixes[i][2][2] = mw->matrixWidget->m33->value();
            amatrixes[i].setFactor( mw->spinBoxFactor->value() );
            amatrixes[i].setOffset( mw->spinBoxOffset->value() );
        }
        
        // XXX make this configurable?
        int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
        amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
        
        return new KisCustomConvolutionConfiguration( amatrixes );
    }
}
