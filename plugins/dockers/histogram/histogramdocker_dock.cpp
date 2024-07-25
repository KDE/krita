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
#include "histogramdockerwidget.h"

HistogramDockerDock::HistogramDockerDock()
    : QDockWidget(i18n("Histogram"))
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
    setEnabled(false);
}


void HistogramDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas == canvas)
        return;

    setEnabled(canvas != 0);

    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2*>(canvas);
    m_canvas = canvas2;
    m_histogramWidget->setCanvas(canvas2);
}

void HistogramDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
    m_histogramWidget->setCanvas(0);
}
