/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _HISTOGRAM_DOCK_H_
#define _HISTOGRAM_DOCK_H_

#include <kddockwidgets/DockWidget.h>
#include <QPointer>

#include <KoCanvasObserverBase.h>

#include <kis_paint_device.h>
#include <kis_canvas2.h>

class QVBoxLayout;
class KisIdleWatcher;
class KoHistogramProducer;
class HistogramDockerWidget;

class HistogramDockerDock : public KDDockWidgets::DockWidget, public KoCanvasObserverBase {
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
