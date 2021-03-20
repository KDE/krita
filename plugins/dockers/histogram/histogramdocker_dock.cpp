/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "histogramdocker_dock.h"

#include <QLabel>
#include <QVBoxLayout>
#include <klocalizedstring.h>

#include "kis_canvas2.h"
#include <KisViewManager.h>
#include <kis_zoom_manager.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_idle_watcher.h"
#include "histogramdockerwidget.h"

HistogramDockerDock::HistogramDockerDock()
    : QDockWidget(i18n("Histogram")),
      m_imageIdleWatcher(new KisIdleWatcher(250, this)),
      m_canvas(0)
{
    QWidget *page = new QWidget(this);
    m_layout = new QVBoxLayout(page);

    m_histogramWidget = new HistogramDockerWidget(this);

    m_histogramWidget->setBackgroundRole(QPalette::AlternateBase);
    m_histogramWidget->setAutoFillBackground(true); // paints background role before paint()

    m_histogramWidget->setMinimumHeight(50);
    //m_histogramWidget->setSmoothHistogram(false);
    m_layout->addWidget(m_histogramWidget, 1);
    setWidget(page);
    connect(m_imageIdleWatcher, &KisIdleWatcher::startedIdleMode, this, &HistogramDockerDock::updateHistogram);
}


void HistogramDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas == canvas)
        return;

    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if (m_canvas) {

        m_imageIdleWatcher->setTrackedImage(m_canvas->image());
        connect(m_imageIdleWatcher, &KisIdleWatcher::startedIdleMode, this, &HistogramDockerDock::updateHistogram, Qt::UniqueConnection);

        connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(startUpdateCanvasProjection()), Qt::UniqueConnection);
        connect(m_canvas->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), this, SLOT(sigColorSpaceChanged(const KoColorSpace*)), Qt::UniqueConnection);
        m_imageIdleWatcher->startCountdown();
    }
}

void HistogramDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
    m_imageIdleWatcher->startCountdown();
}

void HistogramDockerDock::startUpdateCanvasProjection()
{
    if (isVisible()) {
        m_imageIdleWatcher->startCountdown();
    }
}

void HistogramDockerDock::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_imageIdleWatcher->startCountdown();
}


void HistogramDockerDock::sigColorSpaceChanged(const KoColorSpace */*cs*/)
{
    if (isVisible()) {
        m_imageIdleWatcher->startCountdown();
    }
}

void HistogramDockerDock::updateHistogram()
{
    if (isVisible()) {
        m_histogramWidget->updateHistogram(m_canvas);
    }
}
