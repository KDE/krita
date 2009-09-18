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
#ifndef KIS_FILTER_JOB_H
#define KIS_FILTER_JOB_H

#include "krita_export.h"
#include <KoProgressUpdater.h>
#include "kis_types.h"
#include "kis_threaded_applicator.h"
#include "kis_processing_information.h"


class KisFilter;
class KisFilterConfiguration;
class QObject;
class QRect;

typedef QPointer<KoUpdater> KoUpdaterPtr;

class KRITAIMAGE_EXPORT KisFilterJob : public KisJob
{
public:

    KisFilterJob(const KisFilter* filter,
                 const KisFilterConfiguration * config,
                 QObject * parent,
                 KisPaintDeviceSP dev,
                 const QRect & rc,
                 KoUpdaterPtr updater,
                 KisSelectionSP selection);

    virtual ~KisFilterJob() {}


    virtual void run();

private:

    const KisFilter * m_filter;
    const KisFilterConfiguration * m_config;
    KoUpdaterPtr m_updater;
    const KisSelectionSP m_selection;
};

class KRITAIMAGE_EXPORT KisFilterJobFactory : public KisJobFactory
{
public:

    KisFilterJobFactory(const KisFilter* filter, const KisFilterConfiguration * config, KisSelectionSP selection = 0);
    ThreadWeaver::Job * createJob(QObject * parent, KisPaintDeviceSP dev, const QRect & rc, KoUpdaterPtr updater);

private:

    const KisFilter * m_filter;
    const KisFilterConfiguration * m_config;
    const KisSelectionSP m_selection;
};

#endif
