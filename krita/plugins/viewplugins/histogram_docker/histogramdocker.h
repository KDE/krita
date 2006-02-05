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

#ifndef _HISTOGRAMDOCKER_H_
#define _HISTOGRAMDOCKER_H_

#include <qobject.h>
#include <qpopupmenu.h>

#include <kparts/plugin.h>
#include <kis_histogram_view.h>
#include <kis_histogram.h>

#include "kis_cachedhistogram.h"

class KisAccumulatingHistogramProducer;
class KisColorSpace;
class KisHistogramView;
class KisView;
class KisColorSpace;

class KritaHistogramDocker : public KParts::Plugin
{
Q_OBJECT
public:
    KritaHistogramDocker(QObject *parent, const char *name, const QStringList &);
    virtual ~KritaHistogramDocker();
private slots:
    void producerChanged(int pos);
    void popupMenu(const QPoint & pos);
    void colorSpaceChanged(KisColorSpace* cs);
private:
    KisHistogramProducerFactory* m_factory;
    KisCachedHistogramObserver::Producers m_producers;
    KisAccumulatingHistogramProducer* m_producer;
    KisColorSpace* m_cs;
    KisView* m_view;
    KisHistogramView* m_hview;
    KisImageRasteredCache* m_cache;
    QPopupMenu m_popup;
    KisHistogramSP m_histogram;
    uint m_currentProducerPos;
};

class KisGenericRGBHistogramProducerFactory;

class HistogramDockerUpdater : public QObject {
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

#endif //_HISTOGRAMDOCKER_H_
