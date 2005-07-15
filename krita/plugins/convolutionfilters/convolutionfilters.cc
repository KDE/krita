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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_painter.h>

// #include <kmessagebox.h>

#include "convolutionfilters.h"
#include "kis_custom_convolution_filter.h"

typedef KGenericFactory<KritaConvolutionFilters> KritaConvolutionFiltersFactory;
K_EXPORT_COMPONENT_FACTORY( kritaconvolutionfilters, KritaConvolutionFiltersFactory( "krita" ) )

	KritaConvolutionFilters::KritaConvolutionFilters(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
       	setInstance(KritaConvolutionFiltersFactory::instance());

 	kdDebug(DBG_AREA_PLUGINS) << "ConvolutionFilters plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";

	if ( parent->inherits("KisFactory") )
	{
		KisFilterRegistry::instance()->add(new KisGaussianBlurFilter());
		KisFilterRegistry::instance()->add(new KisSharpenFilter());
		KisFilterRegistry::instance()->add(new KisMeanRemovalFilter());
		KisFilterRegistry::instance()->add(new KisEmbossLaplascianFilter());
		KisFilterRegistry::instance()->add(new KisEmbossInAllDirectionsFilter());
		KisFilterRegistry::instance()->add(new KisEmbossHorizontalVerticalFilter());
		KisFilterRegistry::instance()->add(new KisEmbossVerticalFilter());
		KisFilterRegistry::instance()->add(new KisEmbossHorizontalFilter());
		KisFilterRegistry::instance()->add(new KisTopEdgeDetectionFilter());
		KisFilterRegistry::instance()->add(new KisRightEdgeDetectionFilter());
		KisFilterRegistry::instance()->add(new KisBottomEdgeDetectionFilter());
		KisFilterRegistry::instance()->add(new KisLeftEdgeDetectionFilter());
		KisFilterRegistry::instance()->add(new KisCustomConvolutionFilter());
	}
}

KritaConvolutionFilters::~KritaConvolutionFilters()
{
}

KisGaussianBlurFilter::KisGaussianBlurFilter() 
	: KisConvolutionConstFilter(id(), "blur", i18n("&Gaussian Blur"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);
	int mat[3][3] =  { { 1, 2, 1 }, { 2, 4, 2 }, { 1, 2, 1} };
	m_matrixes[0] = KisMatrix3x3(mat, 16, 0);
}


KisSharpenFilter::KisSharpenFilter() 
	: KisConvolutionConstFilter(id(), "enhance", i18n("&Sharpen"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { 0, -2, 0 }, { -2, 11, -2 }, { 0, -2, 0} };
	m_matrixes[0] = KisMatrix3x3(mat, 3, 0);
}

KisMeanRemovalFilter::KisMeanRemovalFilter() 
	: KisConvolutionConstFilter(id(), "enhance", i18n("&Mean Removal"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { -1, -1, -1 }, { -1, 9, -1 }, { -1, -1, -1} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 0);
}

KisEmbossLaplascianFilter::KisEmbossLaplascianFilter() 
	: KisConvolutionConstFilter(id(), "emboss", i18n("Emboss Laplascian"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { -1, 0, -1 }, { 0, 4, 0 }, { -1, 0, -1} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);
}

KisEmbossInAllDirectionsFilter::KisEmbossInAllDirectionsFilter()
	: KisConvolutionConstFilter(id(), "emboss", i18n("Emboss in All Directions"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { -1, -1, -1 }, { -1, 8, -1 }, { -1, -1, -1} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);
}

KisEmbossHorizontalVerticalFilter::KisEmbossHorizontalVerticalFilter()
	: KisConvolutionConstFilter(id(), "emboss", i18n("Emboss Horizontal && Vertical"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { 0, -1, 0 }, { -1, 4, -1 }, { 0, -1, 0} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);
}

KisEmbossVerticalFilter::KisEmbossVerticalFilter() 
	: KisConvolutionConstFilter(id(), "emboss", i18n("Emboss Vertical Only"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { 0, -1, 0 }, { 0, 2, 0 }, { 0, -1, 0} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);
}

KisEmbossHorizontalFilter::KisEmbossHorizontalFilter() :
	KisConvolutionConstFilter(id(), "emboss", i18n("Emboss Horizontal Only"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { 0, 0, 0 }, { -1, 4, -1 }, { 0, 0, 0} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);

}

KisEmbossDiagonalFilter::KisEmbossDiagonalFilter() 
	: KisConvolutionConstFilter(id(), "edge", i18n("Top Edge Detection"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { -1, 0, -1 }, { 0, 4, 0 }, { -1, 0, -1} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);
}


KisTopEdgeDetectionFilter::KisTopEdgeDetectionFilter() 
	: KisConvolutionConstFilter(id(), "edge", i18n("Top Edge Detection"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { 1, 1, 1 }, { 0, 0, 0 }, { -1, -1, -1} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);

}

KisRightEdgeDetectionFilter::KisRightEdgeDetectionFilter()
	: KisConvolutionConstFilter(id(), "edge", i18n("Right Edge Detection"))
{
	m_matrixes = new KisMatrix3x3[1];
	int mat[3][3] =  { { -1, 0, 1 }, { -1, 0, 1 }, { -1, 0, 1} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);
}

KisBottomEdgeDetectionFilter::KisBottomEdgeDetectionFilter() : KisConvolutionConstFilter(id(), "edge", i18n("Bottom Edge Detection"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { -1, -1, -1 }, { 0, 0, 0 }, { 1, 1, 1} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);
}

KisLeftEdgeDetectionFilter::KisLeftEdgeDetectionFilter() : KisConvolutionConstFilter(id(), "edge", i18n("Left Edge Detection"))
{
	m_matrixes = new KisMatrix3x3[1];
	Q_CHECK_PTR(m_matrixes);

	int mat[3][3] =  { { 1, 0, -1 }, { 1, 0, -1 }, { 1, 0, -1} };
	m_matrixes[0] = KisMatrix3x3(mat, 1, 127);
}
