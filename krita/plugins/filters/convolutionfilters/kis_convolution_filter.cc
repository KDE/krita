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

#include "klocale.h"

#include "kis_convolution_filter.h"

#include "kis_convolution_painter.h"
#include "kis_progress_display_interface.h"
#include "kis_progress_subject.h"


KisConvolutionFilter::KisConvolutionFilter(const KisID& id, const QString & category, const QString & entry) :
    KisFilter( id, category, entry )
{

}

void KisConvolutionFilter::process(KisPaintDeviceImplSP src,
                   KisPaintDeviceImplSP dst,
                   KisFilterConfiguration* configuration,
                   const QRect& rect)
{
    // XXX: We don't do anything with src here -- carefully test this for problems.
    KisConvolutionPainter painter( dst );
    if (m_progressDisplay)
        m_progressDisplay->setSubject( &painter, true, true );

    KisKernel * kernel = ((KisConvolutionConfiguration*)configuration)->matrix();
    painter.applyMatrix(kernel, rect.x(), rect.y(), rect.width(), rect.height());

    if (painter.cancelRequested()) {
        cancel();
    }

    setProgressDone();
}

KisConvolutionConstFilter::~KisConvolutionConstFilter()
{
}

KisFilterConfiguration* KisConvolutionConstFilter::configuration(QWidget*, KisPaintDeviceImplSP /*dev*/)
{
    return new KisConvolutionConfiguration( m_matrix );
}

#include "kis_convolution_filter.moc"
