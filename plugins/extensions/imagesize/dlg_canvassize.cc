/*
 *
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#include "dlg_canvassize.h"
#include "kcanvaspreview.h"

#include <kis_config.h>
#include <KoUnit.h>
#include <kis_icon.h>
#include <kis_size_group.h>
#include <klocalizedstring.h>

#include <kis_document_aware_spin_box_unit_manager.h>

#include <QComboBox>
#include <QButtonGroup>

// used to extend KoUnit in comboboxes
static const QString percentStr(i18n("Percent (%)"));

const QString DlgCanvasSize::PARAM_PREFIX = "canvasizedlg";
const QString DlgCanvasSize::PARAM_WIDTH_UNIT = DlgCanvasSize::PARAM_PREFIX + "_widthunit";
const QString DlgCanvasSize::PARAM_HEIGHT_UNIT = DlgCanvasSize::PARAM_PREFIX + "_heightunit";
const QString DlgCanvasSize::PARAM_XOFFSET_UNIT = DlgCanvasSize::PARAM_PREFIX + "_xoffsetunit";
const QString DlgCanvasSize::PARAM_YOFFSET_UNIT = DlgCanvasSize::PARAM_PREFIX + "_yoffsetunit";

DlgCanvasSize::DlgCanvasSize(QWidget *parent, int width, int height, double resolution)
    : KoDialog(parent)
    , m_keepAspect(true)
    , m_aspectRatio((double)width / height)
    , m_resolution(resolution)
    , m_originalWidth(width)
    , m_originalHeight(height)
    , m_newWidth(width)
    , m_newHeight(height)
    , m_xOffset(0)
    , m_yOffset(0)
{
    setCaption(i18n("Resize Canvas"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgCanvasSize(this);
    Q_CHECK_PTR(m_page);
    m_page->layout()->setMargin(0);
    m_page->setObjectName("canvas_size");

    _widthUnitManager = new KisDocumentAwareSpinBoxUnitManager(this);
    _heightUnitManager = new KisDocumentAwareSpinBoxUnitManager(this, KisDocumentAwareSpinBoxUnitManager::PIX_DIR_Y);

    KisConfig cfg(true);

    _widthUnitManager->setApparentUnitFromSymbol("px");
    _heightUnitManager->setApparentUnitFromSymbol("px");

    m_page->newWidthDouble->setUnitManager(_widthUnitManager);
    m_page->newHeightDouble->setUnitManager(_heightUnitManager);
    m_page->newWidthDouble->setDecimals(2);
    m_page->newHeightDouble->setDecimals(2);
    m_page->newWidthDouble->setDisplayUnit(false);
    m_page->newHeightDouble->setDisplayUnit(false);

    m_page->newWidthDouble->setValue(width);
    m_page->newWidthDouble->setFocus();
    m_page->newHeightDouble->setValue(height);

    m_page->widthUnit->setModel(_widthUnitManager);
    m_page->heightUnit->setModel(_heightUnitManager);

    QString unitw = cfg.readEntry<QString>(PARAM_WIDTH_UNIT, "px");
    QString unith = cfg.readEntry<QString>(PARAM_HEIGHT_UNIT, "px");

    _widthUnitManager->setApparentUnitFromSymbol(unitw);
    _heightUnitManager->setApparentUnitFromSymbol(unith);

    const int wUnitIndex = _widthUnitManager->getsUnitSymbolList().indexOf(unitw);
    const int hUnitIndex = _widthUnitManager->getsUnitSymbolList().indexOf(unith);

    m_page->widthUnit->setCurrentIndex(wUnitIndex);
    m_page->heightUnit->setCurrentIndex(hUnitIndex);

    _xOffsetUnitManager = new KisDocumentAwareSpinBoxUnitManager(this);
    _yOffsetUnitManager = new KisDocumentAwareSpinBoxUnitManager(this, KisDocumentAwareSpinBoxUnitManager::PIX_DIR_Y);

    _xOffsetUnitManager->setApparentUnitFromSymbol("px");
    _yOffsetUnitManager->setApparentUnitFromSymbol("px");

    m_page->xOffsetDouble->setUnitManager(_xOffsetUnitManager);
    m_page->yOffsetDouble->setUnitManager(_yOffsetUnitManager);
    m_page->xOffsetDouble->setDecimals(2);
    m_page->yOffsetDouble->setDecimals(2);
    m_page->xOffsetDouble->setDisplayUnit(false);
    m_page->yOffsetDouble->setDisplayUnit(false);

    m_page->xOffUnit->setModel(_xOffsetUnitManager);
    m_page->yOffUnit->setModel(_yOffsetUnitManager);

    m_page->xOffsetDouble->changeValue(m_xOffset);
    m_page->yOffsetDouble->changeValue(m_yOffset);

    QString unitx = cfg.readEntry<QString>(PARAM_XOFFSET_UNIT, "px");
    QString unity = cfg.readEntry<QString>(PARAM_YOFFSET_UNIT, "px");

    _xOffsetUnitManager->setApparentUnitFromSymbol(unitx);
    _yOffsetUnitManager->setApparentUnitFromSymbol(unity);

    const int xUnitIndex = _xOffsetUnitManager->getsUnitSymbolList().indexOf(unitx);
    const int yUnitIndex = _yOffsetUnitManager->getsUnitSymbolList().indexOf(unity);

    m_page->xOffUnit->setCurrentIndex(xUnitIndex);
    m_page->yOffUnit->setCurrentIndex(yUnitIndex);

    m_page->canvasPreview->setImageSize(m_originalWidth, m_originalHeight);
    m_page->canvasPreview->setCanvasSize(m_originalWidth, m_originalHeight);
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);

    m_page->aspectRatioBtn->setKeepAspectRatio(cfg.readEntry("CanvasSize/KeepAspectRatio", false));
    m_page->constrainProportionsCkb->setChecked(cfg.readEntry("CanvasSize/ConstrainProportions", false));
    m_keepAspect = cfg.readEntry("CanvasSize/KeepAspectRatio", false);


    m_group = new QButtonGroup(m_page);
    m_group->addButton(m_page->topLeft, NORTH_WEST);
    m_group->addButton(m_page->topCenter, NORTH);
    m_group->addButton(m_page->topRight, NORTH_EAST);

    m_group->addButton(m_page->middleLeft, WEST);
    m_group->addButton(m_page->middleCenter, CENTER);
    m_group->addButton(m_page->middleRight, EAST);

    m_group->addButton(m_page->bottomLeft, SOUTH_WEST);
    m_group->addButton(m_page->bottomCenter, SOUTH);
    m_group->addButton(m_page->bottomRight, SOUTH_EAST);

    loadAnchorIcons();
    m_group->button(CENTER)->setChecked(true);
    updateAnchorIcons(CENTER);

    KisSizeGroup *labelsGroup = new KisSizeGroup(this);
    labelsGroup->addWidget(m_page->lblNewWidth);
    labelsGroup->addWidget(m_page->lblNewHeight);
    labelsGroup->addWidget(m_page->lblXOff);
    labelsGroup->addWidget(m_page->lblYOff);
    labelsGroup->addWidget(m_page->lblAnchor);

    KisSizeGroup *spinboxesGroup = new KisSizeGroup(this);
    spinboxesGroup->addWidget(m_page->newWidthDouble);
    spinboxesGroup->addWidget(m_page->newHeightDouble);
    spinboxesGroup->addWidget(m_page->xOffsetDouble);
    spinboxesGroup->addWidget(m_page->yOffsetDouble);

    KisSizeGroup *comboboxesGroup = new KisSizeGroup(this);
    comboboxesGroup->addWidget(m_page->widthUnit);
    comboboxesGroup->addWidget(m_page->heightUnit);
    comboboxesGroup->addWidget(m_page->xOffUnit);
    comboboxesGroup->addWidget(m_page->yOffUnit);

    setMainWidget(m_page);
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    connect(m_page->newWidthDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotWidthChanged(double)));
    connect(m_page->newHeightDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotHeightChanged(double)));
    connect(m_page->widthUnit, SIGNAL(currentIndexChanged(int)), _widthUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(m_page->heightUnit, SIGNAL(currentIndexChanged(int)), _heightUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(_widthUnitManager, SIGNAL(unitChanged(int)), m_page->widthUnit, SLOT(setCurrentIndex(int)));
    connect(_heightUnitManager, SIGNAL(unitChanged(int)), m_page->heightUnit, SLOT(setCurrentIndex(int)));

    connect(m_page->xOffsetDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotXOffsetChanged(double)));
    connect(m_page->yOffsetDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotYOffsetChanged(double)));
    connect(m_page->xOffUnit, SIGNAL(currentIndexChanged(int)), _xOffsetUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(m_page->yOffUnit, SIGNAL(currentIndexChanged(int)), _yOffsetUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(_xOffsetUnitManager, SIGNAL(unitChanged(int)), m_page->xOffUnit, SLOT(setCurrentIndex(int)));
    connect(_yOffsetUnitManager, SIGNAL(unitChanged(int)), m_page->yOffUnit, SLOT(setCurrentIndex(int)));

    connect(m_page->constrainProportionsCkb, SIGNAL(toggled(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->aspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));

    connect(m_group, SIGNAL(buttonClicked(int)), SLOT(slotAnchorButtonClicked(int)));
    connect(m_page->canvasPreview, SIGNAL(sigModifiedXOffset(int)), this, SLOT(slotCanvasPreviewXOffsetChanged(int)));
    connect(m_page->canvasPreview, SIGNAL(sigModifiedYOffset(int)), this, SLOT(slotCanvasPreviewYOffsetChanged(int)));
}

DlgCanvasSize::~DlgCanvasSize()
{
    KisConfig cfg(false);
    cfg.writeEntry<bool>("CanvasSize/KeepAspectRatio", m_page->aspectRatioBtn->keepAspectRatio());
    cfg.writeEntry<bool>("CanvasSize/ConstrainProportions", m_page->constrainProportionsCkb->isChecked());

    cfg.writeEntry<QString>(PARAM_WIDTH_UNIT, _widthUnitManager->getApparentUnitSymbol());
    cfg.writeEntry<QString>(PARAM_HEIGHT_UNIT, _heightUnitManager->getApparentUnitSymbol());

    cfg.writeEntry<QString>(PARAM_XOFFSET_UNIT, _xOffsetUnitManager->getApparentUnitSymbol());
    cfg.writeEntry<QString>(PARAM_YOFFSET_UNIT, _yOffsetUnitManager->getApparentUnitSymbol());

    delete m_page;
}

qint32 DlgCanvasSize::width()
{
    return (qint32) m_newWidth;
}

qint32 DlgCanvasSize::height()
{
    return (qint32) m_newHeight;
}

qint32 DlgCanvasSize::xOffset()
{
    return (qint32) m_xOffset;
}

qint32 DlgCanvasSize::yOffset()
{
    return (qint32) m_yOffset;
}

void DlgCanvasSize::slotAspectChanged(bool keep)
{
    m_page->aspectRatioBtn->blockSignals(true);
    m_page->constrainProportionsCkb->blockSignals(true);

    m_page->aspectRatioBtn->setKeepAspectRatio(keep);
    m_page->constrainProportionsCkb->setChecked(keep);

    m_page->aspectRatioBtn->blockSignals(false);
    m_page->constrainProportionsCkb->blockSignals(false);

    m_keepAspect = keep;


    if (keep) {
        // size values may be out of sync, so we need to reset it to defaults
        m_newWidth = m_originalWidth;
        m_newHeight = m_originalHeight;
        m_xOffset = 0;
        m_yOffset = 0;

        m_page->canvasPreview->blockSignals(true);
        m_page->canvasPreview->setCanvasSize(m_newWidth, m_newHeight);
        m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
        m_page->canvasPreview->blockSignals(false);
        updateOffset(CENTER);
        updateButtons(CENTER);
    }
}

void DlgCanvasSize::slotAnchorButtonClicked(int id)
{
    updateOffset(id);
    updateButtons(id);
}

void DlgCanvasSize::slotWidthChanged(double v)
{
    //this slot receiv values in pt from the unitspinbox, so just use the resolution.
    const double resValue = v*_widthUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_newWidth = qRound(resValue);

    if (m_keepAspect) {
        m_newHeight = qRound(m_newWidth / m_aspectRatio);
        m_page->newHeightDouble->blockSignals(true);
        m_page->newHeightDouble->changeValue(v / m_aspectRatio);
        m_page->newHeightDouble->blockSignals(false);
    }

    int savedId = m_group->checkedId();
    m_page->canvasPreview->blockSignals(true);
    m_page->canvasPreview->setCanvasSize(m_newWidth, m_newHeight);
    m_page->canvasPreview->blockSignals(false);
    updateOffset(savedId);
    updateButtons(savedId);
}

void DlgCanvasSize::slotHeightChanged(double v)
{
    //this slot receiv values in pt from the unitspinbox, so just use the resolution.
    const double resValue = v*_heightUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_newHeight = qRound(resValue);

    if (m_keepAspect) {
        m_newWidth = qRound(m_newHeight * m_aspectRatio);
        m_page->newWidthDouble->blockSignals(true);
        m_page->newWidthDouble->changeValue(v * m_aspectRatio);
        m_page->newWidthDouble->blockSignals(false);
    }

    int savedId = m_group->checkedId();
    m_page->canvasPreview->blockSignals(true);
    m_page->canvasPreview->setCanvasSize(m_newWidth, m_newHeight);
    m_page->canvasPreview->blockSignals(false);
    updateOffset(savedId);
    updateButtons(savedId);
}

void DlgCanvasSize::slotXOffsetChanged(double v)
{
    //this slot receiv values in pt from the unitspinbox, so just use the resolution.
    const double resValue = v*_xOffsetUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_xOffset = qRound(resValue);

    m_page->canvasPreview->blockSignals(true);
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
    m_page->canvasPreview->blockSignals(false);

    updateButtons(-1);
}

void DlgCanvasSize::slotYOffsetChanged(double v)
{
    //this slot receiv values in pt from the unitspinbox, so just use the resolution.
    const double resValue = v*_xOffsetUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_yOffset = qRound(resValue);


    m_page->canvasPreview->blockSignals(true);
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
    m_page->canvasPreview->blockSignals(false);

    updateButtons(-1);
}

void DlgCanvasSize::slotCanvasPreviewXOffsetChanged(int v)
{
    double newVal = v / _xOffsetUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_page->xOffsetDouble->changeValue(newVal);
}

void DlgCanvasSize::slotCanvasPreviewYOffsetChanged(int v)
{

    double newVal = v / _yOffsetUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_page->yOffsetDouble->changeValue(newVal);
}

void DlgCanvasSize::loadAnchorIcons()
{

    m_anchorIcons[NORTH_WEST] =  KisIconUtils::loadIcon("arrow-topleft");
    m_anchorIcons[NORTH] = KisIconUtils::loadIcon("arrow-up");
    m_anchorIcons[NORTH_EAST] = KisIconUtils::loadIcon("arrow-topright");
    m_anchorIcons[EAST] = KisIconUtils::loadIcon("arrow-right");
    m_anchorIcons[CENTER] = KisIconUtils::loadIcon("arrow_center");
    m_anchorIcons[WEST] = KisIconUtils::loadIcon("arrow-left");
    m_anchorIcons[SOUTH_WEST] = KisIconUtils::loadIcon("arrow-downleft");
    m_anchorIcons[SOUTH] = KisIconUtils::loadIcon("arrow-down");
    m_anchorIcons[SOUTH_EAST] = KisIconUtils::loadIcon("arrow-downright");

}

void DlgCanvasSize::updateAnchorIcons(int id)
{
    anchor iconLayout[10][9] = {
        {NONE, EAST,  NONE, SOUTH, SOUTH_EAST, NONE, NONE, NONE, NONE},
        {WEST, NONE, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST, NONE, NONE, NONE},
        {NONE, WEST, NONE, NONE, SOUTH_WEST, SOUTH, NONE, NONE, NONE},
        {NORTH, NORTH_EAST, NONE, NONE, EAST, NONE, SOUTH, SOUTH_EAST, NONE},
        {NORTH_WEST, NORTH, NORTH_EAST, WEST, NONE, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST},
        {NONE, NORTH_WEST, NORTH, NONE, WEST, NONE, NONE, SOUTH_WEST, SOUTH},
        {NONE, NONE, NONE, NORTH, NORTH_EAST, NONE, NONE, EAST, NONE},
        {NONE, NONE, NONE, NORTH_WEST, NORTH, NORTH_EAST, WEST, NONE, EAST},
        {NONE, NONE, NONE, NONE, NORTH_WEST, NORTH, NONE, WEST, NONE},
        {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE}
    };

    if (id == -1) {
        id = SOUTH_EAST + 1;
    }

    // we are going to swap arrows direction based on width and height shrinking
    bool shrinkWidth = (m_newWidth < m_originalWidth) ? true : false;
    bool shrinkHeight = (m_newHeight < m_originalHeight) ? true : false;

    for (int i = NORTH_WEST; i <= SOUTH_EAST; i++) {
        anchor iconId = iconLayout[id][i];

        // all corner arrows represents shrinking in some direction
        if (shrinkWidth || shrinkHeight) {
            switch (iconId) {
            case NORTH_WEST: iconId = SOUTH_EAST; break;
            case NORTH_EAST: iconId = SOUTH_WEST; break;
            case SOUTH_WEST: iconId = NORTH_EAST; break;
            case SOUTH_EAST: iconId = NORTH_WEST; break;
            default: break;
            }
        }

        if (shrinkWidth) {
            switch (iconId) {
            case WEST: iconId = EAST; break;
            case EAST: iconId = WEST; break;
            default: break;
            }
        }

        if (shrinkHeight) {
            switch (iconId) {
            case NORTH: iconId = SOUTH; break;
            case SOUTH: iconId = NORTH; break;
            default: break;
            }
        }

        QAbstractButton *button = m_group->button(i);

        if (iconId == NONE) {
            button->setIcon(QIcon());
        } else {
            button->setIcon(m_anchorIcons[iconId]);
        }
    }

}

void DlgCanvasSize::updateButtons(int forceId)
{
    int id = m_group->checkedId();

    if (forceId != -1) {
        m_group->setExclusive(true);
        m_group->button(forceId)->setChecked(true);
        updateAnchorIcons(forceId);
    } else if (id != -1) {
        double xOffset, yOffset;
        expectedOffset(id, xOffset, yOffset);

        // convert values to internal unit
        int internalXOffset = 0;
        int internalYOffset = 0;
        if (m_page->xOffUnit->currentText() == percentStr) {
            internalXOffset = qRound((xOffset * m_newWidth) / 100.0);
            internalYOffset = qRound((yOffset * m_newHeight) / 100.0);
        } else {
            const KoUnit xOffsetUnit = KoUnit::fromListForUi(m_page->xOffUnit->currentIndex());
            internalXOffset = qRound(xOffsetUnit.fromUserValue(xOffset));
            const KoUnit yOffsetUnit = KoUnit::fromListForUi(m_page->yOffUnit->currentIndex());
            internalYOffset = qRound(yOffsetUnit.fromUserValue(yOffset));
        }

        bool offsetAsExpected =
                internalXOffset == m_xOffset &&
                internalYOffset == m_yOffset;

        if (offsetAsExpected) {
            m_group->setExclusive(true);
        } else {
            m_group->setExclusive(false);
            m_group->button(id)->setChecked(false);
            id = -1;
        }

        updateAnchorIcons(id);
    } else {
        updateAnchorIcons(id);
    }
}

void DlgCanvasSize::updateOffset(int id)
{
    if (id == -1) return;

    double xOffset;
    double yOffset;
    expectedOffset(id, xOffset, yOffset);

    m_page->xOffsetDouble->changeValue(xOffset);
    m_page->yOffsetDouble->changeValue(yOffset);
}

void DlgCanvasSize::expectedOffset(int id, double &xOffset, double &yOffset)
{
    const double xCoeff = (id % 3.0) * 0.5;
    const double yCoeff = (id / 3.0) * 0.5;

    const int xDiff = m_newWidth - m_originalWidth;
    const int yDiff = m_newHeight - m_originalHeight;

    //convert to unitmanager default unit.
    xOffset = xDiff * xCoeff / _xOffsetUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    yOffset = yDiff * yCoeff / _yOffsetUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
}
