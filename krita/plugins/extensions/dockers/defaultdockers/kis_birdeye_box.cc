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
#include <QFormLayout>
#include <QCheckBox>
#include <QApplication>
#include <QDesktopWidget>

#include <klocale.h>
#include <kcombobox.h>

#include <KoColorSpace.h>
#include <KoColorSpaceFactory.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

#include <kis_view2.h>
#include <kis_doc2.h>
#include <kis_config.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_config_notifier.h>
#include <widgets/kis_double_widget.h>
#include <kis_image.h>
#include "widgets/squeezedcombobox.h"

KisBirdEyeBox::KisBirdEyeBox()
        : QDockWidget(i18n("Overview"))
        , m_canvas(0)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget * w = new QWidget(this);
    setWidget(w);

    QFormLayout *layout = new QFormLayout(w);

    m_exposureDoubleWidget = new KisDoubleWidget(-10, 10, w);
    m_exposureDoubleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_exposureDoubleWidget->setToolTip(i18n("Select the exposure (stops) for HDR images."));
    m_exposureDoubleWidget->setPrecision(1);
    m_exposureDoubleWidget->setValue(0);
    m_exposureDoubleWidget->setSingleStep(0.1);
    m_exposureDoubleWidget->setPageStep(1);

    connect(m_exposureDoubleWidget, SIGNAL(valueChanged(double)), SLOT(exposureValueChanged(double)));
    connect(m_exposureDoubleWidget, SIGNAL(sliderPressed()), SLOT(exposureSliderPressed()));
    connect(m_exposureDoubleWidget, SIGNAL(sliderReleased()), SLOT(exposureSliderReleased()));

    m_gammaDoubleWidget = new KisDoubleWidget(0, 5, w);
    m_gammaDoubleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_gammaDoubleWidget->setToolTip(i18n("Select the amount of gamma modificiation for display. This does not affect the pixels of your image."));
    m_gammaDoubleWidget->setPrecision(2);
    m_gammaDoubleWidget->setValue(2.2);
    m_gammaDoubleWidget->setSingleStep(0.1);
    m_gammaDoubleWidget->setPageStep(1);

    connect(m_gammaDoubleWidget, SIGNAL(valueChanged(double)), SLOT(gammaValueChanged(double)));
    connect(m_gammaDoubleWidget, SIGNAL(sliderPressed()), SLOT(gammaSliderPressed()));
    connect(m_gammaDoubleWidget, SIGNAL(sliderReleased()), SLOT(gammaSliderReleased()));

    m_cmbDisplayProfile = new SqueezedComboBox(w);
    m_cmbDisplayProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_cmbDisplayProfile, SIGNAL(activated(int)), SLOT(updateDisplaySettings()));

    m_cmbMonitorIntent = new KComboBox(w);
    m_cmbMonitorIntent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_cmbMonitorIntent->addItems(QStringList() << i18n("Perceptual") << i18n("Relative Colorimetric") << i18n("Saturation") << i18n("Absolute Colorimetric"));
    connect(m_cmbMonitorIntent, SIGNAL(activated(int)), SLOT(updateDisplaySettings()));

    m_chkBlackPoint = new QCheckBox(i18n("Use Blackpoint Compensation"));
    connect(m_chkBlackPoint, SIGNAL(toggled(bool)), SLOT(updateDisplaySettings()));

    m_draggingSlider = false;

    layout->addRow(i18n("Exposure:"), m_exposureDoubleWidget);
    layout->addRow(i18n("Gamma:"), m_gammaDoubleWidget);
    layout->addRow(i18n("Display profile"), m_cmbDisplayProfile);
    layout->addRow(i18n("Rendering Intent"), m_cmbMonitorIntent);
    layout->addWidget(m_chkBlackPoint);
    layout->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
}

KisBirdEyeBox::~KisBirdEyeBox()
{
}

void KisBirdEyeBox::setCanvas(KoCanvasBase* _canvas)
{
    if (KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(_canvas)) {
        m_canvas = canvas;

        m_exposureDoubleWidget->setValue(m_canvas->view()->resourceProvider()->HDRExposure());
        m_gammaDoubleWidget->setValue(m_canvas->view()->resourceProvider()->HDRGamma());

        connect(m_canvas->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), SLOT(slotImageColorSpaceChanged(const KoColorSpace*)), Qt::UniqueConnection);

        const KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->colorSpaceFactory("RGBA");
        m_cmbDisplayProfile->clear();

        if (csf) {

            QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);

            foreach(const KoColorProfile *profile, profileList) {
                if (profile->isSuitableForDisplay())
                    m_cmbDisplayProfile->addSqueezedItem(profile->name());
            }

            KoColorProfile *monitorProfile = m_canvas->monitorProfile();
            if (monitorProfile) {
                m_cmbDisplayProfile->setCurrent(monitorProfile->name());
            }
            else {
                m_cmbDisplayProfile->setCurrent(csf->defaultProfile());
            }

        }
        KisConfig cfg;
        m_chkBlackPoint->setChecked(cfg.useBlackPointCompensation());
        m_cmbMonitorIntent->setCurrentIndex(cfg.renderIntent());
    }
}

void KisBirdEyeBox::slotImageColorSpaceChanged(const KoColorSpace *cs)
{
    m_exposureDoubleWidget->setEnabled(cs->hasHighDynamicRange() && m_canvas->canvasIsOpenGL());
    m_gammaDoubleWidget->setEnabled(cs->hasHighDynamicRange() && m_canvas->canvasIsOpenGL());
}

void KisBirdEyeBox::exposureValueChanged(double exposure)
{
    if (m_canvas && (!m_draggingSlider || m_canvas->usingHDRExposureProgram())) {
        m_canvas->view()->resourceProvider()->setHDRExposure(exposure);
    }
}

void KisBirdEyeBox::exposureSliderPressed()
{
    m_draggingSlider = true;
}

void KisBirdEyeBox::exposureSliderReleased()
{
    m_draggingSlider = false;
    exposureValueChanged(m_exposureDoubleWidget->value());
}

void KisBirdEyeBox::gammaValueChanged(double gamma)
{
    if (m_canvas && (!m_draggingSlider || m_canvas->usingHDRExposureProgram())) {
        m_canvas->view()->resourceProvider()->setHDRGamma(gamma);
    }
}

void KisBirdEyeBox::gammaSliderPressed()
{
    m_draggingSlider = true;
}

void KisBirdEyeBox::gammaSliderReleased()
{
    m_draggingSlider = false;
    gammaValueChanged(m_gammaDoubleWidget->value());
}

void KisBirdEyeBox::updateDisplaySettings()
{
    KisConfig cfg;
    cfg.setMonitorProfile(m_cmbDisplayProfile->itemHighlighted(), true);
    cfg.setUseBlackPointCompensation(m_chkBlackPoint->isChecked());
    cfg.setRenderIntent(m_cmbMonitorIntent->currentIndex());

    KisConfigNotifier::instance()->notifyConfigChanged();
    m_canvas->view()->resourceProvider()->resetDisplayProfile(QApplication::desktop()->screenNumber(this));
}

#include "kis_birdeye_box.moc"
