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

#include "kis_double_parse_unit_spin_box.h"
#include "kis_document_aware_spin_box_unit_manager.h"

static const QString pixelStr(KoUnit::unitDescription(KoUnit::Pixel));
static const QString percentStr(i18n("Percent (%)"));
static const QString pixelsInchStr(i18n("Pixels/Inch"));
static const QString pixelsCentimeterStr(i18n("Pixels/Centimeter"));

DlgImageSize::DlgImageSize(QWidget *parent, int width, int height, double resolution)
    : KoDialog(parent)
    , m_aspectRatio(((double) width) / height)
    , m_originalWidth(width)
    , m_originalHeight(height)
    , m_width(width)
    , m_height(height)
    , m_printWidth(width / resolution)
    , m_printHeight(height / resolution)
    , m_originalResolution(resolution)
    , m_resolution(resolution)
    , m_keepAspect(true)
{
    setCaption(i18n("Scale To New Size"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgImageSize(this);

    Q_CHECK_PTR(m_page);
    m_page->layout()->setMargin(0);
    m_page->setObjectName("image_size");

    _widthUnitManager = new KisDocumentAwareSpinBoxUnitManager(this);
    _heightUnitManager = new KisDocumentAwareSpinBoxUnitManager(this, KisDocumentAwareSpinBoxUnitManager::PIX_DIR_Y);

    //configure the unit to image length, default unit is pixel and printing units are forbiden.
    _widthUnitManager->setUnitDimension(KisSpinBoxUnitManager::IMLENGTH);
    _heightUnitManager->setUnitDimension(KisSpinBoxUnitManager::IMLENGTH);

    m_page->pixelWidthDouble->setUnitManager(_widthUnitManager);
    m_page->pixelHeightDouble->setUnitManager(_heightUnitManager);

    m_page->pixelWidthDouble->changeValue(width);
    m_page->pixelHeightDouble->changeValue(height);
    m_page->pixelWidthDouble->setDecimals(2);
    m_page->pixelHeightDouble->setDecimals(2);
    m_page->pixelWidthDouble->setDisplayUnit(false);
    m_page->pixelHeightDouble->setDisplayUnit(false);

    m_page->pixelWidthUnit->setModel(_widthUnitManager);
    m_page->pixelHeightUnit->setModel(_widthUnitManager);
    m_page->pixelWidthUnit->setCurrentText("px");
    m_page->pixelHeightUnit->setCurrentText("px");

    m_page->pixelFilterCmb->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->pixelFilterCmb->setToolTip(KisFilterStrategyRegistry::instance()->formattedDescriptions());
    m_page->pixelFilterCmb->setCurrent("Bicubic");

    _printWidthUnitManager = new KisSpinBoxUnitManager(this);
    _printHeightUnitManager = new KisSpinBoxUnitManager(this);

    m_page->printWidth->setUnitManager(_printWidthUnitManager);
    m_page->printHeight->setUnitManager(_printHeightUnitManager);
    m_page->printWidth->setDecimals(2);
    m_page->printHeight->setDecimals(2);
    m_page->printWidth->setDisplayUnit(false);
    m_page->printHeight->setDisplayUnit(false);
    m_page->printResolution->setDecimals(2);
    m_page->printResolution->setAlignment(Qt::AlignRight);

    m_page->printWidthUnit->setModel(_printWidthUnitManager);
    m_page->printHeightUnit->setModel(_printHeightUnitManager);

    m_page->printWidth->changeValue(m_printWidth);
    m_page->printHeight->changeValue(m_printHeight);

    //TODO: create a resolution dimension in the unit manager.
    m_page->printResolutionUnit->addItem(pixelsInchStr);
    m_page->printResolutionUnit->addItem(pixelsCentimeterStr);

    m_page->pixelAspectRatioBtn->setKeepAspectRatio(true);
    m_page->printAspectRatioBtn->setKeepAspectRatio(true);
    m_page->constrainProportionsCkb->setChecked(true);

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
    comboboxesGroup->addWidget(m_page->pixelWidthUnit);
    comboboxesGroup->addWidget(m_page->pixelHeightUnit);
    comboboxesGroup->addWidget(m_page->printWidthUnit);
    comboboxesGroup->addWidget(m_page->printHeightUnit);
    comboboxesGroup->addWidget(m_page->printResolutionUnit);
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    connect(m_page->pixelAspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->printAspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->constrainProportionsCkb, SIGNAL(toggled(bool)), this, SLOT(slotAspectChanged(bool)));

    connect(m_page->pixelWidthDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotPixelWidthChanged(double)));
    connect(m_page->pixelHeightDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotPixelHeightChanged(double)));
    connect(m_page->pixelWidthUnit, SIGNAL(currentIndexChanged(int)), _widthUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(m_page->pixelHeightUnit, SIGNAL(currentIndexChanged(int)), _heightUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(_widthUnitManager, SIGNAL(unitChanged(int)), m_page->pixelWidthUnit, SLOT(setCurrentIndex(int)));
    connect(_heightUnitManager, SIGNAL(unitChanged(int)), m_page->pixelHeightUnit, SLOT(setCurrentIndex(int)));

    connect(m_page->printWidth, SIGNAL(valueChangedPt(double)), this, SLOT(slotPrintWidthChanged(double)));
    connect(m_page->printHeight, SIGNAL(valueChangedPt(double)), this, SLOT(slotPrintHeightChanged(double)));
    connect(m_page->printWidthUnit, SIGNAL(currentIndexChanged(int)), _printWidthUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(m_page->printHeightUnit, SIGNAL(currentIndexChanged(int)), _printHeightUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(_printWidthUnitManager, SIGNAL(unitChanged(int)), m_page->printWidthUnit, SLOT(setCurrentIndex(int)));
    connect(_printHeightUnitManager, SIGNAL(unitChanged(int)), m_page->printHeightUnit, SLOT(setCurrentIndex(int)));

    connect(m_page->printResolution, SIGNAL(valueChanged(double)), this, SLOT(slotPrintResolutionChanged(double)));
    connect(m_page->printResolution, SIGNAL(editingFinished()), this, SLOT(slotPrintResolutionEditFinished()));
    connect(m_page->printResolutionUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPrintResolutionUnitChanged()));

    // pick selected print units from user locale (after slots connection, so the spinbox will be updated too).
    if (QLocale().measurementSystem() == QLocale::MetricSystem) {
        m_page->printWidthUnit->setCurrentText("cm");
        m_page->printHeightUnit->setCurrentText("cm");
        m_page->printResolutionUnit->setCurrentIndex(0); // Pixels/Centimeter
        slotPrintResolutionUnitChanged(); //ensure the resolution is updated, even if the index didn't changed.
    } else { // Imperial
        m_page->printWidthUnit->setCurrentText("in");
        m_page->printHeightUnit->setCurrentText("in");
        m_page->printResolutionUnit->setCurrentIndex(1); // Pixels/Inch
        slotPrintResolutionUnitChanged(); //ensure the resolution is updated, even if the index didn't changed.
    }

    setMainWidget(m_page);
}

DlgImageSize::~DlgImageSize()
{
    delete m_page;
}

qint32 DlgImageSize::width()
{
    return (qint32)m_width;
}

qint32 DlgImageSize::height()
{
    return (qint32)m_height;
}

double DlgImageSize::resolution()
{
    return m_resolution;
}

KisFilterStrategy *DlgImageSize::filterType()
{
    KoID filterID = m_page->pixelFilterCmb->currentItem();
    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value(filterID.id());
    return filter;
}

// SLOTS

void DlgImageSize::slotPixelWidthChanged(double w)
{
    m_width = w;

    m_printWidth = m_width / m_resolution;
    updatePrintWidthUIValue(m_printWidth);

    if (m_keepAspect) {
        m_height = qRound(m_width / m_aspectRatio);
        updatePixelHeightUIValue(m_height);

        m_printHeight = m_height / m_resolution;
        updatePrintHeightUIValue(m_printHeight);
    }
}

void DlgImageSize::slotPixelHeightChanged(double h)
{
    m_height = h;

    m_printHeight = m_height / m_resolution;
    updatePrintHeightUIValue(m_printHeight);

    if (m_keepAspect) {
        m_width = qRound(m_height * m_aspectRatio);
        updatePixelWidthUIValue(m_width);

        m_printWidth = m_width / m_resolution;
        updatePrintWidthUIValue(m_printWidth);
    }
}

void DlgImageSize::slotPrintWidthChanged(double w)
{
    m_printWidth = w;

    if (m_keepAspect) {
        m_printHeight = m_printWidth / m_aspectRatio;
        updatePrintHeightUIValue(m_printHeight);
    }

    if (m_page->adjustPrintSizeSeparatelyCkb->isChecked()) {
        m_resolution = m_width / m_printWidth;
        updatePrintResolutionUIValue(m_resolution);

        if (!m_keepAspect) {
            // compute and update a new image height value from the print size values
            const double printHeightInch = KoUnit::convertFromUnitToUnit(m_printHeight, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));
            m_height = qRound(printHeightInch * 72 * m_resolution);
            updatePixelHeightUIValue(m_height);
        }
    } else {
        const double printWidthInch = KoUnit::convertFromUnitToUnit(m_printWidth, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));
        m_width = qRound(printWidthInch * 72 * m_resolution);
        updatePixelWidthUIValue(m_width);

        if (m_keepAspect) {
            m_height = qRound(m_width / m_aspectRatio);
            updatePixelHeightUIValue(m_height);
        }
    }
}

void DlgImageSize::slotPrintHeightChanged(double h)
{
    m_printHeight = h;

    if (m_keepAspect) {
        m_printWidth = m_printHeight * m_aspectRatio;
        updatePrintWidthUIValue(m_printWidth);
    }

    if (m_page->adjustPrintSizeSeparatelyCkb->isChecked()) {
        m_resolution = m_height / m_printHeight;
        updatePrintResolutionUIValue(m_resolution);

        if (!m_keepAspect) {
            // compute and update a new image width value from the print size values
            const double printWidthInch = KoUnit::convertFromUnitToUnit(m_printWidth, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));
            m_width = qRound(printWidthInch * 72 * m_resolution);
            updatePixelWidthUIValue(m_width);
        }
    } else {
        const double printHeightInch = KoUnit::convertFromUnitToUnit(m_printHeight, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));
        m_height = qRound(printHeightInch * 72 * m_resolution);
        updatePixelHeightUIValue(m_height);

        if (m_keepAspect) {
            m_width = qRound(m_height * m_aspectRatio);
            updatePixelWidthUIValue(m_width);
        }
    }
}

