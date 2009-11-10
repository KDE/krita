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
#ifndef HISTOGRAM_DOCK_H
#define HISTOGRAM_DOCK_H

#include <QDockWidget>
#include <QMenu>

#include <KoCanvasObserver.h>

#include <kis_types.h>
#include "kis_cachedhistogram.h"

class KoColorSpace;
class KoHistogramProducerFactory;
class KisAccumulatingHistogramProducer;
class KisCanvas2;
class KisHistogramView;
class KisImagerasteredCache;

class KisHistogramDocker : public QDockWidget, public KoCanvasObserver
{

    Q_OBJECT

public:
    KisHistogramDocker();
    virtual ~KisHistogramDocker();

    /// reimplemented from KoCanvasObserver
    virtual void setCanvas(KoCanvasBase *canvas);

private slots:

    void setImage(KisImageWSP image);
    void producerChanged(QAction *action);
    void popupMenu(const QPoint & pos);
    void colorSpaceChanged(const KoColorSpace* cs);
    void reset();

private:

    void setChannels();

    KisCanvas2* m_canvas;
    KisImageWSP m_image;
    KoHistogramProducerFactory* m_factory;
    KisCachedHistogramObserver::Producers m_producers;
    KisAccumulatingHistogramProducer* m_producer;
    const KoColorSpace* m_cs;
    KisHistogramView* m_hview;
    KisImageRasteredCache* m_cache;
    QMenu m_popup;
    KisHistogramSP m_histogram;
    int m_currentProducerPos;


};

#endif
