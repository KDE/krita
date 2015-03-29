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

#include <KoUnit.h>
#include <KoSizeGroup.h>
#include <klocalizedstring.h>
#include <klocale.h>

#include <kis_filter_strategy.h>

static const QString pixelStr(KoUnit::unitDescription(KoUnit::Pixel));
static const QString percentStr(i18n("Percent (%)"));
static const QString pixelsInchStr(i18n("Pixels/Inch"));
static const QString pixelsCentimeterStr(i18n("Pixels/Centimeter"));

DlgImageSize::DlgImageSize(QWidget *parent, int width, int height, double resolution)
        : KDialog(parent)
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

    m_page->pixelWidth->setValue(width);
    m_page->pixelWidth->setFocus();
    m_page->pixelHeight->setValue(height);

    m_page->pixelWidthDouble->setVisible(false);
    m_page->pixelHeightDouble->setVisible(false);

    m_page->pixelFilterCmb->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->pixelFilterCmb->setToolTip(KisFilterStrategyRegistry::instance()->formatedDescriptions());
    m_page->pixelFilterCmb->setCurrent("Bicubic");

    m_page->pixelWidthUnit->addItem(pixelStr);
    m_page->pixelWidthUnit->addItem(percentStr);
    m_page->pixelWidthUnit->setCurrentIndex(0);

    m_page->pixelHeightUnit->addItem(pixelStr);
    m_page->pixelHeightUnit->addItem(percentStr);
    m_page->pixelHeightUnit->setCurrentIndex(0);

    m_page->printWidthUnit->addItems(KoUnit::listOfUnitNameForUi(KoUnit::HidePixel));
    m_page->printWidthUnit->addItem(percentStr);
    m_page->printHeightUnit->addItems(KoUnit::listOfUnitNameForUi(KoUnit::HidePixel));
    m_page->printHeightUnit->addItem(percentStr);

    m_page->printResolutionUnit->addItem(pixelsInchStr);
    m_page->printResolutionUnit->addItem(pixelsCentimeterStr);

    // pick selected print units from user locale
    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        const int unitIndex = KoUnit(KoUnit::Centimeter).indexInListForUi(KoUnit::HidePixel);
        m_page->printWidthUnit->setCurrentIndex(unitIndex);
        m_page->printHeightUnit->setCurrentIndex(unitIndex);
        m_page->printResolutionUnit->setCurrentIndex(0); // Pixels/Centimeter
    } else { // Imperial
        const int unitIndex = KoUnit(KoUnit::Inch).indexInListForUi(KoUnit::HidePixel);
        m_page->printWidthUnit->setCurrentIndex(unitIndex);
        m_page->printHeightUnit->setCurrentIndex(unitIndex);
        m_page->printResolutionUnit->setCurrentIndex(1); // Pixels/Inch
    }
    updatePrintWidthUIValue(m_printWidth);
    updatePrintHeightUIValue(m_printHeight);
    updatePrintResolutionUIValue(m_resolution);

    m_page->pixelAspectRatioBtn->setKeepAspectRatio(true);
    m_page->printAspectRatioBtn->setKeepAspectRatio(true);
    m_page->constrainProportionsCkb->setChecked(true);

    KoSizeGroup *labelsGroup = new KoSizeGroup(this);
    labelsGroup->addWidget(m_page->lblPixelWidth);
    labelsGroup->addWidget(m_page->lblPixelHeight);
    labelsGroup->addWidget(m_page->lblPixelFilter);
    labelsGroup->addWidget(m_page->lblPrintWidth);
    labelsGroup->addWidget(m_page->lblPrintHeight);
    labelsGroup->addWidget(m_page->lblResolution);

    KoSizeGroup *spinboxesGroup = new KoSizeGroup(this);
    spinboxesGroup->addWidget(m_page->pixelWidth);
    spinboxesGroup->addWidget(m_page->pixelWidthDouble);
    spinboxesGroup->addWidget(m_page->pixelHeight);
    spinboxesGroup->addWidget(m_page->pixelHeightDouble);
    spinboxesGroup->addWidget(m_page->printWidth);
    spinboxesGroup->addWidget(m_page->printHeight);
    spinboxesGroup->addWidget(m_page->printResolution);

    KoSizeGroup *comboboxesGroup = new KoSizeGroup(this);
    comboboxesGroup->addWidget(m_page->pixelWidthUnit);
    comboboxesGroup->addWidget(m_page->pixelHeightUnit);
    comboboxesGroup->addWidget(m_page->printWidthUnit);
    comboboxesGroup->addWidget(m_page->printHeightUnit);
    comboboxesGroup->addWidget(m_page->printResolutionUnit);
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    connect(m_page->pixelAspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->printAspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->constrainProportionsCkb, SIGNAL(toggled(bool)), this, SLOT(slotAspectChanged(bool)));

    connect(m_page->pixelWidth, SIGNAL(valueChanged(int)), this, SLOT(slotPixelWidthChanged(int)));
    connect(m_page->pixelHeight, SIGNAL(valueChanged(int)), this, SLOT(slotPixelHeightChanged(int)));
    connect(m_page->pixelWidthDouble, SIGNAL(valueChanged(double)), this, SLOT(slotPixelWidthChanged(double)));
    connect(m_page->pixelHeightDouble, SIGNAL(valueChanged(double)), this, SLOT(slotPixelHeightChanged(double)));
    connect(m_page->pixelWidthUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPixelWidthUnitChanged()));
    connect(m_page->pixelHeightUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPixelHeightUnitChanged()));

    connect(m_page->printWidth, SIGNAL(valueChanged(double)), this, SLOT(slotPrintWidthChanged(double)));
    connect(m_page->printHeight, SIGNAL(valueChanged(double)), this, SLOT(slotPrintHeightChanged(double)));
    connect(m_page->printWidthUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPrintWidthUnitChanged()));
    connect(m_page->printHeightUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPrintHeightUnitChanged()));

    connect(m_page->printResolution, SIGNAL(valueChanged(double)), this, SLOT(slotPrintResolutionChanged(double)));
    connect(m_page->printResolution, SIGNAL(editingFinished()), this, SLOT(slotPrintResolutionEditFinished()));
    connect(m_page->printResolutionUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPrintResolutionUnitChanged()));

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

