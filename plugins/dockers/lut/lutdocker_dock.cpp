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
#include <config-hdr.h>

#include <sstream>

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

#include <klocalizedstring.h>

#include <KisMainWindow.h>
#include <KoFileDialog.h>
#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceFactory.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>

#include "kis_icon_utils.h"
#include <KisViewManager.h>
#include <KisDocument.h>
#include <kis_config.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_config_notifier.h>
#include <widgets/kis_double_widget.h>
#include <kis_image.h>
#include <KisSqueezedComboBox.h>
#include "kis_signals_blocker.h"
#include "krita_utils.h"

#include "ocio_display_filter.h"
#include "black_white_point_chooser.h"
#include "KisOcioConfiguration.h"
#include <opengl/KisOpenGLModeProber.h>


OCIO::ConstConfigRcPtr defaultRawProfile()
{
    /**
     * Copied from OCIO, just a noop profile
     */
    const char * INTERNAL_RAW_PROFILE =
        "ocio_profile_version: 1\n"
        "strictparsing: false\n"
        "roles:\n"
        "  default: raw\n"
        "displays:\n"
        "  sRGB:\n"
        "  - !<View> {name: Raw, colorspace: raw}\n"
        "colorspaces:\n"
        "  - !<ColorSpace>\n"
        "      name: raw\n"
        "      family: raw\n"
        "      equalitygroup:\n"
        "      bitdepth: 32f\n"
        "      isdata: true\n"
        "      allocation: uniform\n"
        "      description: 'A raw color space. Conversions to and from this space are no-ops.'\n";


    std::istringstream istream;
    istream.str(INTERNAL_RAW_PROFILE);
    return OCIO::Config::CreateFromStream(istream);
}

LutDockerDock::LutDockerDock()
    : QDockWidget(i18n("LUT Management"))
    , m_canvas(0)
    , m_draggingSlider(false)
{
    using namespace std::placeholders; // For _1
    m_exposureCompressor.reset(
        new KisSignalCompressorWithParam<qreal>(40, std::bind(&LutDockerDock::setCurrentExposureImpl, this, _1)));
    m_gammaCompressor.reset(
        new KisSignalCompressorWithParam<qreal>(40, std::bind(&LutDockerDock::setCurrentGammaImpl, this, _1)));

    m_page = new QWidget(this);
    setupUi(m_page);
    setWidget(m_page);

    KisConfig cfg(true);
    m_chkUseOcio->setChecked(cfg.useOcio());
    connect(m_chkUseOcio, SIGNAL(toggled(bool)), SLOT(updateDisplaySettings()));
    connect(m_colorManagement, SIGNAL(currentIndexChanged(int)), SLOT(slotColorManagementModeChanged()));

    m_bnSelectConfigurationFile->setToolTip(i18n("Select custom configuration file."));
    connect(m_bnSelectConfigurationFile,SIGNAL(clicked()), SLOT(selectOcioConfiguration()));

    KisOcioConfiguration ocioOptions = cfg.ocioConfiguration();
    m_txtConfigurationPath->setText(ocioOptions.configurationPath);
    m_txtLut->setText(ocioOptions.lutPath);

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

    m_bwPointChooser = new BlackWhitePointChooser(this);

    connect(m_bwPointChooser, SIGNAL(sigBlackPointChanged(qreal)), SLOT(updateDisplaySettings()));
    connect(m_bwPointChooser, SIGNAL(sigWhitePointChanged(qreal)), SLOT(updateDisplaySettings()));

    connect(m_btnConvertCurrentColor, SIGNAL(toggled(bool)), SLOT(updateDisplaySettings()));
    connect(m_btmShowBWConfiguration, SIGNAL(clicked()), SLOT(slotShowBWConfiguration()));
    slotUpdateIcons();

    connect(m_cmbInputColorSpace, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));
    connect(m_cmbDisplayDevice, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));
    connect(m_cmbView, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));
    connect(m_cmbLook, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));
    connect(m_cmbComponents, SIGNAL(currentIndexChanged(int)), SLOT(updateDisplaySettings()));

    m_draggingSlider = false;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetOcioConfiguration()));

    resetOcioConfiguration();
}

LutDockerDock::~LutDockerDock()
{
}