void DlgImageSize::slotAspectChanged(bool keep)
{
    m_page->pixelAspectRatioBtn->blockSignals(true);
    m_page->printAspectRatioBtn->blockSignals(true);
    m_page->constrainProportionsCkb->blockSignals(true);

    m_page->pixelAspectRatioBtn->setKeepAspectRatio(keep);
    m_page->printAspectRatioBtn->setKeepAspectRatio(keep);
    m_page->constrainProportionsCkb->setChecked(keep);

    m_page->pixelAspectRatioBtn->blockSignals(false);
    m_page->printAspectRatioBtn->blockSignals(false);
    m_page->constrainProportionsCkb->blockSignals(false);

    m_keepAspect = keep;

    if (keep) {
        // values may be out of sync, so we need to reset it to defaults
        m_width = m_originalWidth;
        m_height = m_originalHeight;
        m_printWidth = m_originalWidth / m_originalResolution;
        m_printHeight = m_originalHeight / m_originalResolution;
        m_resolution = m_originalResolution;

        updatePixelWidthUIValue(m_width);
        updatePixelHeightUIValue(m_height);
        updatePrintWidthUIValue(m_printWidth);
        updatePrintHeightUIValue(m_printHeight);
        updatePrintResolutionUIValue(m_resolution);
    }
}

