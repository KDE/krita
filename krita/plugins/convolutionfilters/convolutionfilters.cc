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
#include <kis_iterators_quantum.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_painter.h>

// #include <kmessagebox.h>

#include "convolutionfilters.h"

typedef KGenericFactory<KritaConvolutionFilters> KritaConvolutionFiltersFactory;
K_EXPORT_COMPONENT_FACTORY( kritaconvolutionfilters, KritaConvolutionFiltersFactory( "krita" ) )

	KritaConvolutionFilters::KritaConvolutionFilters(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
       	setInstance(KritaConvolutionFiltersFactory::instance());

// 	kdDebug() << "ConvolutionFilters plugin. Class: " 
// 		  << className() 
// 		  << ", Parent: " 
// 		  << parent -> className()
// 		  << "\n";
	KisView * view;
	
	if ( !parent->inherits("KisView") )
	{
		return;
	} else {
		view = (KisView*) parent;
	}


	
	KisFilterSP kgbf = createFilter<KisGaussianBlurFilter>(view);
	(void) new KAction(i18n("&Gaussian blur"), 0, 0, kgbf, SLOT(slotActivated()), actionCollection(), "convolution_blur");

	KisFilterSP kgsf = createFilter<KisSharpenFilter>(view);
	(void) new KAction(i18n("&Sharpen"), 0, 0, kgsf, SLOT(slotActivated()), actionCollection(), "convolution_sharpen");

	KisFilterSP kmrf = createFilter<KisMeanRemovalFilter>(view);
	(void) new KAction(i18n("&Mean removal"), 0, 0, kmrf, SLOT(slotActivated()), actionCollection(), "convolution_meanremoval");

	KisFilterSP kelf = createFilter<KisEmbossLaplascianFilter>(view);
	(void) new KAction("Emboss laplascian", 0, 0, kelf, SLOT(slotActivated()), actionCollection(), "convolution_embosslaplascian");

	KisFilterSP keiadf = createFilter<KisEmbossInAllDirectionsFilter>(view);
	(void) new KAction("Emboss in all direction", 0, 0, keiadf, SLOT(slotActivated()), actionCollection(), "convolution_embossalldirections");
	
	KisFilterSP kehvf = createFilter<KisEmbossHorizontalVerticalFilter>(view);
	(void) new KAction("Emboss horizontal and vertical", 0, 0, kehvf, SLOT(slotActivated()), actionCollection(), "convolution_embosshorzvertical");
	
	KisFilterSP kevf = createFilter<KisEmbossVerticalFilter>(view);
	(void) new KAction("Emboss vertical only", 0, 0, kevf, SLOT(slotActivated()), actionCollection(), "convolution_embossverticalonly");
	
	KisFilterSP kehf = createFilter<KisEmbossHorizontalFilter>(view);
	(void) new KAction("Emboss horizontal only", 0, 0, kehf, SLOT(slotActivated()), actionCollection(), "convolution_embosshorizontalonly");
	
	KisFilterSP kedf = createFilter<KisEmbossDiagonalFilter>(view);
 	(void) new KAction("Emboss in diagonal", 0, 0, kedf, SLOT(slotActivated()), actionCollection(), "convolution_embossdiagonal");
	
	KisFilterSP ktedf = createFilter<KisTopEdgeDetectionFilter>(view);
	(void) new KAction("Top Edge detection", 0, 0, ktedf, SLOT(slotActivated()), actionCollection(), "convolution_edgedetectiontop");

	KisFilterSP kredf = createFilter<KisRightEdgeDetectionFilter>(view);
	(void) new KAction("Right Edge detection", 0, 0, kredf, SLOT(slotActivated()), actionCollection(), "convolution_edgedetectionright");
	
	KisFilterSP kbedf = createFilter<KisBottomEdgeDetectionFilter>(view);
	(void) new KAction("Bottom Edge detection", 0, 0, kbedf, SLOT(slotActivated()), actionCollection(), "convolution_edgedetectionbottom");
	
	KisFilterSP kledf = createFilter<KisLeftEdgeDetectionFilter>(view);
	(void) new KAction("Left Edge detection", 0, 0, kledf, SLOT(slotActivated()), actionCollection(), "convolution_edgedetectionleft");

}

KritaConvolutionFilters::~KritaConvolutionFilters()
{
}

KisGaussianBlurFilter::KisGaussianBlurFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisGaussianBlurFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 1, 2, 1 }, { 2, 4, 2 }, { 1, 2, 1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 16, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}


KisSharpenFilter::KisSharpenFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisSharpenFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, -2, 0 }, { -2, 11, -2 }, { 0, -2, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 3, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisMeanRemovalFilter::KisMeanRemovalFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisMeanRemovalFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, -1, -1 }, { -1, 9, -1 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisEmbossLaplascianFilter::KisEmbossLaplascianFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisEmbossLaplascianFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, 0, -1 }, { 0, 4, 0 }, { -1, 0, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisEmbossInAllDirectionsFilter::KisEmbossInAllDirectionsFilter(KisView * view) 
	: KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisEmbossInAllDirectionsFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, -1, -1 }, { -1, 8, -1 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisEmbossHorizontalVerticalFilter::KisEmbossHorizontalVerticalFilter(KisView * view) 
	: KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisEmbossHorizontalVerticalFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, -1, 0 }, { -1, 4, -1 }, { 0, -1, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisEmbossVerticalFilter::KisEmbossVerticalFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisEmbossVerticalFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, -1, 0 }, { 0, 2, 0 }, { 0, -1, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisEmbossHorizontalFilter::KisEmbossHorizontalFilter(KisView * view) : 
	KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisEmbossHorizontalFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, 0, 0 }, { -1, 4, -1 }, { 0, 0, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisEmbossDiagonalFilter::KisEmbossDiagonalFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisEmbossDiagonalFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, 0, -1 }, { 0, 4, 0 }, { -1, 0, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}


KisTopEdgeDetectionFilter::KisTopEdgeDetectionFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisTopEdgeDetectionFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 1, 1, 1 }, { 0, 0, 0 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisRightEdgeDetectionFilter::KisRightEdgeDetectionFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisRightEdgeDetectionFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, 0, 1 }, { -1, 0, 1 }, { -1, 0, 1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisBottomEdgeDetectionFilter::KisBottomEdgeDetectionFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisBottomEdgeDetectionFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, -1, -1 }, { 0, 0, 0 }, { 1, 1, 1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}

KisLeftEdgeDetectionFilter::KisLeftEdgeDetectionFilter(KisView * view) : KisConvolutionFilter(name(), view)
{
}

KisMatrix3x3* KisLeftEdgeDetectionFilter::matrixes()
{
	Q_INT32 imgdepth = colorStrategy()->depth();
	KisMatrix3x3* amatrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 1, 0, -1 }, { 1, 0, -1 }, { 1, 0, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		amatrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	amatrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	return amatrixes;
}
