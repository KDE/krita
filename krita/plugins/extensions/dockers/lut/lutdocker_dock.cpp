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

#include <KoFileDialogHelper.h>
#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceFactory.h>
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


struct LutDockerDock::Private {
    Private()
        : page(0),
          canvas(0),
          displayFilter(0),
          draggingSlider(false),
          lastConfigSnapshot(KisConfig())
    {
    }

    QWidget *page;
    KisCanvas2 *canvas;
    OCIO::ConstConfigRcPtr ocioConfig;
    OcioDisplayFilter *displayFilter;

    bool draggingSlider;

    struct KritaOcioConfigSnapshot
    {
        KritaOcioConfigSnapshot(const KisConfig &config)
            : m_useOcio(config.useOcio()),
              m_useOcioEnvironmentVariable(config.useOcioEnvironmentVariable()),
              m_ocioConfigurationPath(config.ocioConfigurationPath()),
              m_ocioLutPath(config.ocioLutPath())
            {
            }

        bool operator ==(const KritaOcioConfigSnapshot &rhs) {
            return
                m_useOcio == rhs.m_useOcio &&
                m_useOcioEnvironmentVariable == rhs.m_useOcioEnvironmentVariable &&
                m_ocioConfigurationPath == rhs.m_ocioConfigurationPath &&
                m_ocioLutPath == rhs.m_ocioLutPath;
        }

        bool operator !=(const KritaOcioConfigSnapshot &rhs) {
            return !(*this == rhs);
        }

        bool m_useOcio;
        bool m_useOcioEnvironmentVariable;
        QString m_ocioConfigurationPath;
        QString m_ocioLutPath;
    };

    KritaOcioConfigSnapshot lastConfigSnapshot;
};

LutDockerDock::LutDockerDock()
    : QDockWidget(i18n("LUT Management")),
      m_d(new Private())
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    m_d->page = new QWidget(this);
    setupUi(m_d->page);
    setWidget(m_d->page);

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

    m_d->draggingSlider = false;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));

    m_d->displayFilter = new OcioDisplayFilter;

    resetOcioConfiguration();
}

LutDockerDock::~LutDockerDock()
{
    delete m_d->displayFilter;
}

void LutDockerDock::setCanvas(KoCanvasBase* _canvas)
{
    //qDebug() << "setCanvas";
    if (KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(_canvas)) {
        m_d->canvas = canvas;
        if (m_d->canvas) {
            connect(m_d->canvas->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), SLOT(slotImageColorSpaceChanged()), Qt::UniqueConnection);
            canvas->setDisplayFilter(m_d->displayFilter);
        }
        slotImageColorSpaceChanged();

    }
    updateDisplaySettings();
}

void LutDockerDock::unsetCanvas()
{
    m_d->canvas = 0;
}

void LutDockerDock::slotConfigChanged()
{
    KisConfig cfg;
    Private::KritaOcioConfigSnapshot snapshot(cfg);

    if (m_d->lastConfigSnapshot != snapshot) {
        slotImageColorSpaceChanged();
    }
}

void LutDockerDock::slotImageColorSpaceChanged()
{
    //qDebug() << "slotImageColorSpaceChanged();";

    if (m_d->canvas && m_d->canvas->view() && m_d->canvas->view()->image()) {
        const KoColorSpace *cs = m_d->canvas->view()->image()->colorSpace();

        m_d->page->setEnabled(cs->colorModelId() == RGBAColorModelID);

        refillComboboxes();

        m_exposureDoubleWidget->setValue(m_d->canvas->view()->resourceProvider()->HDRExposure());
        m_gammaDoubleWidget->setValue(m_d->canvas->view()->resourceProvider()->HDRGamma());

        m_cmbComponents->clear();
        m_cmbComponents->addSqueezedItem(i18n("Luminance"));
        m_cmbComponents->addSqueezedItem(i18n("All Channels"));
        foreach(KoChannelInfo *channel, KoChannelInfo::displayOrderSorted(cs->channels())) {
            m_cmbComponents->addSqueezedItem(channel->name());
        }
        m_cmbComponents->setCurrentIndex(1); // All Channels...
    }
    updateDisplaySettings();
    m_d->lastConfigSnapshot = Private::KritaOcioConfigSnapshot(KisConfig());
}

void LutDockerDock::exposureValueChanged(double exposure)
{
    //qDebug() << "exposureValueChanged();" << exposure;
    if (m_d->canvas && !m_d->draggingSlider) {
        m_d->canvas->view()->resourceProvider()->setHDRExposure(exposure);
        updateDisplaySettings();
    }
}

void LutDockerDock::exposureSliderPressed()
{
    m_d->draggingSlider = true;
}

void LutDockerDock::exposureSliderReleased()
{
    m_d->draggingSlider = false;
    exposureValueChanged(m_exposureDoubleWidget->value());
}


void LutDockerDock::gammaValueChanged(double gamma)
{
    //qDebug() << "gammaValueChanged" << gamma;
    if (m_d->canvas && !m_d->draggingSlider) {
        m_d->canvas->view()->resourceProvider()->setHDRGamma(gamma);
        updateDisplaySettings();
    }
}

void LutDockerDock::gammaSliderPressed()
{
    m_d->draggingSlider = true;
}

void LutDockerDock::gammaSliderReleased()
{
    m_d->draggingSlider = false;
    gammaValueChanged(m_gammaDoubleWidget->value());
}