void DlgImageSize::slotPrintResolutionChanged(double r)
{
    if (m_page->printResolutionUnit->currentText() == pixelsInchStr)
        m_resolution = KoUnit::convertFromUnitToUnit(r, KoUnit(KoUnit::Pixel), KoUnit(KoUnit::Inch));
    else
        m_resolution = KoUnit::convertFromUnitToUnit(r, KoUnit(KoUnit::Pixel), KoUnit(KoUnit::Centimeter));

    if (m_page->adjustPrintSizeSeparatelyCkb->isChecked()) {
        m_printWidth = m_width / m_resolution;
        m_printHeight = m_height / m_resolution;

        updatePrintWidthUIValue(m_printWidth);
        updatePrintHeightUIValue(m_printHeight);
    } else {
        // Do not commit m_width and m_height values yet. This is done to avoid
        // nasty results in image size values while the user is typing a resolution value
        const double printWidthInch = KoUnit::convertFromUnitToUnit(m_printWidth, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));
        const int width = qRound(printWidthInch * 72 * m_resolution);
        const double printHeightInch = KoUnit::convertFromUnitToUnit(m_printHeight, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));
        const int height = qRound(printHeightInch * 72 * m_resolution);

        updatePixelWidthUIValue(width);
        updatePixelHeightUIValue(height);
    }
}

