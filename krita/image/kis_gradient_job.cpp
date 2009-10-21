/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2009
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

#include "kis_gradient_job.h"

#include <QObject>
#include <QRect>

#include <kis_debug.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include "kis_gradient_painter.h"
#include "kis_paint_device.h"
#include "kis_processing_information.h"
#include "kis_painter.h"
#include "kis_selection.h"

KisGradientJob::KisGradientJob(const KisGradientPainter::Configuration* config,
                               QObject* parent,
                               KisPaintDeviceSP dev,
                               const QRect & rc,
                               KoUpdaterPtr updater,
                               KisSelectionSP selection)
        : KisJob(parent, dev, rc)
        , m_config(config)
        , m_updater(updater)
        , m_selection(selection)
{
}

void KisGradientJob::run()
{
    KisGradientPainter painter(m_dev);

    painter.setSelection(m_selection);

    painter.setOpacity(m_config->opacity);
    painter.setCompositeOp(m_config->compositeOp);
    painter.setGradient(m_config->gradient);
    painter.setPaintColor(m_config->fgColor);
    painter.beginTransaction(m_config->transaction);

    painter.paintGradient(m_config->vectorStart, m_config->vectorEnd,
                          m_config->shape, m_config->repeat,
                          m_config->antiAliasThreshold,
                          m_config->reverse,
                          m_rc.x(), m_rc.y(), m_rc.width(), m_rc.height());

    painter.endTransaction();

    m_updater->setProgress(100);
}

KisGradientJobFactory::KisGradientJobFactory(const KisGradientPainter::Configuration * config, KisSelectionSP selection)
        : m_config(config)
        , m_selection(selection)
{
}

ThreadWeaver::Job * KisGradientJobFactory::createJob(QObject * parent, KisPaintDeviceSP dev, const QRect & rc, KoUpdaterPtr updater)
{
    return new KisGradientJob(m_config, parent, dev, rc, updater, m_selection);
}

