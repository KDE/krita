/*
 *  Copyright (c) 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _HISTOGRAM_DOCK_H_
#define _HISTOGRAM_DOCK_H_

#include <QDockWidget>
#include <KoCanvasObserverBase.h>
#include <kis_paint_device.h>

class QVBoxLayout;
class KisCanvas2;
class KisHistogramView;
class KisSignalCompressor;
class KoHistogramProducer;

class HistogramDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    HistogramDockerDock();

    QString observerName() { return "HistogramDockerDock"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();

public Q_SLOTS:
    virtual void startUpdateCanvasProjection();
    //virtual void sigProfileChanged(const KoColorProfile* cp);
    virtual void sigColorSpaceChanged(const KoColorSpace* cs);

private:
    QVBoxLayout *m_layout;
    KisSignalCompressor *m_compressor;
    KisHistogramView *m_histogramWidget;
    KisCanvas2 *m_canvas;
    KoHistogramProducer *m_producer;
};


#endif
