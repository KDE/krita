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

#include "kis_convolution_filter.h"

#include <klocale.h>
#include <kdebug.h>

#include <KoCompositeOp.h>

#include "kis_painter.h"
#include "kis_convolution_painter.h"
#include "KoProgressUpdater.h"

KisConvolutionFilter::KisConvolutionFilter(const KoID& id, const KoID & category, const QString & entry)
    : KisFilter( id, category, entry )
{
    setSupportsIncrementalPainting( false );
    setColorSpaceIndependence( FULLY_INDEPENDENT);
}


void KisConvolutionFilter::process(KisFilterConstantProcessingInformation srcInfo,
                 KisFilterProcessingInformation dstInfo,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater
        ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    if (dst != src) { // TODO: fix the convolution painter to avoid that stupid copy
//         kDebug() <<"src != dst";
        KisPainter gc(dst);
        gc.bitBlt(dstTopLeft.x(), dstTopLeft.y(), COMPOSITE_COPY, src, srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height());
        gc.end();
    }

    KisConvolutionPainter painter( dst, dstInfo.selection() );

    painter.setProgress( progressUpdater );
    painter.applyMatrix(m_matrix, dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height(), BORDER_REPEAT);

}

int KisConvolutionFilter::overlapMarginNeeded(KisFilterConfiguration* /*c*/) const {
    return qMax(m_matrix->width / 2, m_matrix->height / 2);
}
