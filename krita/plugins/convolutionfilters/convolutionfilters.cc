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
#include <kis_view.h>
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

 	kdDebug() << "ConvolutionFilters plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";
	KisView * view;

	if ( !parent->inherits("KisView") )
	{
		return;
	} else {
		view = (KisView*) parent;
	}



	KisFilterSP kgbf = createFilter<KisGaussianBlurFilter>(view);
	(void) new KAction(i18n("&Gaussian Blur"), 0, 0, kgbf, SLOT(slotActivated()), actionCollection(), "convolution_blur");

	KisFilterSP kgsf = createFilter<KisSharpenFilter>(view);
	(void) new KAction(i18n("&Sharpen"), 0, 0, kgsf, SLOT(slotActivated()), actionCollection(), "convolution_sharpen");

	KisFilterSP kmrf = createFilter<KisMeanRemovalFilter>(view);
	(void) new KAction(i18n("&Mean Removal"), 0, 0, kmrf, SLOT(slotActivated()), actionCollection(), "convolution_meanremoval");

	KisFilterSP kelf = createFilter<KisEmbossLaplascianFilter>(view);
	(void) new KAction("Emboss Laplascian", 0, 0, kelf, SLOT(slotActivated()), actionCollection(), "convolution_embosslaplascian");

	KisFilterSP keiadf = createFilter<KisEmbossInAllDirectionsFilter>(view);
	(void) new KAction("Emboss in All Directions", 0, 0, keiadf, SLOT(slotActivated()), actionCollection(), "convolution_embossalldirections");

	KisFilterSP kehvf = createFilter<KisEmbossHorizontalVerticalFilter>(view);
	(void) new KAction("Emboss Horizontal && Vertical", 0, 0, kehvf, SLOT(slotActivated()), actionCollection(), "convolution_embosshorzvertical");

	KisFilterSP kevf = createFilter<KisEmbossVerticalFilter>(view);
	(void) new KAction("Emboss Vertical Only", 0, 0, kevf, SLOT(slotActivated()), actionCollection(), "convolution_embossverticalonly");

	KisFilterSP kehf = createFilter<KisEmbossHorizontalFilter>(view);
	(void) new KAction("Emboss Horizontal Only", 0, 0, kehf, SLOT(slotActivated()), actionCollection(), "convolution_embosshorizontalonly");

	KisFilterSP kedf = createFilter<KisEmbossDiagonalFilter>(view);
 	(void) new KAction("Emboss in Diagonal", 0, 0, kedf, SLOT(slotActivated()), actionCollection(), "convolution_embossdiagonal");

	KisFilterSP ktedf = createFilter<KisTopEdgeDetectionFilter>(view);
	(void) new KAction("Top Edge Detection", 0, 0, ktedf, SLOT(slotActivated()), actionCollection(), "convolution_edgedetectiontop");

	KisFilterSP kredf = createFilter<KisRightEdgeDetectionFilter>(view);
	(void) new KAction("Right Edge Detection", 0, 0, kredf, SLOT(slotActivated()), actionCollection(), "convolution_edgedetectionright");

	KisFilterSP kbedf = createFilter<KisBottomEdgeDetectionFilter>(view);
	(void) new KAction("Bottom Edge Detection", 0, 0, kbedf, SLOT(slotActivated()), actionCollection(), "convolution_edgedetectionbottom");

	KisFilterSP kledf = createFilter<KisLeftEdgeDetectionFilter>(view);
	(void) new KAction("Left Edge Detection", 0, 0, kledf, SLOT(slotActivated()), actionCollection(), "convolution_edgedetectionleft");

// XXX: This filter crashes Krita
#if 0
	KisFilterSP kccf = createFilter<KisCustomConvolutionFilter>(view);
	(void) new KAction("Custom Convolution...", 0, 0, kccf, SLOT(slotActivated()), actionCollection(), "convolution_custom");
#endif
}

KritaConvolutionFilters::~KritaConvolutionFilters()
{
}

KisGaussianBlurFilter::KisGaussianBlurFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;
	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 1, 2, 1 }, { 2, 4, 2 }, { 1, 2, 1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 16, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}


KisSharpenFilter::KisSharpenFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() +1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, -2, 0 }, { -2, 11, -2 }, { 0, -2, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 3, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}

KisMeanRemovalFilter::KisMeanRemovalFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, -1, -1 }, { -1, 9, -1 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}

KisEmbossLaplascianFilter::KisEmbossLaplascianFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, 0, -1 }, { 0, 4, 0 }, { -1, 0, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}

KisEmbossInAllDirectionsFilter::KisEmbossInAllDirectionsFilter(KisView * view)
	: KisConvolutionConstFilter(name(), view)
{
	if(!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, -1, -1 }, { -1, 8, -1 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}

KisEmbossHorizontalVerticalFilter::KisEmbossHorizontalVerticalFilter(KisView * view)
	: KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, -1, 0 }, { -1, 4, -1 }, { 0, -1, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}

KisEmbossVerticalFilter::KisEmbossVerticalFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, -1, 0 }, { 0, 2, 0 }, { 0, -1, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}

KisEmbossHorizontalFilter::KisEmbossHorizontalFilter(KisView * view) :
	KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, 0, 0 }, { -1, 4, -1 }, { 0, 0, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);

}

KisEmbossDiagonalFilter::KisEmbossDiagonalFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, 0, -1 }, { 0, 4, 0 }, { -1, 0, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}


KisTopEdgeDetectionFilter::KisTopEdgeDetectionFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 1, 1, 1 }, { 0, 0, 0 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);

}

KisRightEdgeDetectionFilter::KisRightEdgeDetectionFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, 0, 1 }, { -1, 0, 1 }, { -1, 0, 1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}

KisBottomEdgeDetectionFilter::KisBottomEdgeDetectionFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, -1, -1 }, { 0, 0, 0 }, { 1, 1, 1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}

KisLeftEdgeDetectionFilter::KisLeftEdgeDetectionFilter(KisView * view) : KisConvolutionConstFilter(name(), view)
{
	if (!colorStrategy()) return;

	Q_INT32 imgdepth = colorStrategy()->nColorChannels() + 1;
	m_matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 1, 0, -1 }, { 1, 0, -1 }, { 1, 0, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		m_matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	m_matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
}
