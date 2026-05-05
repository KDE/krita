/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "histogramdocker_dock.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QSpacerItem>
#include <klocalizedstring.h>


#include "kis_canvas2.h"
#include <KisViewManager.h>
#include "histogramdockerwidget.h"

HistogramDockerDock::HistogramDockerDock()
    : QDockWidget(i18n("Histogram"))
{
    QWidget *page = new QWidget(this);
    m_layout = new QVBoxLayout(page);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_layout->addLayout(buttonLayout);
    QToolButton *toggleLogarithm = new QToolButton(this);
    buttonLayout->addWidget(toggleLogarithm);
    buttonLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    toggleLogarithm->setIcon(KisIconUtils::loadIcon("histogram-logarithmic"));
    toggleLogarithm->setToolTip(i18nc("@info:tooltip", "Toggle logarithmic histogram."));
    toggleLogarithm->setCheckable(true);

    m_histogramWidget = new HistogramDockerWidget(this);

    m_histogramWidget->setBackgroundRole(QPalette::AlternateBase);
    m_histogramWidget->setAutoFillBackground(true); // paints background role before paint()

    m_histogramWidget->setMinimumHeight(50);
    //m_histogramWidget->setSmoothHistogram(false);
    m_layout->addWidget(m_histogramWidget, 1);

    connect(toggleLogarithm, SIGNAL(toggled(bool)), m_histogramWidget, SLOT(enableLogarithmic(bool)));

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
