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
#include <kis_matrix.h>
#include <kis_painter.h>

// #include <kmessagebox.h>

#include "convolutionfilters.moc"

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
	
	(void) new KAction("&Gaussian blur", 0, 0, this, SLOT(slotGaussianBlurActivated()), actionCollection(), "convolution_blur");
	(void) new KAction("&Sharpen", 0, 0, this, SLOT(slotSharpenActivated()), actionCollection(), "convolution_sharpen");
	(void) new KAction("&Mean removal", 0, 0, this, SLOT(slotMeanRemovalActivated()), actionCollection(), "convolution_meanremoval");
	(void) new KAction("Emboss laplascian", 0, 0, this, SLOT(slotEmbossLaplascianActivated()), actionCollection(), "convolution_embosslaplascian");
	(void) new KAction("Emboss in all direction", 0, 0, this, SLOT(slotGaussianBlurActivated()), actionCollection(), "convolution_embossalldirections");
	(void) new KAction("Emboss horizontal and vertical", 0, 0, this, SLOT(slotEmbossEmbossAllDirectionsActivated()), actionCollection(), "convolution_embosshorzvertical");
	(void) new KAction("Emboss vertical only", 0, 0, this, SLOT(slotGaussianBlurActivated()), actionCollection(), "convolution_embossverticalonly");
	(void) new KAction("Emboss horizontal only", 0, 0, this, SLOT(slotGaussianBlurActivated()), actionCollection(), "convolution_embosshorizontalonly");
	(void) new KAction("Emboss in diagonal", 0, 0, this, SLOT(slotGaussianBlurActivated()), actionCollection(), "convolution_embossdiagonal");
	(void) new KAction("Top Edge detection", 0, 0, this, SLOT(slotTopEdgeDetectionActivated()), actionCollection(), "convolution_edgedetectiontop");

	if ( !parent->inherits("KisView") )
	{
		kdDebug() << "KritaConvolutionFiltersFactory: KisView expected. Parent is " << parent -> className() << endl;
		m_view = 0;
	} else {
		this->m_view = (KisView*) parent;
	}
}

KritaConvolutionFilters::~KritaConvolutionFilters()
{
}

void KritaConvolutionFilters::slotGaussianBlurActivated()
{
	Q_INT32 imgdepth = depth();
	KisMatrix3x3* matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 1, 2, 1 }, { 2, 4, 2 }, { 1, 2, 1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		matrixes[i] = KisMatrix3x3(mat, 16, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	doIt(matrixes, "Gaussian blur");
}

void KritaConvolutionFilters::slotSharpenActivated()
{
	Q_INT32 imgdepth = depth();
	KisMatrix3x3* matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 0, -2, 0 }, { -2, 11, -2 }, { 0, -2, 0} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		matrixes[i] = KisMatrix3x3(mat, 3, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	doIt(matrixes, "Sharpen");
}

void KritaConvolutionFilters::slotMeanRemovalActivated()
{
	Q_INT32 imgdepth = depth();
	KisMatrix3x3* matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, -1, -1 }, { -1, 9, -1 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		matrixes[i] = KisMatrix3x3(mat, 1, 0);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	doIt(matrixes, "Mean Removal");
}

void KritaConvolutionFilters::slotEmbossLaplascianActivated()
{
	Q_INT32 imgdepth = depth();
	KisMatrix3x3* matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, 0, -1 }, { 0, 4, 0 }, { -1, 0, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	doIt(matrixes, "Emboss laplascian");
}

void KritaConvolutionFilters::slotEmbossEmbossAllDirectionsActivated()
{
	Q_INT32 imgdepth = depth();
	KisMatrix3x3* matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { -1, -1, -1 }, { -1, 8, -1 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	doIt(matrixes, "Emboss all directions");
}

void KritaConvolutionFilters::slotTopEdgeDetectionActivated()
{
	Q_INT32 imgdepth = depth();
	KisMatrix3x3* matrixes = new KisMatrix3x3[imgdepth];
	int mat[3][3] =  { { 1, 1, 1 }, { 0, 0, 0 }, { -1, -1, -1} };
	for(int i = 0; i < imgdepth - 1; i ++)
	{
		matrixes[i] = KisMatrix3x3(mat, 1, 127);
	}
	int matalpha[3][3] =  { { 0, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0} };
	matrixes[imgdepth - 1] = KisMatrix3x3(matalpha, 1, 0);
	doIt(matrixes, "Top Edge detections");
}

Q_INT32 KritaConvolutionFilters::depth()
{
	KisDoc* kD = (KisDoc*) this->m_view->koDocument();
	if( kD->imageNum(0) == 0 )
		return 0;
	return ::imgTypeDepth( kD->imageNum(0)->activeDevice()->typeWithoutAlpha() ) + 1;
	
}

void KritaConvolutionFilters::doIt(KisMatrix3x3* matrixes, const char* name)
{
	KisDoc* kD = (KisDoc*) this->m_view->koDocument();
	if( kD->imageNum(0) == 0 )
		return;
	KisPainter painter( kD->imageNum(0)-> activeDevice());
	painter.beginTransaction(name);
	painter.applyConvolutionColorTransformation(matrixes);
	KisUndoAdapter *adapter = kD->imageNum(0) -> undoAdapter();
	if (adapter ) {
		adapter -> addCommand(painter.endTransaction());
	}
	kD->imageNum(0)->notify();
}