void LutDockerDock::updateDisplaySettings()
{
    if (!m_d->canvas || !m_d->canvas->view() || !m_d->canvas->view()->image()) return;

    m_d->page->setEnabled(m_d->canvas->view()->image()->colorSpace()->colorModelId() == RGBAColorModelID);

    //    qDebug() << "updateDisplaySettings();" << m_chkUseOcio->isChecked() << m_d->ocioConfig << m_d->canvas->canvasIsOpenGL();
    if (m_chkUseOcio->isChecked() && m_d->ocioConfig) {
        m_d->displayFilter->config = m_d->ocioConfig;
        //        qDebug() << "\t" << m_d->displayFilter->config;
        m_d->displayFilter->inputColorSpaceName = m_d->ocioConfig->getColorSpaceNameByIndex(m_cmbInputColorSpace->currentIndex());
        //        qDebug() << "\t" << m_d->displayFilter->inputColorSpaceName;
        m_d->displayFilter->displayDevice = m_d->ocioConfig->getDisplay(m_cmbDisplayDevice->currentIndex());
        //        qDebug() << "\t" << m_d->displayFilter->displayDevice;
        m_d->displayFilter->view = m_d->ocioConfig->getView(m_d->displayFilter->displayDevice, m_cmbView->currentIndex());
        //        qDebug() << "\t" << m_d->displayFilter->view;
        m_d->displayFilter->gamma = m_gammaDoubleWidget->value();
        //        qDebug() << "\t" << m_d->displayFilter->gamma;
        m_d->displayFilter->exposure = m_exposureDoubleWidget->value();
        //        qDebug() << "\t" << m_d->displayFilter->exposure;
        m_d->displayFilter->swizzle = (OCIO_CHANNEL_SWIZZLE)m_cmbComponents->currentIndex();
        //        qDebug() << "\t" << m_d->displayFilter->swizzle;

        m_d->displayFilter->updateProcessor();
        m_d->canvas->setDisplayFilter(m_d->displayFilter);
    }
    else {
        m_d->canvas->setDisplayFilter(0);
    }
    m_d->canvas->updateCanvas();
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

    filename = KoFileDialogHelper::getOpenFileName(this,
                                                   i18n("Select OpenColorIO Configuration"),
                                                   QDir::cleanPath(filename),
                                                   QStringList("*.ocio"));
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
    m_d->ocioConfig.reset();
    KisConfig cfg;
    //qDebug() << "resetOcioConfiguration" << cfg.useOcioEnvironmentVariable() << cfg.ocioConfigurationPath();
    try {
        if (cfg.useOcioEnvironmentVariable()) {
            //qDebug() << "using OCIO from the environment";
            m_d->ocioConfig = OCIO::Config::CreateFromEnv();
        }
        else {
            QString configFile = cfg.ocioConfigurationPath();
            //qDebug() << "using OCIO config file" << configFile;
            if (QFile::exists(configFile)) {
                m_d->ocioConfig = OCIO::Config::CreateFromFile(configFile.toUtf8());
            }
        }
        if (m_d->ocioConfig) {
            OCIO::SetCurrentConfig(m_d->ocioConfig );
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

    if (!m_d->ocioConfig) return;

    int numOcioColorSpaces = m_d->ocioConfig->getNumColorSpaces();
    for(int i = 0; i < numOcioColorSpaces; ++i) {
        const char *cs = m_d->ocioConfig->getColorSpaceNameByIndex(i);
        OCIO::ConstColorSpaceRcPtr colorSpace = m_d->ocioConfig->getColorSpace(cs);
        m_cmbInputColorSpace->addSqueezedItem(QString::fromUtf8(colorSpace->getName()));
    }
    m_cmbInputColorSpace->blockSignals(false);

    //    int numRoles = m_d->ocioConfig->getNumRoles();
    //    for (int i = 0; i < numRoles; ++i) {
    //        //qDebug() << "role" << m_d->ocioConfig->getRoleName(i);
    //    }

    m_cmbDisplayDevice->blockSignals(true);
    m_cmbDisplayDevice->clear();
    int numDisplays = m_d->ocioConfig->getNumDisplays();
    for (int i = 0; i < numDisplays; ++i) {
        m_cmbDisplayDevice->addSqueezedItem(QString::fromUtf8(m_d->ocioConfig->getDisplay(i)));

    }
    m_cmbDisplayDevice->blockSignals(false);
    refillViewCombobox();

    //    int numLooks = m_d->ocioConfig->getNumLooks();
    //    //qDebug() << "number of looks" << numLooks;
    //    for (int i = 0; i < numLooks; ++i) {
    //        //qDebug() << "look" << m_d->ocioConfig->getLookNameByIndex(i);
    //    }


}

void LutDockerDock::refillViewCombobox()
{
    //    qDebug() << "refillViewCombobox();";
    m_cmbView->blockSignals(true);
    m_cmbView->clear();
    if (!m_d->ocioConfig) return;

    const char *display = m_d->ocioConfig->getDisplay(m_cmbDisplayDevice->currentIndex());
    int numViews = m_d->ocioConfig->getNumViews(display);

    for (int j = 0; j < numViews; ++j) {
        //        qDebug() << "\tview" << m_d->ocioConfig->getView(display, j);
        m_cmbView->addSqueezedItem(QString::fromUtf8(m_d->ocioConfig->getView(display, j)));
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
