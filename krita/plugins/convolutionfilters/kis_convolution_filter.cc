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

#include "klocale.h"

#include "kis_convolution_filter.h"

#include "kis_painter.h"

KisConvolutionFilter::KisConvolutionFilter(const QString& name) :
	KisFilter( name )
{
	
}

void KisConvolutionFilter::process(KisPaintDeviceSP device, KisFilterConfiguration* , const QRect& rect, KisTileCommand* command)
{
	kdDebug() << device << endl;
	KisPainter painter( device );
	kdDebug() << device << endl;
	painter.beginTransaction(i18n("convolve"));
	KisMatrix3x3* amatrixes = matrixes();
	painter.applyConvolutionColorTransformation(amatrixes);
	delete amatrixes;
	painter.endTransaction();
}
