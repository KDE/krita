/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_birdeye_box.h"

#include <QLayout>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klocale.h>

#include <KoColorSpace.h>

#include <kis_view2.h>
#include <kis_doc2.h>

#include <widgets/kis_double_widget.h>
#include <canvas/kis_canvas2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_canvas_resource_provider.h>


KisBirdEyeBox::KisBirdEyeBox()
        : QDockWidget(i18n("Overview"))
        , m_canvas(0)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget * w = new QWidget(this);
    setWidget(w);

    QVBoxLayout * l = new QVBoxLayout(w);

    QHBoxLayout * hl = new QHBoxLayout();
    l->addLayout(hl);

    m_exposureLabel = new QLabel(i18n("Exposure:"), w);
    hl->addWidget(m_exposureLabel);

    m_exposureDoubleWidget = new KisDoubleWidget(-10, 10, w);
    m_exposureDoubleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_exposureDoubleWidget->setToolTip("Select the exposure (stops) for HDR images");
    hl->addWidget(m_exposureDoubleWidget);

    l->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

    m_exposureDoubleWidget->setPrecision(1);
    m_exposureDoubleWidget->setValue(0);
    m_exposureDoubleWidget->setSingleStep(0.1);
    m_exposureDoubleWidget->setPageStep(1);

    connect(m_exposureDoubleWidget, SIGNAL(valueChanged(double)), SLOT(exposureValueChanged(double)));
    connect(m_exposureDoubleWidget, SIGNAL(sliderPressed()), SLOT(exposureSliderPressed()));
    connect(m_exposureDoubleWidget, SIGNAL(sliderReleased()), SLOT(exposureSliderReleased()));

    m_draggingExposureSlider = false;
}

KisBirdEyeBox::~KisBirdEyeBox()
{
}

void KisBirdEyeBox::setCanvas(KoCanvasBase* _canvas)
{
    if (KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(_canvas)) {
        m_canvas = canvas;
    }
}

void KisBirdEyeBox::slotImageColorSpaceChanged(const KoColorSpace *cs)
{
    if (cs->hasHighDynamicRange()) {
        m_exposureDoubleWidget->show();
        m_exposureLabel->show();
    } else {
        m_exposureDoubleWidget->hide();
        m_exposureLabel->hide();
    }
}

void KisBirdEyeBox::exposureValueChanged(double exposure)
{
    if (m_canvas && (!m_draggingExposureSlider || m_canvas->usingHDRExposureProgram())) {
        m_canvas->view()->resourceProvider()->setHDRExposure(exposure);
    }
}

void KisBirdEyeBox::exposureSliderPressed()
{
    m_draggingExposureSlider = true;
}

void KisBirdEyeBox::exposureSliderReleased()
{
    m_draggingExposureSlider = false;
    exposureValueChanged(m_exposureDoubleWidget->value());
}

#include "kis_birdeye_box.moc"