void LutDockerDock::setCanvas(KoCanvasBase* _canvas)
{
    if (m_canvas) {
        m_canvas->disconnect(this);
    }

    setEnabled(_canvas != 0);

    if (KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(_canvas)) {
        m_canvas = canvas;
        if (m_canvas) {
            if (!m_canvas->displayFilter()) {
                resetOcioConfiguration();
                updateDisplaySettings();
            }
            else {
                m_displayFilter = m_canvas->displayFilter();
                OcioDisplayFilter *displayFilter = qobject_cast<OcioDisplayFilter*>(m_displayFilter.data());
                Q_ASSERT(displayFilter);
                m_ocioConfig = displayFilter->config;
                KisSignalsBlocker exposureBlocker(m_exposureDoubleWidget);
                m_exposureDoubleWidget->setValue(displayFilter->exposure);
                KisSignalsBlocker gammaBlocker(m_gammaDoubleWidget);
                m_gammaDoubleWidget->setValue(displayFilter->gamma);
                KisSignalsBlocker componentsBlocker(m_cmbComponents);
                m_cmbComponents->setCurrentIndex((int)displayFilter->swizzle);
                KisSignalsBlocker bwBlocker(m_bwPointChooser);
                m_bwPointChooser->setBlackPoint(displayFilter->blackPoint);
                m_bwPointChooser->setWhitePoint(displayFilter->whitePoint);
            }

            connect(m_canvas->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), SLOT(slotImageColorSpaceChanged()), Qt::UniqueConnection);
            connect(m_canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()), SLOT(slotUpdateIcons()), Qt::UniqueConnection);

        }
    }
}

void LutDockerDock::unsetCanvas()
{
    m_canvas = 0;
    setEnabled(false);
    m_displayFilter = QSharedPointer<KisDisplayFilter>(0);
}

void LutDockerDock::slotUpdateIcons()
{
    m_btnConvertCurrentColor->setIcon(KisIconUtils::loadIcon("krita_tool_freehand"));
    m_btmShowBWConfiguration->setIcon(KisIconUtils::loadIcon("properties"));
}

void LutDockerDock::slotShowBWConfiguration()
{
    m_bwPointChooser->showPopup(m_btmShowBWConfiguration->mapToGlobal(QPoint()));
}

bool LutDockerDock::canChangeExposureAndGamma() const
{
    if (!m_chkUseOcio->isChecked() || !m_ocioConfig) return false;

    const bool externalColorManagementEnabled =
        m_colorManagement->currentIndex() != (int)KisOcioConfiguration::INTERNAL;

    const bool exposureManagementEnabled =
        externalColorManagementEnabled
#ifdef HAVE_HDR
            || KisOpenGLModeProber::instance()->surfaceformatInUse().colorSpace() == QSurfaceFormat::scRGBColorSpace
#endif
            ;

    return exposureManagementEnabled;
}

qreal LutDockerDock::currentExposure() const
{
    if (!m_displayFilter) return 0.0;
    OcioDisplayFilter *displayFilter = qobject_cast<OcioDisplayFilter*>(m_displayFilter.data());
    return canChangeExposureAndGamma() ? displayFilter->exposure : 0.0;
}

void LutDockerDock::setCurrentExposure(qreal value)
{
    if (!canChangeExposureAndGamma()) return;
    m_exposureCompressor->start(value);
}

qreal LutDockerDock::currentGamma() const
{
    if (!m_displayFilter) return 1.0;
    OcioDisplayFilter *displayFilter = qobject_cast<OcioDisplayFilter*>(m_displayFilter.data());
    return canChangeExposureAndGamma() ? displayFilter->gamma : 1.0;
}

void LutDockerDock::setCurrentGamma(qreal value)
{
    if (!canChangeExposureAndGamma()) return;
    m_gammaCompressor->start(value);
}

void LutDockerDock::setCurrentExposureImpl(qreal value)
{
    m_exposureDoubleWidget->setValue(value);
    if (!m_canvas) return;

    m_canvas->viewManager()->showFloatingMessage(
        i18nc("floating message about exposure", "Exposure: %1",
              KritaUtils::prettyFormatReal(m_exposureDoubleWidget->value())),
        QIcon(), 500, KisFloatingMessage::Low);
}

void LutDockerDock::setCurrentGammaImpl(qreal value)
{
    m_gammaDoubleWidget->setValue(value);
    if (!m_canvas) return;

    m_canvas->viewManager()->showFloatingMessage(
        i18nc("floating message about gamma", "Gamma: %1",
              KritaUtils::prettyFormatReal(m_gammaDoubleWidget->value())),
        QIcon(), 500, KisFloatingMessage::Low);
}

void LutDockerDock::slotImageColorSpaceChanged()
{
    enableControls();
    writeControls();
    resetOcioConfiguration();
}