void DlgImageSize::slotPrintResolutionEditFinished()
{
    if (!m_page->adjustPrintSizeSeparatelyCkb->isChecked()) {
        const double printWidthInch = KoUnit::convertFromUnitToUnit(m_printWidth, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));
        const double printHeightInch = KoUnit::convertFromUnitToUnit(m_printHeight, KoUnit(KoUnit::Point), KoUnit(KoUnit::Inch));

        // Commit width and height values
        m_width = qRound(printWidthInch * 72 * m_resolution);
        m_height = qRound(printHeightInch * 72 * m_resolution);

        // Note that spinbox values should be up to date
        // (updated through slotResolutionChanged())
    }
}

void DlgImageSize::slotPrintResolutionUnitChanged()
{
    updatePrintResolutionUIValue(m_resolution);
}

void DlgImageSize::updatePixelWidthUIValue(double value)
{
    m_page->pixelWidthDouble->blockSignals(true);
    m_page->pixelWidthDouble->changeValue(value);
    m_page->pixelWidthDouble->blockSignals(false);
}

void DlgImageSize::updatePixelHeightUIValue(double value)
{
    m_page->pixelHeightDouble->blockSignals(true);
    m_page->pixelHeightDouble->changeValue(value);
    m_page->pixelHeightDouble->blockSignals(false);
}

void DlgImageSize::updatePrintWidthUIValue(double value)
{
    m_page->printWidth->blockSignals(true);
    m_page->printWidth->changeValue(value);
    m_page->printWidth->blockSignals(false);
}

void DlgImageSize::updatePrintHeightUIValue(double value)
{
    m_page->printHeight->blockSignals(true);
    m_page->printHeight->changeValue(value);
    m_page->printHeight->blockSignals(false);
}

void DlgImageSize::updatePrintResolutionUIValue(double value)
{
    double uiValue = 0.0;
    if (m_page->printResolutionUnit->currentText() == pixelsInchStr) {
        // show the value in pixel/inch unit
        uiValue = KoUnit::convertFromUnitToUnit(value, KoUnit(KoUnit::Inch), KoUnit(KoUnit::Pixel));
    } else {
        // show the value in pixel/centimeter unit
        uiValue = KoUnit::convertFromUnitToUnit(value, KoUnit(KoUnit::Centimeter), KoUnit(KoUnit::Pixel));
    }

    m_page->printResolution->blockSignals(true);
    m_page->printResolution->setValue(uiValue);
    m_page->printResolution->blockSignals(false);
}

