/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include "histogram_updater.h"
#include <QMenu>
#include <QDockWidget>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>

#include "KoDockFactory.h"
#include "KoBasicHistogramProducers.h"
#include "KoColorSpaceRegistry.h"
#include "KoID.h"

#include "kis_global.h"
#include "kis_types.h"
#include "kis_image.h"
#include "kis_view2.h"
#include <kis_histogram_view.h>

#include "kis_imagerasteredcache.h"
#include "kis_accumulating_producer.h"

HistogramDockerUpdater::HistogramDockerUpdater(QObject* /*parent*/, KisHistogramSP h, KisHistogramView* v,
        KisAccumulatingHistogramProducer* p)
        : m_histogram(h), m_view(v), m_producer(p)
{
    connect(p, SIGNAL(completed()), this, SLOT(completed()));
}

void HistogramDockerUpdater::updated()
{
    // We don't [!] do m_histogram->updateHistogram();, because that will try to compute
    // the histogram synchronously, while we want it asynchronously.
    m_producer->addRegionsToBinAsync();
}

void HistogramDockerUpdater::completed()
{
    if (m_histogram) {
        m_histogram->computeHistogram();
    }
    m_view->updateHistogram();
}

#include "histogram_updater.moc"