void DlgImageSize::slotPixelWidthChanged(int w)
{
    slotPixelWidthChanged((double) w);
}

void DlgImageSize::slotPixelHeightChanged(int h)
{
    slotPixelHeightChanged((double) h);
}

void DlgImageSize::slotPixelWidthChanged(double w)
{
    if (m_page->pixelWidthUnit->currentText() == percentStr) {
        m_width = qRound((w * m_originalWidth) / 100.0);
    } else {
        m_width = w;
    }

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
    if (m_page->pixelHeightUnit->currentText() == percentStr) {
        m_height = qRound((h * m_originalHeight) / 100.0);
    } else {
        m_height = h;
    }

    m_printHeight = m_height / m_resolution;
    updatePrintHeightUIValue(m_printHeight);

    if (m_keepAspect) {
        m_width = qRound(m_height * m_aspectRatio);
        updatePixelWidthUIValue(m_width);

        m_printWidth = m_width / m_resolution;
        updatePrintWidthUIValue(m_printWidth);
    }
}

void DlgImageSize::slotPixelWidthUnitChanged()
{
    updatePixelWidthUIValue(m_width);

    m_page->pixelWidth->setVisible(m_page->pixelWidthUnit->currentText() == pixelStr);
    m_page->pixelWidthDouble->setVisible(m_page->pixelWidthUnit->currentText() == percentStr);
}

