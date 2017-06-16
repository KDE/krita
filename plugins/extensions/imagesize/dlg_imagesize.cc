/*
 *  dlg_imagesize.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2009 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2013 Juan Palacios <jpalaciosdev@gmail.com>
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

#include "dlg_imagesize.h"

#include <QLocale>

#include <KoUnit.h>
#include <kis_size_group.h>
#include <klocalizedstring.h>

#include <kis_filter_strategy.h>

#include "kis_aspect_ratio_locker.h"
#include "kis_acyclic_signal_connector.h"
#include "kis_signals_blocker.h"

#include "kis_double_parse_unit_spin_box.h"
#include "kis_document_aware_spin_box_unit_manager.h"

static const int maxImagePixelSize = 10000;

static const QString pixelStr(KoUnit::unitDescription(KoUnit::Pixel));
static const QString percentStr(i18n("Percent (%)"));
static const QString pixelsInchStr(i18n("Pixels/Inch"));
static const QString pixelsCentimeterStr(i18n("Pixels/Centimeter"));

DlgImageSize::DlgImageSize(QWidget *parent, int width, int height, double resolution)
    : KoDialog(parent)
{
    setCaption(i18n("Scale To New Size"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgImageSize(this);

    Q_CHECK_PTR(m_page);
    m_page->layout()->setMargin(0);
    m_page->setObjectName("image_size");

    m_page->pixelFilterCmb->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->pixelFilterCmb->setToolTip(KisFilterStrategyRegistry::instance()->formattedDescriptions());
    m_page->pixelFilterCmb->setCurrent("Bicubic");


    /**
     * Initialize Pixel Width and Height fields
     */

    m_widthUnitManager = new KisDocumentAwareSpinBoxUnitManager(this);
    m_heightUnitManager = new KisDocumentAwareSpinBoxUnitManager(this, KisDocumentAwareSpinBoxUnitManager::PIX_DIR_Y);

    /// configure the unit to image length, default unit is pixel and printing units are forbiden.
    m_widthUnitManager->setUnitDimension(KisSpinBoxUnitManager::IMLENGTH);
    m_heightUnitManager->setUnitDimension(KisSpinBoxUnitManager::IMLENGTH);

    m_page->pixelWidthDouble->setUnitManager(m_widthUnitManager);
    m_page->pixelHeightDouble->setUnitManager(m_heightUnitManager);
    m_page->pixelWidthDouble->changeValue(width);
    m_page->pixelHeightDouble->changeValue(height);
    m_page->pixelWidthDouble->setDisplayUnit(false);
    m_page->pixelHeightDouble->setDisplayUnit(false);

    /// add custom units
    m_page->pixelSizeUnit->addItem(pixelStr);
    m_page->pixelSizeUnit->addItem(percentStr);
    m_page->pixelSizeUnit->setCurrentText(pixelStr);

    /**
     * Initialize Print Width, Height and Resolution fields
     */

    m_printSizeUnitManager = new KisSpinBoxUnitManager(this);

    m_page->printWidth->setUnitManager(m_printSizeUnitManager);
    m_page->printHeight->setUnitManager(m_printSizeUnitManager);
    m_page->printWidth->setDecimals(2);
    m_page->printHeight->setDecimals(2);
    m_page->printWidth->setDisplayUnit(false);
    m_page->printHeight->setDisplayUnit(false);
    m_page->printResolution->setDecimals(2);
    m_page->printResolution->setAlignment(Qt::AlignRight);

    m_page->printWidthUnit->setModel(m_printSizeUnitManager);

    //TODO: create a resolution dimension in the unit manager.
    m_page->printResolutionUnit->addItem(pixelsInchStr);
    m_page->printResolutionUnit->addItem(pixelsCentimeterStr);


    /**
     * Initialize labels and layout
     */
    KisSizeGroup *labelsGroup = new KisSizeGroup(this);
    labelsGroup->addWidget(m_page->lblPixelWidth);
    labelsGroup->addWidget(m_page->lblPixelHeight);
    labelsGroup->addWidget(m_page->lblPixelFilter);
    labelsGroup->addWidget(m_page->lblPrintWidth);
    labelsGroup->addWidget(m_page->lblPrintHeight);
    labelsGroup->addWidget(m_page->lblResolution);

    KisSizeGroup *spinboxesGroup = new KisSizeGroup(this);
    spinboxesGroup->addWidget(m_page->pixelWidthDouble);
    spinboxesGroup->addWidget(m_page->pixelHeightDouble);
    spinboxesGroup->addWidget(m_page->printWidth);
    spinboxesGroup->addWidget(m_page->printHeight);
    spinboxesGroup->addWidget(m_page->printResolution);

    KisSizeGroup *comboboxesGroup = new KisSizeGroup(this);
    comboboxesGroup->addWidget(m_page->pixelSizeUnit);
    comboboxesGroup->addWidget(m_page->printWidthUnit);
    comboboxesGroup->addWidget(m_page->printResolutionUnit);
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    /**
     * Initialize aspect ratio buttons and lockers
     */

    m_page->pixelAspectRatioBtn->setKeepAspectRatio(true);
    m_page->printAspectRatioBtn->setKeepAspectRatio(true);
    m_page->constrainProportionsCkb->setChecked(true);

    m_pixelSizeLocker = new KisAspectRatioLocker(this);
    m_pixelSizeLocker->connectSpinBoxes(m_page->pixelWidthDouble, m_page->pixelHeightDouble, m_page->pixelAspectRatioBtn);

    m_printSizeLocker = new KisAspectRatioLocker(this);
    m_printSizeLocker->connectSpinBoxes(m_page->printWidth, m_page->printHeight, m_page->printAspectRatioBtn);

    /**
     * Connect Keep Aspect Lock buttons
     */

    KisAcyclicSignalConnector *constrainsConnector = new KisAcyclicSignalConnector(this);
    constrainsConnector->connectBackwardBool(
        m_page->constrainProportionsCkb, SIGNAL(toggled(bool)),
        this, SLOT(slotLockAllRatioSwitched(bool)));

    constrainsConnector->connectForwardBool(
        m_pixelSizeLocker, SIGNAL(aspectButtonToggled(bool)),
        this, SLOT(slotLockPixelRatioSwitched(bool)));

    constrainsConnector->createCoordinatedConnector()->connectBackwardBool(
        m_printSizeLocker, SIGNAL(aspectButtonToggled(bool)),
        this, SLOT(slotLockPrintRatioSwitched(bool)));

    constrainsConnector->createCoordinatedConnector()->connectBackwardBool(
        m_page->adjustPrintSizeSeparatelyCkb, SIGNAL(toggled(bool)),
        this, SLOT(slotAdjustSeparatelySwitched(bool)));

    /**
     * Connect Pixel Unit switching controls
     */

    KisAcyclicSignalConnector *pixelUnitConnector = new KisAcyclicSignalConnector(this);
    pixelUnitConnector->connectForwardInt(
        m_page->pixelSizeUnit, SIGNAL(currentIndexChanged(int)),
        this, SLOT(slotPixelUnitBoxChanged()));

    pixelUnitConnector->connectBackwardInt(
        m_widthUnitManager, SIGNAL(unitChanged(int)),
        this, SLOT(slotPixelWidthUnitSuffixChanged()));

    pixelUnitConnector->createCoordinatedConnector()->connectBackwardInt(
        m_heightUnitManager, SIGNAL(unitChanged(int)),
        this, SLOT(slotPixelHeightUnitSuffixChanged()));

    /**
     * Connect Print Unit switching controls
     */

    KisAcyclicSignalConnector *printUnitConnector = new KisAcyclicSignalConnector(this);
    printUnitConnector->connectForwardInt(
        m_page->printWidthUnit, SIGNAL(currentIndexChanged(int)),
        m_printSizeUnitManager, SLOT(selectApparentUnitFromIndex(int)));

    printUnitConnector->connectBackwardInt(
        m_printSizeUnitManager, SIGNAL(unitChanged(int)),
        m_page->printWidthUnit, SLOT(setCurrentIndex(int)));

    /// connect resolution
    connect(m_page->printResolutionUnit, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotPrintResolutionUnitChanged()));


    /**
     * Create syncing connections between Pixel and Print values
     */
    KisAcyclicSignalConnector *syncConnector = new KisAcyclicSignalConnector(this);
    syncConnector->connectForwardVoid(
        m_pixelSizeLocker, SIGNAL(sliderValueChanged()),
        this, SLOT(slotSyncPixelToPrintSize()));

    syncConnector->connectBackwardVoid(
        m_printSizeLocker, SIGNAL(sliderValueChanged()),
        this, SLOT(slotSyncPrintToPixelSize()));

    syncConnector->createCoordinatedConnector()->connectBackwardVoid(
        m_page->printResolution, SIGNAL(valueChanged(double)),
        this, SLOT(slotPrintResolutionChanged()));


    /**
     * Initialize printing values from the predefined image values
     */
    if (QLocale().measurementSystem() == QLocale::MetricSystem) {
        m_page->printWidthUnit->setCurrentText("cm");
    } else { // Imperial
        m_page->printWidthUnit->setCurrentText("in");
    }

    setCurrentResilutionPPI(resolution);
    slotSyncPixelToPrintSize();
    slotPixelUnitBoxChanged();

    /**
     * Initialize aspect ratio lockers with the current proportion.
     * Print locker gets the values only after the first call to slotSyncPixelToPrintSize().
     */
    m_pixelSizeLocker->updateAspect();
    m_printSizeLocker->updateAspect();

    setMainWidget(m_page);
}

