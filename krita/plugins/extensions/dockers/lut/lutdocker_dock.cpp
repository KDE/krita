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

#include "lutdocker_dock.h"

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


LutDockerDock::LutDockerDock(OCIO::ConstConfigRcPtr config)
        : QDockWidget(i18n("LUT Management"))
        , m_canvas(0)
        , m_ocioConfig(config)
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
    connect(m_cmbDisplayProfile, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));

    m_draggingSlider = false;

    layout->addRow(i18n("Exposure:"), m_exposureDoubleWidget);
    layout->addRow(i18n("Gamma:"), m_gammaDoubleWidget);

    layout->addRow(i18n("Display profile"), m_cmbDisplayProfile);
    layout->addItem(new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotImageColorSpaceChanged()));

}

LutDockerDock::~LutDockerDock()
{
}

void LutDockerDock::setCanvas(KoCanvasBase* _canvas)
{
    if (KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(_canvas)) {
        m_canvas = canvas;
        if (m_canvas) {
            connect(m_canvas->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), SLOT(slotImageColorSpaceChanged()), Qt::UniqueConnection);
        }
        slotImageColorSpaceChanged();
    }
}

void LutDockerDock::slotImageColorSpaceChanged()
{
    KisConfig cfg;

    const KoColorSpace *cs = m_canvas->view()->image()->colorSpace();

    m_exposureDoubleWidget->setEnabled(cs->hasHighDynamicRange() && cfg.useOpenGL() && cfg.useOpenGLShaders());
    m_gammaDoubleWidget->setEnabled(cs->hasHighDynamicRange() && cfg.useOpenGL() && cfg.useOpenGLShaders());

    if (m_canvas) {

        m_updateDisplay = false;

        m_exposureDoubleWidget->setValue(m_canvas->view()->resourceProvider()->HDRExposure());
        m_gammaDoubleWidget->setValue(m_canvas->view()->resourceProvider()->HDRGamma());

        m_cmbDisplayProfile->clear();
        int numOcioColorSpaces = m_ocioConfig->getNumColorSpaces();
        for(int i = 0; i < numOcioColorSpaces; ++i) {
            const char *cs = m_ocioConfig->getColorSpaceNameByIndex(i);
            OCIO::ConstColorSpaceRcPtr colorSpace = m_ocioConfig->getColorSpace(cs);
            m_cmbDisplayProfile->addSqueezedItem(QString::fromUtf8(colorSpace->getDescription()).replace("\n", ""));
            qDebug() << colorSpace->getDescription();
        }

        int numRoles = m_ocioConfig->getNumRoles();
        for (int i = 0; i < numRoles; ++i) {
            qDebug() << "role" << m_ocioConfig->getRoleName(i);
        }
        int numDisplays = m_ocioConfig->getNumDisplays();
        for (int i = 0; i < numDisplays; ++i) {
            qDebug() << "display" << m_ocioConfig->getDisplay(i);
            int numViews = m_ocioConfig->getNumViews(m_ocioConfig->getDisplay(i));
            for (int j = 0; j < numViews; ++j) {
                qDebug() << "\tview" << m_ocioConfig->getView(m_ocioConfig->getDisplay(i), j);
            }

        }
        int numLooks = m_ocioConfig->getNumLooks();
        for (int i = 0; i < numLooks; ++i) {
            qDebug() << "look" << m_ocioConfig->getLookNameByIndex(i);
        }

        m_updateDisplay = true;
    }
}

void LutDockerDock::exposureValueChanged(double exposure)
{
    if (m_canvas && (!m_draggingSlider || m_canvas->usingHDRExposureProgram())) {
        m_canvas->view()->resourceProvider()->setHDRExposure(exposure);
    }
}

void LutDockerDock::exposureSliderPressed()
{
    m_draggingSlider = true;
}

void LutDockerDock::exposureSliderReleased()
{
    m_draggingSlider = false;
    exposureValueChanged(m_exposureDoubleWidget->value());
}

void LutDockerDock::gammaValueChanged(double gamma)
{
    if (m_canvas && (!m_draggingSlider || m_canvas->usingHDRExposureProgram())) {
        m_canvas->view()->resourceProvider()->setHDRGamma(gamma);
    }
}

void LutDockerDock::gammaSliderPressed()
{
    m_draggingSlider = true;
}

void LutDockerDock::gammaSliderReleased()
{
    m_draggingSlider = false;
    gammaValueChanged(m_gammaDoubleWidget->value());
}

void LutDockerDock::updateDisplaySettings()
{
    if (m_updateDisplay) {
        KisConfig cfg;
        cfg.setMonitorProfile(m_cmbDisplayProfile->itemHighlighted(), true);

        KisConfigNotifier::instance()->notifyConfigChanged();
        m_canvas->view()->resourceProvider()->resetDisplayProfile(QApplication::desktop()->screenNumber(this));
    }
}

#include "lutdocker_dock.moc"
