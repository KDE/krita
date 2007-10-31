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

void KisConvolutionFilter::process(KisFilterConstantProcessingInformation src,
                 KisFilterProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater
        ) const
{
#if 0
    if (!configuration) {
        setProgressDone();
        return;
    }


    if (dst != src) { // TODO: fix the convolution painter to avoid that stupid copy
//         kDebug() <<"src != dst";
        KisPainter gc(dst);
        gc.bitBlt(dstTopLeft.x(), dstTopLeft.y(), COMPOSITE_COPY, src, srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height());
        gc.end();
    }

    KisConvolutionPainter painter( dst );
    if (progressDisplay())
        progressDisplay()->setSubject( &painter, true, true );

//     KisKernelSP kernel = ((KisConvolutionConfiguration*)configuration)->matrix();
    painter.applyMatrix(m_matrix, dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height(), BORDER_REPEAT);

    if (painter.cancelRequested()) {
        progressUpdater->cancel();
    }

    setProgressDone();
#endif
}

int KisConvolutionFilter::overlapMarginNeeded(KisFilterConfiguration* /*c*/) const {
    return qMax(m_matrix->width / 2, m_matrix->height / 2);
}
