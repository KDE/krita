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
#include <QToolButton>

#include <klocale.h>
#include <kcombobox.h>

#include <KoColorSpace.h>
#include <KoColorSpaceFactory.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>

#include <kis_view2.h>
#include <kis_doc2.h>
#include <kis_config.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_config_notifier.h>
#include <widgets/kis_double_widget.h>
#include <kis_image.h>
#include "widgets/squeezedcombobox.h"


#include "ocio_display_filter.h"

LutDockerDock::LutDockerDock()
        : QDockWidget(i18n("LUT Management"))
        , m_canvas(0)
        , m_displayFilter(0)
        , m_draggingSlider(false)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget * w = new QWidget(this);
    setupUi(w);
    setWidget(w);

    KisConfig cfg;
    m_chkUseOcio->setChecked(cfg.useOcio());
    connect(m_chkUseOcio, SIGNAL(toggled(bool)), SLOT(updateWidgets()));

    m_chkUseOcioEnvironment->setChecked(cfg.useOcioEnvironmentVariable());
    connect(m_chkUseOcioEnvironment, SIGNAL(toggled(bool)), SLOT(updateWidgets()));

    m_txtConfigurationPath->setText(cfg.ocioConfigurationPath());

    m_bnSelectConfigurationFile->setToolTip(i18n("Select custom configuration file."));
    connect(m_bnSelectConfigurationFile,SIGNAL(clicked()), SLOT(selectOcioConfiguration()));

    m_txtLut->setText(cfg.ocioLutPath());

    m_bnSelectLut->setToolTip(i18n("Select LUT file"));
    connect(m_bnSelectLut, SIGNAL(clicked()), SLOT(selectLut()));
    connect(m_bnClearLut, SIGNAL(clicked()), SLOT(clearLut()));

    // See http://groups.google.com/group/ocio-dev/browse_thread/thread/ec95c5f54a74af65 -- maybe need to be reinstated
    // when people ask for it.
    m_lblLut->hide();
    m_txtLut->hide();
    m_bnSelectLut->hide();
    m_bnClearLut->hide();

    connect(m_cmbDisplayDevice, SIGNAL(currentIndexChanged(int)), SLOT(refillViewCombobox()));
    connect(m_cmbDisplayDevice, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));

    m_exposureDoubleWidget->setToolTip(i18n("Select the exposure (stops) for HDR images."));
    m_exposureDoubleWidget->setRange(-10, 10);
    m_exposureDoubleWidget->setPrecision(1);
    m_exposureDoubleWidget->setValue(0.0);
    m_exposureDoubleWidget->setSingleStep(0.25);
    m_exposureDoubleWidget->setPageStep(1);

    connect(m_exposureDoubleWidget, SIGNAL(valueChanged(double)), SLOT(exposureValueChanged(double)));
    connect(m_exposureDoubleWidget, SIGNAL(sliderPressed()), SLOT(exposureSliderPressed()));
    connect(m_exposureDoubleWidget, SIGNAL(sliderReleased()), SLOT(exposureSliderReleased()));

    // Gamma needs to be exponential (gamma *= 1.1f, gamma /= 1.1f as steps)

    m_gammaDoubleWidget->setToolTip(i18n("Select the amount of gamma modification for display. This does not affect the pixels of your image."));
    m_gammaDoubleWidget->setRange(0.1, 5);
    m_gammaDoubleWidget->setPrecision(2);
    m_gammaDoubleWidget->setValue(1.0);
    m_gammaDoubleWidget->setSingleStep(0.1);
    m_gammaDoubleWidget->setPageStep(1);

    connect(m_gammaDoubleWidget, SIGNAL(valueChanged(double)), SLOT(gammaValueChanged(double)));
    connect(m_gammaDoubleWidget, SIGNAL(sliderPressed()), SLOT(gammaSliderPressed()));
    connect(m_gammaDoubleWidget, SIGNAL(sliderReleased()), SLOT(gammaSliderReleased()));


    connect(m_cmbInputColorSpace, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));
    connect(m_cmbDisplayDevice, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));
    connect(m_cmbView, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));
    connect(m_cmbComponents, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));

    m_draggingSlider = false;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotImageColorSpaceChanged()));

    m_displayFilter = new OcioDisplayFilter;

    resetOcioConfiguration();
}

LutDockerDock::~LutDockerDock()
{
    delete m_displayFilter;
}