DlgImageSize::~DlgImageSize()
{
    delete m_page;
}

qint32 DlgImageSize::width()
{
    return int(m_page->pixelWidthDouble->value());
}

qint32 DlgImageSize::height()
{
    return int(m_page->pixelHeightDouble->value());
}

double DlgImageSize::resolution()
{
    return currentResolutionPPI();
}

KisFilterStrategy *DlgImageSize::filterType()
{
    KoID filterID = m_page->pixelFilterCmb->currentItem();
    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value(filterID.id());
    return filter;
}

void DlgImageSize::slotSyncPrintToPixelSize()
{
    const bool printIsSeparate = m_page->adjustPrintSizeSeparatelyCkb->isChecked();

    if (!printIsSeparate) {
        KisSignalsBlocker b(m_page->pixelWidthDouble, m_page->pixelHeightDouble);
        m_page->pixelWidthDouble->changeValue(m_page->printWidth->value() * currentResolutionPPI());
        m_page->pixelHeightDouble->changeValue(m_page->printHeight->value() * currentResolutionPPI());
    } else if (m_page->pixelWidthDouble->value() != 0.0) {
        setCurrentResilutionPPI(m_page->pixelWidthDouble->value() / m_page->printWidth->value());
    }
}

void DlgImageSize::slotSyncPixelToPrintSize()
{
    const qreal resolution = currentResolutionPPI();
    if (resolution != 0.0) {
        KisSignalsBlocker b(m_page->printWidth, m_page->printHeight);
        m_page->printWidth->changeValue(m_page->pixelWidthDouble->value() / resolution);
        m_page->printHeight->changeValue(m_page->pixelHeightDouble->value() / resolution);
    }
}

