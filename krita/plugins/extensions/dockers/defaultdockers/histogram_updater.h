/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#ifndef _HISTOGRAM_UPDATER_H_
#define _HISTOGRAM_UPDATER_H_

#include <QObject>
#include <QMenu>
#include <QDockWidget>

#include <kparts/plugin.h>
#include <kis_histogram.h>

#include "kis_cachedhistogram.h"

class KisAccumulatingHistogramProducer;
class KoColorSpace;
class KisHistogramView;
class KisView2;
class KoColorSpace;

class HistogramDockerUpdater : public QObject
{
    Q_OBJECT
public:
    HistogramDockerUpdater(QObject* parent, KisHistogramSP h, KisHistogramView* v,
                           KisAccumulatingHistogramProducer* p);
public slots:
    void updated();
private slots:
    void completed();
private:
    KisHistogramSP m_histogram;
    KisHistogramView* m_view;
    KisAccumulatingHistogramProducer* m_producer;
};

#endif