void LutDockerDock::setCanvas(KoCanvasBase* _canvas)
{
    //qDebug() << "setCanvas";
    if (KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(_canvas)) {
        m_canvas = canvas;
        if (m_canvas) {
            connect(m_canvas->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), SLOT(slotImageColorSpaceChanged()), Qt::UniqueConnection);
            canvas->setDisplayFilter(m_displayFilter);
        }
        slotImageColorSpaceChanged();

    }
    updateDisplaySettings();
}

void LutDockerDock::slotImageColorSpaceChanged()
{
    //qDebug() << "slotImageColorSpaceChanged();";
    const KoColorSpace *cs = m_canvas->view()->image()->colorSpace();

    if (m_canvas) {
        refillComboboxes();

        m_exposureDoubleWidget->setValue(m_canvas->view()->resourceProvider()->HDRExposure());
        m_gammaDoubleWidget->setValue(m_canvas->view()->resourceProvider()->HDRGamma());

        m_cmbComponents->clear();
        m_cmbComponents->addSqueezedItem(i18n("Luminance"));
        m_cmbComponents->addSqueezedItem(i18n("All Channels"));
        foreach(KoChannelInfo *channel, KoChannelInfo::displayOrderSorted(cs->channels())) {
            m_cmbComponents->addSqueezedItem(channel->name());
        }
        m_cmbComponents->setCurrentIndex(1); // All Channels...
    }
    updateDisplaySettings();
}