void DlgImageSize::slotPrintResolutionChanged()
{
    const bool printIsSeparate = m_page->adjustPrintSizeSeparatelyCkb->isChecked();

    if (printIsSeparate) {
        slotSyncPixelToPrintSize();
    } else {
        slotSyncPrintToPixelSize();
    }

    updatePrintSizeMaximum();
}

void DlgImageSize::slotPrintResolutionUnitChanged()
{
    qreal resolution = m_page->printResolution->value();

    if (m_page->printResolutionUnit->currentText() == pixelsInchStr) {
        resolution = KoUnit::convertFromUnitToUnit(resolution, KoUnit(KoUnit::Inch), KoUnit(KoUnit::Centimeter));
    } else {
        resolution = KoUnit::convertFromUnitToUnit(resolution, KoUnit(KoUnit::Centimeter), KoUnit(KoUnit::Inch));
    }

    {
        KisSignalsBlocker b(m_page->printResolution);
        m_page->printResolution->setValue(resolution);
    }
}

void DlgImageSize::slotLockPixelRatioSwitched(bool value)
{
    const bool printIsSeparate = m_page->adjustPrintSizeSeparatelyCkb->isChecked();

    if (!printIsSeparate) {
        m_page->printAspectRatioBtn->setKeepAspectRatio(value);
    }
    m_page->constrainProportionsCkb->setChecked(value);
}

