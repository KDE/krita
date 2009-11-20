/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2007
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "filter/kis_filter_job.h"
#include <QObject>
#include <QRect>

#include <kis_debug.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>


#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_paint_device.h"
#include "kis_processing_information.h"
#include "kis_painter.h"
#include "kis_selection.h"

KisFilterJob::KisFilterJob(const KisFilter* filter,
                           const KisFilterConfiguration * config,
                           QObject * parent, KisPaintDeviceSP dev,
                           const QRect & rc,
                           KoUpdaterPtr updater,
                           KisSelectionSP selection)
        : KisJob(parent, dev, rc)
        , m_filter(filter)
        , m_config(config)
        , m_updater(updater)
        , m_selection(selection)
{
}


void KisFilterJob::run()
{
    KisPaintDeviceSP dst = new KisPaintDevice(m_dev->colorSpace());
    QRect marginRect = m_filter->neededRect(m_rc, m_config);

    m_filter->process(KisConstProcessingInformation(m_dev, marginRect.topLeft(), m_selection),
                      KisProcessingInformation(dst, marginRect.topLeft(), m_selection),
                      marginRect.size(),
                      m_config,
                      m_updater);

    // blt back onto the original
    KisPainter p2(m_dev);
    p2.setCompositeOp(m_dev->colorSpace()->compositeOp(COMPOSITE_OVER));
    p2.setSelection(m_selection);
    p2.bitBlt(m_rc.topLeft(), dst, m_rc);
    p2.end();

    m_updater->setProgress(100);
    m_interrupted = m_updater->interrupted();
}

KisFilterJobFactory::KisFilterJobFactory(const KisFilter* filter, const KisFilterConfiguration * config, KisSelectionSP selection)
        : m_filter(filter)
        , m_config(config)
        , m_selection(selection)
{
}

ThreadWeaver::Job * KisFilterJobFactory::createJob(QObject * parent, KisPaintDeviceSP dev, const QRect & rc, KoUpdaterPtr updater)
{
    return new KisFilterJob(m_filter, m_config, parent, dev, rc, updater, m_selection);
}