void LutDockerDock::exposureValueChanged(double exposure)
{
    //qDebug() << "exposureValueChanged();" << exposure;
    if (m_canvas && !m_draggingSlider) {
        m_canvas->view()->resourceProvider()->setHDRExposure(exposure);
        updateDisplaySettings();
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
    //qDebug() << "gammaValueChanged" << gamma;
    if (m_canvas && !m_draggingSlider) {
        m_canvas->view()->resourceProvider()->setHDRGamma(gamma);
        updateDisplaySettings();
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
    //qDebug() << "updateDisplaySettings();" << m_chkUseOcio->isChecked() << m_ocioConfig;
    if (m_chkUseOcio->isChecked() && m_ocioConfig) {
        m_displayFilter->config = m_ocioConfig;
        m_displayFilter->inputColorSpaceName = m_ocioConfig->getColorSpaceNameByIndex(m_cmbInputColorSpace->currentIndex());
        m_displayFilter->displayDevice = m_ocioConfig->getDisplay(m_cmbDisplayDevice->currentIndex());
        m_displayFilter->view = m_ocioConfig->getView(m_displayFilter->displayDevice, m_cmbView->currentIndex());
        m_displayFilter->gamma = m_gammaDoubleWidget->value();
        m_displayFilter->exposure = m_exposureDoubleWidget->value();
        m_displayFilter->swizzle = (OCIO_CHANNEL_SWIZZLE)m_cmbComponents->currentIndex();

        m_displayFilter->updateProcessor();
        m_canvas->setDisplayFilter(m_displayFilter);
    }
    else {
        m_canvas->setDisplayFilter(0);
    }
    m_canvas->updateCanvas();
}

void LutDockerDock::updateWidgets()
{
    //qDebug() << "udpateWidgets";
    KisConfig cfg;

    if (cfg.useOcio() != m_chkUseOcio->isChecked()
            || cfg.useOcioEnvironmentVariable() != m_chkUseOcioEnvironment->isChecked()) {

        cfg.setUseOcio(m_chkUseOcio->isChecked());
        cfg.setUseOcioEnvironmentVariable(m_chkUseOcioEnvironment->isChecked());
        resetOcioConfiguration();
    }

    cfg.setOcioConfigurationPath(m_txtConfigurationPath->text());

    lblConfig->setEnabled(!m_chkUseOcioEnvironment->isChecked() && m_chkUseOcio->isChecked());
    m_txtConfigurationPath->setEnabled(!m_chkUseOcioEnvironment->isChecked() && m_chkUseOcio->isChecked());
    m_bnSelectConfigurationFile->setEnabled(!m_chkUseOcioEnvironment->isChecked() && m_chkUseOcio->isChecked());

    updateDisplaySettings();
}

void LutDockerDock::selectOcioConfiguration()
{
    //qDebug() << "selectOcioConfiguration";
    QString filename = m_txtConfigurationPath->text();

    filename = KFileDialog::getOpenFileName(QDir::cleanPath(filename), "*.ocio|OpenColorIO configuration (*.ocio)", this);
    QFile f(filename);
    if (f.exists()) {
        m_txtConfigurationPath->setText(filename);
        KisConfig cfg;
        cfg.setOcioConfigurationPath(filename);
        resetOcioConfiguration();
    }
    updateWidgets();
}

void LutDockerDock::resetOcioConfiguration()
{
    m_ocioConfig.reset();
    KisConfig cfg;
    //qDebug() << "resetOcioConfiguration" << cfg.useOcioEnvironmentVariable() << cfg.ocioConfigurationPath();
    try {
        if (cfg.useOcioEnvironmentVariable()) {
            //qDebug() << "using OCIO from the environment";
            m_ocioConfig = OCIO::Config::CreateFromEnv();
        }
        else {
            QString configFile = cfg.ocioConfigurationPath();
            //qDebug() << "using OCIO config file" << configFile;
            if (QFile::exists(configFile)) {
                m_ocioConfig = OCIO::Config::CreateFromFile(configFile.toUtf8());
            }
        }
        if (m_ocioConfig) {
            OCIO::SetCurrentConfig(m_ocioConfig );
        }
        refillComboboxes();
    }
    catch (OCIO::Exception &exception) {
        kWarning() << "OpenColorIO Error:" << exception.what() << "Cannot create the LUT docker";
    }
}

void LutDockerDock::refillComboboxes()
{
    //qDebug() << "refillComboboxes();";
    m_cmbInputColorSpace->blockSignals(true);

    m_cmbInputColorSpace->clear();

    if (!m_ocioConfig) return;

    int numOcioColorSpaces = m_ocioConfig->getNumColorSpaces();
    for(int i = 0; i < numOcioColorSpaces; ++i) {
        const char *cs = m_ocioConfig->getColorSpaceNameByIndex(i);
        OCIO::ConstColorSpaceRcPtr colorSpace = m_ocioConfig->getColorSpace(cs);
        m_cmbInputColorSpace->addSqueezedItem(QString::fromUtf8(colorSpace->getName()));
    }
    m_cmbInputColorSpace->blockSignals(false);

//    int numRoles = m_ocioConfig->getNumRoles();
//    for (int i = 0; i < numRoles; ++i) {
//        //qDebug() << "role" << m_ocioConfig->getRoleName(i);
//    }

    m_cmbDisplayDevice->blockSignals(true);
    m_cmbDisplayDevice->clear();
    int numDisplays = m_ocioConfig->getNumDisplays();
    for (int i = 0; i < numDisplays; ++i) {
        m_cmbDisplayDevice->addSqueezedItem(QString::fromUtf8(m_ocioConfig->getDisplay(i)));

    }
    m_cmbDisplayDevice->blockSignals(false);
    refillViewCombobox();

//    int numLooks = m_ocioConfig->getNumLooks();
//    //qDebug() << "number of looks" << numLooks;
//    for (int i = 0; i < numLooks; ++i) {
//        //qDebug() << "look" << m_ocioConfig->getLookNameByIndex(i);
//    }


}

void LutDockerDock::refillViewCombobox()
{
    //qDebug() << "refillViewCombobox();";
    m_cmbView->blockSignals(true);
    m_cmbView->clear();
    if (!m_ocioConfig) return;

    const char *display = m_ocioConfig->getDisplay(m_cmbInputColorSpace->currentIndex());
    int numViews = m_ocioConfig->getNumViews(display);

    for (int j = 0; j < numViews; ++j) {
//        //qDebug() << "\tview" << m_ocioConfig->getView(display, j);
        m_cmbView->addSqueezedItem(QString::fromUtf8(m_ocioConfig->getView(display, j)));
    }
    m_cmbView->blockSignals(false);
}

void LutDockerDock::selectLut()
{
    //qDebug() << "selectLut();";
    QString filename = m_txtLut->text();

    filename = KFileDialog::getOpenFileName(QDir::cleanPath(filename), "*.*", this);
    QFile f(filename);
    if (f.exists() && filename != m_txtLut->text()) {
        m_txtLut->setText(filename);
        KisConfig cfg;
        cfg.setOcioLutPath(filename);
        updateDisplaySettings();
    }
}

void LutDockerDock::clearLut()
{
    //qDebug() << "clearLut();";
    m_txtLut->clear();
    updateDisplaySettings();
}

#include "lutdocker_dock.moc"