void DlgImageSize::slotPixelHeightUnitChanged()
{
    updatePixelHeightUIValue(m_height);

    m_page->pixelHeight->setVisible(m_page->pixelHeightUnit->currentText() == pixelStr);
    m_page->pixelHeightDouble->setVisible(m_page->pixelHeightUnit->currentText() == percentStr);
}

void DlgImageSize::slotPrintWidthChanged(double w)
{
    if (m_page->printWidthUnit->currentText() == percentStr) {
        const double originalWidthPoint = m_originalWidth / m_originalResolution;
        m_printWidth = (w * originalWidthPoint) / 100.0;
    } else {
        KoUnit selectedUnit = KoUnit::fromListForUi(m_page->printWidthUnit->currentIndex(), KoUnit::HidePixel);
        m_printWidth = selectedUnit.fromUserValue(w);
    }

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
    if (m_page->printHeightUnit->currentText() == percentStr) {
        const double originalHeightPoint = m_originalHeight / m_originalResolution;
        m_printHeight = (h * originalHeightPoint) / 100.0;
    } else {
        KoUnit selectedUnit = KoUnit::fromListForUi(m_page->printHeightUnit->currentIndex(), KoUnit::HidePixel);
        m_printHeight = selectedUnit.fromUserValue(h);
    }

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

void DlgImageSize::slotPrintWidthUnitChanged()
{
    updatePrintWidthUIValue(m_printWidth);
}

void DlgImageSize::slotPrintHeightUnitChanged()
{
    updatePrintHeightUIValue(m_printHeight);
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
    if (m_page->pixelWidthUnit->currentText() == percentStr) {
        m_page->pixelWidthDouble->blockSignals(true);
        m_page->pixelWidthDouble->setValue((value * 100.0) / m_originalWidth);
        m_page->pixelWidthDouble->blockSignals(false);
    } else {
        m_page->pixelWidth->blockSignals(true);
        m_page->pixelWidth->setValue(value);
        m_page->pixelWidth->blockSignals(false);
    }
}

void DlgImageSize::updatePixelHeightUIValue(double value)
{
    if (m_page->pixelHeightUnit->currentText() == percentStr) {
        m_page->pixelHeightDouble->blockSignals(true);
        m_page->pixelHeightDouble->setValue((value * 100.0) / m_originalHeight);
        m_page->pixelHeightDouble->blockSignals(false);
    } else {
        m_page->pixelHeight->blockSignals(true);
        m_page->pixelHeight->setValue(value);
        m_page->pixelHeight->blockSignals(false);
    }
}

void DlgImageSize::updatePrintWidthUIValue(double value)
{
    double uiValue = 0.0;
    if (m_page->printWidthUnit->currentText() == percentStr) {
        // We need to compute percent in point unit because:
        // - originalWith is a value expressed in px (original resolution)
        // - value is expressed in point unit (current resolution)
        // - the percentage value should be based on the original print size
        const double originalWidthPoint = m_originalWidth / m_originalResolution;
        uiValue = (value * 100.0) / originalWidthPoint;
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->printWidthUnit->currentIndex());
        uiValue = selectedUnit.toUserValue(value);
    }
    m_page->printWidth->blockSignals(true);
    m_page->printWidth->setValue(uiValue);
    m_page->printWidth->blockSignals(false);
}

void DlgImageSize::updatePrintHeightUIValue(double value)
{
    double uiValue = 0.0;
    if (m_page->printHeightUnit->currentText() == percentStr) {
        // We need to compute percent in point unit because:
        // - originalHeight is a value expressed in px (original resolution)
        // - value is expressed in point unit (current resolution)
        // - the percentage value should be based on the original print size
        const double originalHeightPoint = m_originalHeight / m_originalResolution;
        uiValue = (value * 100.0) / originalHeightPoint;
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->printHeightUnit->currentIndex());
        uiValue = selectedUnit.toUserValue(value);
    }
    m_page->printHeight->blockSignals(true);
    m_page->printHeight->setValue(uiValue);
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

