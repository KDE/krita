/*
 *  Copyright (c) 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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
#include <QPointer>

#include <KoCanvasObserverBase.h>

#include <kis_paint_device.h>
#include <kis_canvas2.h>

class QVBoxLayout;
class KisIdleWatcher;
class KoHistogramProducer;
class HistogramDockerWidget;

class HistogramDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    HistogramDockerDock();

    QString observerName() override { return "HistogramDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public Q_SLOTS:
    void startUpdateCanvasProjection();
    void sigColorSpaceChanged(const KoColorSpace* cs);
    void updateHistogram();

protected:
    void showEvent(QShowEvent *event) override;

private:
    QVBoxLayout *m_layout;
    KisIdleWatcher *m_imageIdleWatcher;
    HistogramDockerWidget *m_histogramWidget;
    QPointer<KisCanvas2> m_canvas;
};


#endif