void LutDockerDock::exposureValueChanged(double exposure)
{
    if (m_canvas && !m_draggingSlider) {
        m_canvas->viewManager()->canvasResourceProvider()->setHDRExposure(exposure);
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
    if (m_canvas && !m_draggingSlider) {
        m_canvas->viewManager()->canvasResourceProvider()->setHDRGamma(gamma);
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

void LutDockerDock::enableControls()
{
    bool canDoExternalColorCorrection = false;

    if (m_canvas) {
        KisImageSP image = m_canvas->viewManager()->image();
        canDoExternalColorCorrection =
            image->colorSpace()->colorModelId() == RGBAColorModelID;
    }

    if (!canDoExternalColorCorrection) {
        KisSignalsBlocker colorManagementBlocker(m_colorManagement);
        Q_UNUSED(colorManagementBlocker);
        m_colorManagement->setCurrentIndex((int) KisOcioConfiguration::INTERNAL);
    }

    const bool ocioEnabled = m_chkUseOcio->isChecked();
    m_colorManagement->setEnabled(ocioEnabled && canDoExternalColorCorrection);

    const bool externalColorManagementEnabled =
        m_colorManagement->currentIndex() != (int)KisOcioConfiguration::INTERNAL;

    m_lblInputColorSpace->setEnabled(ocioEnabled && externalColorManagementEnabled);
    m_cmbInputColorSpace->setEnabled(ocioEnabled && externalColorManagementEnabled);
    m_lblDisplayDevice->setEnabled(ocioEnabled && externalColorManagementEnabled);
    m_cmbDisplayDevice->setEnabled(ocioEnabled && externalColorManagementEnabled);
    m_lblView->setEnabled(ocioEnabled && externalColorManagementEnabled);
    m_cmbView->setEnabled(ocioEnabled && externalColorManagementEnabled);
    m_lblLook->setEnabled(ocioEnabled && externalColorManagementEnabled);
    m_cmbLook->setEnabled(ocioEnabled && externalColorManagementEnabled);

    const bool exposureManagementEnabled = canChangeExposureAndGamma();

    m_exposureDoubleWidget->setEnabled(exposureManagementEnabled);
    m_gammaDoubleWidget->setEnabled(exposureManagementEnabled);
    m_lblExposure->setEnabled(exposureManagementEnabled);
    m_lblGamma->setEnabled(exposureManagementEnabled);

    QString exposureToolTip;

    if (!exposureManagementEnabled) {
        exposureToolTip = i18nc("@info:tooltip", "Exposure and Gamma corrections are disabled in Internal mode. Switch to OCIO mode to use them");
    }
    m_exposureDoubleWidget->setToolTip(exposureToolTip);
    m_gammaDoubleWidget->setToolTip(exposureToolTip);
    m_lblExposure->setToolTip(exposureToolTip);
    m_lblGamma->setToolTip(exposureToolTip);

    bool enableConfigPath = m_colorManagement->currentIndex() == (int) KisOcioConfiguration::OCIO_CONFIG;

    lblConfig->setEnabled(ocioEnabled && enableConfigPath);
    m_txtConfigurationPath->setEnabled(ocioEnabled && enableConfigPath);
    m_bnSelectConfigurationFile->setEnabled(ocioEnabled && enableConfigPath);
}

void LutDockerDock::updateDisplaySettings()
{
    if (!m_canvas || !m_canvas->viewManager() || !m_canvas->viewManager()->image()) {
        return;
    }

    enableControls();
    writeControls();

    if (m_chkUseOcio->isChecked() && m_ocioConfig) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_canvas->displayFilter() ||
                                     m_canvas->displayFilter() == m_displayFilter);

        if (!m_displayFilter) {
            m_displayFilter =
                m_canvas->displayFilter() ?
                    m_canvas->displayFilter() :
                    QSharedPointer<KisDisplayFilter>(new OcioDisplayFilter(this));
        }

        OcioDisplayFilter *displayFilter = qobject_cast<OcioDisplayFilter*>(m_displayFilter.data());
        displayFilter->config = m_ocioConfig;
        displayFilter->inputColorSpaceName = m_ocioConfig->getColorSpaceNameByIndex(m_cmbInputColorSpace->currentIndex());
        displayFilter->displayDevice = m_ocioConfig->getDisplay(m_cmbDisplayDevice->currentIndex());
        displayFilter->view = m_ocioConfig->getView(displayFilter->displayDevice, m_cmbView->currentIndex());
        displayFilter->look = m_ocioConfig->getLookNameByIndex(m_cmbLook->currentIndex());
        displayFilter->gamma = m_gammaDoubleWidget->isEnabled() ? m_gammaDoubleWidget->value() : 1.0;
        displayFilter->exposure = m_exposureDoubleWidget->isEnabled() ? m_exposureDoubleWidget->value() : 0.0;
        displayFilter->swizzle = (OCIO_CHANNEL_SWIZZLE)m_cmbComponents->currentIndex();

        displayFilter->blackPoint = m_bwPointChooser->blackPoint();
        displayFilter->whitePoint = m_bwPointChooser->whitePoint();

        displayFilter->forceInternalColorManagement =
            m_colorManagement->currentIndex() == (int)KisOcioConfiguration::INTERNAL;

        displayFilter->setLockCurrentColorVisualRepresentation(m_btnConvertCurrentColor->isChecked());

        displayFilter->updateProcessor();
        m_canvas->setDisplayFilter(m_displayFilter);
    }
    else {
        m_canvas->setDisplayFilter(QSharedPointer<KisDisplayFilter>(0));
    }
    m_canvas->updateCanvas();
}

void LutDockerDock::writeControls()
{
    KisOcioConfiguration ocioOptions;
    ocioOptions.mode = (KisOcioConfiguration::Mode)m_colorManagement->currentIndex();
    ocioOptions.configurationPath = m_txtConfigurationPath->text();
    ocioOptions.lutPath = m_txtLut->text();
    ocioOptions.inputColorSpace = m_cmbInputColorSpace->itemHighlighted();
    ocioOptions.displayDevice = m_cmbDisplayDevice->itemHighlighted();
    ocioOptions.displayView = m_cmbView->itemHighlighted();
    ocioOptions.look = m_cmbLook->itemHighlighted();

    KisConfig cfg(false);
    cfg.setUseOcio(m_chkUseOcio->isChecked());
    cfg.setOcioConfiguration(ocioOptions);
    cfg.setOcioLockColorVisualRepresentation(m_btnConvertCurrentColor->isChecked());
}

void LutDockerDock::slotColorManagementModeChanged()
{
    enableControls();
    writeControls();
    resetOcioConfiguration();
}

void LutDockerDock::selectOcioConfiguration()
{
    QString filename = m_txtConfigurationPath->text();

    KoFileDialog dialog(this, KoFileDialog::OpenFile, "lutdocker");
    dialog.setCaption(i18n("Select OpenColorIO Configuration"));
    dialog.setDefaultDir(QDir::cleanPath(filename));
    dialog.setMimeTypeFilters(QStringList() << "application/x-opencolorio-configuration");
    filename = dialog.filename();
    QFile f(filename);
    if (f.exists()) {
        m_txtConfigurationPath->setText(filename);
        writeControls();
        resetOcioConfiguration();
    }
}

void LutDockerDock::resetOcioConfiguration()
{
    KisConfig cfg(true);
    KisOcioConfiguration ocioOptions = cfg.ocioConfiguration();
    m_ocioConfig.reset();

    try {
        if (ocioOptions.mode == KisOcioConfiguration::INTERNAL) {
            m_ocioConfig = defaultRawProfile();
        } else if (ocioOptions.mode == KisOcioConfiguration::OCIO_ENVIRONMENT) {
            m_ocioConfig = OCIO::Config::CreateFromEnv();
        }
        else if (ocioOptions.mode == KisOcioConfiguration::OCIO_CONFIG) {
            QString configFile = ocioOptions.configurationPath;

            if (QFile::exists(configFile)) {
                m_ocioConfig = OCIO::Config::CreateFromFile(configFile.toUtf8());
            } else {
                m_ocioConfig = defaultRawProfile();
            }
        }
        if (m_ocioConfig) {
            OCIO::SetCurrentConfig(m_ocioConfig);
        }
    }
    catch (OCIO::Exception &exception) {
        dbgKrita << "OpenColorIO Error:" << exception.what() << "Cannot create the LUT docker";
    }


    if (m_ocioConfig) {
        refillControls();
    }
}

void LutDockerDock::refillControls()
{
    if (!m_canvas) return;
    if (!m_canvas->viewManager()) return;
    if (!m_canvas->viewManager()->canvasResourceProvider()) return;
    if (!m_canvas->viewManager()->image()) return;

    KIS_ASSERT_RECOVER_RETURN(m_ocioConfig);

    KisConfig cfg(true);
    KisOcioConfiguration ocioOptions = cfg.ocioConfiguration();

    { // Color Management Mode
        KisSignalsBlocker modeBlocker(m_colorManagement);
        m_colorManagement->setCurrentIndex((int) ocioOptions.mode);
    }

    { // Exposure
        KisSignalsBlocker exposureBlocker(m_exposureDoubleWidget);
        m_exposureDoubleWidget->setValue(m_canvas->viewManager()->canvasResourceProvider()->HDRExposure());
    }

    { // Gamma
        KisSignalsBlocker gammaBlocker(m_gammaDoubleWidget);
        m_gammaDoubleWidget->setValue(m_canvas->viewManager()->canvasResourceProvider()->HDRGamma());
    }

    { // Components
        const KoColorSpace *cs = m_canvas->viewManager()->image()->colorSpace();

        QStringList itemsList;
        itemsList << i18n("Luminance");
        itemsList << i18n("All Channels");
        Q_FOREACH (KoChannelInfo *channel, KoChannelInfo::displayOrderSorted(cs->channels())) {
            itemsList << channel->name();
        }

        if (m_cmbComponents->originalTexts() != itemsList) {
            KisSignalsBlocker componentsBlocker(m_cmbComponents);
            m_cmbComponents->resetOriginalTexts(itemsList);
            m_cmbComponents->setCurrentIndex(1); // All Channels...
        }
    }

    { // Input Color Space
        QStringList itemsList;
        int numOcioColorSpaces = m_ocioConfig->getNumColorSpaces();
        for(int i = 0; i < numOcioColorSpaces; ++i) {
            const char *cs = m_ocioConfig->getColorSpaceNameByIndex(i);
            OCIO::ConstColorSpaceRcPtr colorSpace = m_ocioConfig->getColorSpace(cs);
            itemsList << QString::fromUtf8(colorSpace->getName());
        }

        KisSignalsBlocker inputCSBlocker(m_cmbInputColorSpace);

        if (itemsList != m_cmbInputColorSpace->originalTexts()) {
            m_cmbInputColorSpace->resetOriginalTexts(itemsList);
        }

        m_cmbInputColorSpace->setCurrent(ocioOptions.inputColorSpace);
    }

    { // Display Device
        QStringList itemsList;
        int numDisplays = m_ocioConfig->getNumDisplays();
        for (int i = 0; i < numDisplays; ++i) {
            itemsList << QString::fromUtf8(m_ocioConfig->getDisplay(i));
        }

        KisSignalsBlocker displayDeviceLocker(m_cmbDisplayDevice);

        if (itemsList != m_cmbDisplayDevice->originalTexts()) {
            m_cmbDisplayDevice->resetOriginalTexts(itemsList);
        }

        m_cmbDisplayDevice->setCurrent(ocioOptions.displayDevice);
    }

    { // Lock Current Color
        KisSignalsBlocker locker(m_btnConvertCurrentColor);
        m_btnConvertCurrentColor->setChecked(cfg.ocioLockColorVisualRepresentation());
    }

    refillViewCombobox();

    {
        QStringList itemsList;
        int numLooks = m_ocioConfig->getNumLooks();
        for (int k = 0; k < numLooks; k++) {
           itemsList << QString::fromUtf8(m_ocioConfig->getLookNameByIndex(k));
        }
        itemsList << i18nc("Item to indicate no look transform being selected","None");

        KisSignalsBlocker LookComboLocker(m_cmbLook);

        if (itemsList != m_cmbLook->originalTexts()) {
            m_cmbLook->resetOriginalTexts(itemsList);
        }

        m_cmbLook->setCurrent(ocioOptions.look);
    }
    updateDisplaySettings();
}

void LutDockerDock::refillViewCombobox()
{
    KisSignalsBlocker viewComboLocker(m_cmbView);
    m_cmbView->clear();

    if (!m_canvas || !m_ocioConfig) return;

    const char *display = m_ocioConfig->getDisplay(m_cmbDisplayDevice->currentIndex());
    int numViews = m_ocioConfig->getNumViews(display);

    for (int j = 0; j < numViews; ++j) {
        m_cmbView->addSqueezedItem(QString::fromUtf8(m_ocioConfig->getView(display, j)));
    }

    KisConfig cfg(true);
    KisOcioConfiguration ocioOptions = cfg.ocioConfiguration();
    m_cmbView->setCurrent(ocioOptions.displayView);
}

void LutDockerDock::selectLut()
{
    QString filename = m_txtLut->text();

    KoFileDialog dialog(this, KoFileDialog::OpenFile, "lutdocker");
    dialog.setCaption(i18n("Select LUT file"));
    dialog.setDefaultDir(QDir::cleanPath(filename));
    dialog.setMimeTypeFilters(QStringList() << "application/octet-stream", "application/octet-stream");
    filename = dialog.filename();

    QFile f(filename);
    if (f.exists() && filename != m_txtLut->text()) {
        m_txtLut->setText(filename);
        writeControls();
        updateDisplaySettings();
    }
}

void LutDockerDock::clearLut()
{
    m_txtLut->clear();
    updateDisplaySettings();
}

