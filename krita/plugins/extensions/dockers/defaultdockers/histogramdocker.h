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

class KritaHistogramDocker : public KParts::Plugin
{
    Q_OBJECT
public:
    KritaHistogramDocker(QObject *parent, const QStringList &);
    virtual ~KritaHistogramDocker();
private slots:
    void producerChanged(QAction *action);
    void popupMenu(const QPoint & pos);
    void colorSpaceChanged(const KoColorSpace* cs);
private:
    void setChannels();
    KoHistogramProducerFactory* m_factory;
    KisCachedHistogramObserver::Producers m_producers;
    KisAccumulatingHistogramProducer* m_producer;
    const KoColorSpace* m_cs;
    KisView2* m_view;
    KisHistogramView* m_hview;
    KisImageRasteredCache* m_cache;
    QMenu m_popup;
    KisHistogramSP m_histogram;
    int m_currentProducerPos;
    QDockWidget* m_docker;
};

#endif //_HISTOGRAMDOCKER_H_