void DlgImageSize::slotLockPrintRatioSwitched(bool value)
{
    m_page->pixelAspectRatioBtn->setKeepAspectRatio(value);
    m_page->constrainProportionsCkb->setChecked(value);
}

void DlgImageSize::slotLockAllRatioSwitched(bool value)
{
    const bool printIsSeparate = m_page->adjustPrintSizeSeparatelyCkb->isChecked();

    m_page->pixelAspectRatioBtn->setKeepAspectRatio(value);

    if (!printIsSeparate) {
        m_page->printAspectRatioBtn->setKeepAspectRatio(value);
    }
}

void DlgImageSize::slotAdjustSeparatelySwitched(bool value)
{
    m_page->printAspectRatioBtn->setEnabled(!value);
    m_page->printAspectRatioBtn->setKeepAspectRatio(!value ? m_page->constrainProportionsCkb->isChecked() : true);
}

void DlgImageSize::slotPixelUnitBoxChanged()
{
    {
        KisSignalsBlocker b(m_page->pixelWidthDouble, m_page->pixelHeightDouble);

        if (m_page->pixelSizeUnit->currentText() == pixelStr) {
            // TODO: adjust single step as well

            m_page->pixelWidthDouble->setDecimals(0);
            m_page->pixelHeightDouble->setDecimals(0);
            m_page->pixelWidthDouble->setSingleStep(1);
            m_page->pixelHeightDouble->setSingleStep(1);
            m_widthUnitManager->setApparentUnitFromSymbol("px");
            m_heightUnitManager->setApparentUnitFromSymbol("px");
        } else {
            m_page->pixelWidthDouble->setDecimals(2);
            m_page->pixelHeightDouble->setDecimals(2);
            m_page->pixelWidthDouble->setSingleStep(0.01);
            m_page->pixelHeightDouble->setSingleStep(0.01);
            m_widthUnitManager->setApparentUnitFromSymbol("vw");
            m_heightUnitManager->setApparentUnitFromSymbol("vh");
        }
    }
}

void DlgImageSize::slotPixelWidthUnitSuffixChanged()
{
    m_page->pixelSizeUnit->setCurrentIndex(m_widthUnitManager->getApparentUnitSymbol() != "px");
    slotPixelUnitBoxChanged();
}

void DlgImageSize::slotPixelHeightUnitSuffixChanged()
{
    m_page->pixelSizeUnit->setCurrentIndex(m_heightUnitManager->getApparentUnitSymbol() != "px");
    slotPixelUnitBoxChanged();
}

qreal DlgImageSize::currentResolutionPPI() const
{
    qreal resolution = m_page->printResolution->value();

    if (m_page->printResolutionUnit->currentText() == pixelsInchStr) {
        resolution = KoUnit::convertFromUnitToUnit(resolution, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));
    } else {
        resolution = KoUnit::convertFromUnitToUnit(resolution, KoUnit(KoUnit::Point), KoUnit(KoUnit::Centimeter));
    }

    return resolution;
}

void DlgImageSize::setCurrentResilutionPPI(qreal value)
{
    qreal newValue = value;

    if (m_page->printResolutionUnit->currentText() == pixelsInchStr) {
        newValue = KoUnit::convertFromUnitToUnit(value, KoUnit(KoUnit::Inch), KoUnit(KoUnit::Point));
    } else {
        newValue = KoUnit::convertFromUnitToUnit(value, KoUnit(KoUnit::Centimeter), KoUnit(KoUnit::Point));
    }

    {
        KisSignalsBlocker b(m_page->printResolution);
        m_page->printResolution->setValue(newValue);
    }

    updatePrintSizeMaximum();
}

void DlgImageSize::updatePrintSizeMaximum()
{
    const qreal value = currentResolutionPPI();
    if (value == 0.0) return;

    m_page->printWidth->setMaximum(maxImagePixelSize / value);
    m_page->printHeight->setMaximum(maxImagePixelSize / value);
}
