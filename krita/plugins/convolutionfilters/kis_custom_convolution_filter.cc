/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kis_custom_convolution_filter.h"

#include <qspinbox.h>

#include "kis_custom_convolution_filter_configuration_widget.h"
#include "kis_custom_convolution_filter_configuration_base_widget.h"
#include "kis_matrix_widget.h"


KisCustomConvolutionFilter::KisCustomConvolutionFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
	
}
KisFilterConfigurationWidget* KisCustomConvolutionFilter::createConfigurationWidget(QWidget* parent)
{
	KisCustomConvolutionFilterConfigurationWidget* ccfcw = new KisCustomConvolutionFilterConfigurationWidget(this,parent, "");
	kdDebug() << ccfcw << endl;
	return ccfcw;
}

KisFilterConfiguration* KisCustomConvolutionFilter::configuration(KisFilterConfigurationWidget* nwidget)
{
	KisCustomConvolutionFilterConfigurationWidget* widget = (KisCustomConvolutionFilterConfigurationWidget*) nwidget;
	Q_INT32 imgdepth = colorStrategy()->depth();
	if( widget == 0 )
	{
		// Create an identity matrixes :
		KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
		int mat[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
		for(int i = 0; i < imgdepth - 1; i ++)
		{
			amatrixes[i] = KisMatrix3x3(mat, 1, 127);
		}
		int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
		amatrixes[imgdepth - 1] = KisMatrix3x3(mat, 1, 0);
		return new KisCustomConvolutionConfiguration( amatrixes );
	} else {
		KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
		for(int i = 0; i < imgdepth; i ++)
		{
			KisCustomConvolutionFilterConfigurationBaseWidget* mw = widget->matrixWidget(i);
			int pos = widget->pos(i);
			amatrixes[pos][0][0] = mw->matrixWidget->m11->value();
			amatrixes[pos][1][0] = mw->matrixWidget->m21->value();
			amatrixes[pos][2][0] = mw->matrixWidget->m31->value();
			amatrixes[pos][0][1] = mw->matrixWidget->m12->value();
			amatrixes[pos][1][1] = mw->matrixWidget->m22->value();
			amatrixes[pos][2][1] = mw->matrixWidget->m32->value();
			amatrixes[pos][0][2] = mw->matrixWidget->m13->value();
			amatrixes[pos][1][2] = mw->matrixWidget->m23->value();
			amatrixes[pos][2][2] = mw->matrixWidget->m33->value();
			amatrixes[pos].setFactor( mw->spinBoxFactor->value() );
			amatrixes[pos].setOffset( mw->spinBoxOffset->value() );
		}
		return new KisCustomConvolutionConfiguration( amatrixes );
	}
}
