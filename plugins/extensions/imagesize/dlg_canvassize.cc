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

#include <KoUnit.h>
#include <kis_icon.h>
#include <kis_size_group.h>
#include <klocalizedstring.h>

#include <kis_config.h>

#include <QButtonGroup>

// used to extend KoUnit in comboboxes
static const QString percentStr(i18n("Percent (%)"));

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

    m_page->newWidth->setValue(width);
    m_page->newWidth->setFocus();
    m_page->newHeight->setValue(height);

    m_page->newWidthDouble->setVisible(false);
    m_page->newHeightDouble->setVisible(false);

    m_page->widthUnit->addItems(KoUnit::listOfUnitNameForUi());
    m_page->widthUnit->addItem(percentStr);
    m_page->heightUnit->addItems(KoUnit::listOfUnitNameForUi());
    m_page->heightUnit->addItem(percentStr);

    const int pixelUnitIndex = KoUnit(KoUnit::Pixel).indexInListForUi();
    m_page->widthUnit->setCurrentIndex(pixelUnitIndex);
    m_page->heightUnit->setCurrentIndex(pixelUnitIndex);

    m_page->xOffsetDouble->setVisible(false);
    m_page->yOffsetDouble->setVisible(false);

    m_page->xOffUnit->addItems(KoUnit::listOfUnitNameForUi());
    m_page->xOffUnit->addItem(percentStr);
    m_page->yOffUnit->addItems(KoUnit::listOfUnitNameForUi());
    m_page->yOffUnit->addItem(percentStr);

    m_page->xOffUnit->setCurrentIndex(pixelUnitIndex);
    m_page->yOffUnit->setCurrentIndex(pixelUnitIndex);

    m_page->canvasPreview->setImageSize(m_originalWidth, m_originalHeight);
    m_page->canvasPreview->setCanvasSize(m_originalWidth, m_originalHeight);
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);

    KisConfig cfg;

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
    spinboxesGroup->addWidget(m_page->newWidth);
    spinboxesGroup->addWidget(m_page->newWidthDouble);
    spinboxesGroup->addWidget(m_page->newHeight);
    spinboxesGroup->addWidget(m_page->newHeightDouble);
    spinboxesGroup->addWidget(m_page->xOffset);
    spinboxesGroup->addWidget(m_page->xOffsetDouble);
    spinboxesGroup->addWidget(m_page->yOffset);
    spinboxesGroup->addWidget(m_page->yOffsetDouble);

    KisSizeGroup *comboboxesGroup = new KisSizeGroup(this);
    comboboxesGroup->addWidget(m_page->widthUnit);
    comboboxesGroup->addWidget(m_page->heightUnit);
    comboboxesGroup->addWidget(m_page->xOffUnit);
    comboboxesGroup->addWidget(m_page->yOffUnit);

    setMainWidget(m_page);
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    connect(m_page->newWidth, SIGNAL(valueChanged(int)), this, SLOT(slotWidthChanged(int)));
    connect(m_page->newHeight, SIGNAL(valueChanged(int)), this, SLOT(slotHeightChanged(int)));
    connect(m_page->newWidthDouble, SIGNAL(valueChanged(double)), this, SLOT(slotWidthChanged(double)));
    connect(m_page->newHeightDouble, SIGNAL(valueChanged(double)), this, SLOT(slotHeightChanged(double)));
    connect(m_page->widthUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWidthUnitChanged(int)));
    connect(m_page->heightUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotHeightUnitChanged(int)));

    connect(m_page->xOffset, SIGNAL(valueChanged(int)), this, SLOT(slotXOffsetChanged(int)));
    connect(m_page->yOffset, SIGNAL(valueChanged(int)), this, SLOT(slotYOffsetChanged(int)));
    connect(m_page->xOffsetDouble, SIGNAL(valueChanged(double)), this, SLOT(slotXOffsetChanged(double)));
    connect(m_page->yOffsetDouble, SIGNAL(valueChanged(double)), this, SLOT(slotYOffsetChanged(double)));
    connect(m_page->xOffUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotXOffsetUnitChanged(int)));
    connect(m_page->yOffUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotYOffsetUnitChanged(int)));

    connect(m_page->constrainProportionsCkb, SIGNAL(toggled(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->aspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->aspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));

    connect(m_group, SIGNAL(buttonClicked(int)), SLOT(slotAnchorButtonClicked(int)));
    connect(m_page->canvasPreview, SIGNAL(sigModifiedXOffset(int)), this, SLOT(slotCanvasPreviewXOffsetChanged(int)));
    connect(m_page->canvasPreview, SIGNAL(sigModifiedYOffset(int)), this, SLOT(slotCanvasPreviewYOffsetChanged(int)));
}

DlgCanvasSize::~DlgCanvasSize()
{
    KisConfig cfg;
    cfg.writeEntry<bool>("CanvasSize/KeepAspectRatio", m_page->aspectRatioBtn->keepAspectRatio());
    cfg.writeEntry<bool>("CanvasSize/ConstrainProportions", m_page->constrainProportionsCkb->isChecked());

    delete m_page;
}

qint32 DlgCanvasSize::width()
{
    return (qint32)m_newWidth;
}

qint32 DlgCanvasSize::height()
{
    return (qint32)m_newHeight;
}

qint32 DlgCanvasSize::xOffset()
{
    return (qint32)m_page->xOffset->value();
}

qint32 DlgCanvasSize::yOffset()
{
    return (qint32)m_page->yOffset->value();
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

        updateWidthUIValue(m_newWidth);
        updateHeightUIValue(m_newHeight);
        updateXOffsetUIValue(m_xOffset);
        updateYOffsetUIValue(m_yOffset);

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

void DlgCanvasSize::slotWidthChanged(int v)
{
    slotWidthChanged((double) v);
}

void DlgCanvasSize::slotHeightChanged(int v)
{
    slotHeightChanged((double) v);
}

void DlgCanvasSize::slotWidthChanged(double v)
{
    if (m_page->widthUnit->currentText() == percentStr) {
        m_newWidth = qRound((v * m_originalWidth) / 100.0);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->widthUnit->currentIndex());
        const double resValue = (selectedUnit == KoUnit(KoUnit::Pixel)) ? v : (v * m_resolution);
        m_newWidth = qRound(selectedUnit.fromUserValue(resValue));
    }

    if (m_keepAspect) {
        m_newHeight = qRound(m_newWidth / m_aspectRatio);
        updateHeightUIValue(m_newHeight);
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
    if (m_page->heightUnit->currentText() == percentStr) {
        m_newHeight = qRound((v * m_originalHeight) / 100.0);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->heightUnit->currentIndex());
        const double resValue = (selectedUnit == KoUnit(KoUnit::Pixel)) ? v : (v * m_resolution);
        m_newHeight = qRound(selectedUnit.fromUserValue(resValue));
    }

    if (m_keepAspect) {
        m_newWidth = qRound(m_newHeight * m_aspectRatio);
        updateWidthUIValue(m_newWidth);
    }

    int savedId = m_group->checkedId();
    m_page->canvasPreview->blockSignals(true);
    m_page->canvasPreview->setCanvasSize(m_newWidth, m_newHeight);
    m_page->canvasPreview->blockSignals(false);
    updateOffset(savedId);
    updateButtons(savedId);
}

void DlgCanvasSize::slotWidthUnitChanged(int index)
{
    updateWidthUIValue(m_newWidth);

    if (m_page->widthUnit->currentText() == percentStr) {
        m_page->newWidth->setVisible(false);
        m_page->newWidthDouble->setVisible(true);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(index);
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->newWidth->setVisible(false);
            m_page->newWidthDouble->setVisible(true);
        } else {
            m_page->newWidth->setVisible(true);
            m_page->newWidthDouble->setVisible(false);
        }
    }
}

void DlgCanvasSize::slotHeightUnitChanged(int index)
{
    updateHeightUIValue(m_newHeight);

    if (m_page->heightUnit->currentText() == percentStr) {
        m_page->newHeight->setVisible(false);
        m_page->newHeightDouble->setVisible(true);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(index);
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->newHeight->setVisible(false);
            m_page->newHeightDouble->setVisible(true);
        } else {
            m_page->newHeight->setVisible(true);
            m_page->newHeightDouble->setVisible(false);
        }
    }
}

void DlgCanvasSize::slotXOffsetChanged(int v)
{
    slotXOffsetChanged((double) v);
}

void DlgCanvasSize::slotYOffsetChanged(int v)
{
    slotYOffsetChanged((double) v);
}

void DlgCanvasSize::slotXOffsetChanged(double v)
{
    if (m_page->xOffUnit->currentText() == percentStr) {
        m_xOffset = qRound((v * m_newWidth) / 100.0);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->xOffUnit->currentIndex());
        const double resValue = (selectedUnit == KoUnit(KoUnit::Pixel)) ? v : (v * m_resolution);
        m_xOffset = qRound(selectedUnit.fromUserValue(resValue));
    }

    m_page->canvasPreview->blockSignals(true);
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
    m_page->canvasPreview->blockSignals(false);

    updateButtons(-1);
}

void DlgCanvasSize::slotYOffsetChanged(double v)
{
    if (m_page->yOffUnit->currentText() == percentStr) {
        m_yOffset = qRound((v * m_newHeight) / 100.0);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->yOffUnit->currentIndex());
        const double resValue = (selectedUnit == KoUnit(KoUnit::Pixel)) ? v : (v * m_resolution);
        m_yOffset = qRound(selectedUnit.fromUserValue(resValue));
    }

    m_page->canvasPreview->blockSignals(true);
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
    m_page->canvasPreview->blockSignals(false);

    updateButtons(-1);
}

void DlgCanvasSize::slotXOffsetUnitChanged(int index)
{
    updateXOffsetUIValue(m_xOffset);

    if (m_page->xOffUnit->currentText() == percentStr) {
        m_page->xOffset->setVisible(false);
        m_page->xOffsetDouble->setVisible(true);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(index);
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->xOffset->setVisible(false);
            m_page->xOffsetDouble->setVisible(true);
        } else {
            m_page->xOffset->setVisible(true);
            m_page->xOffsetDouble->setVisible(false);
        }
    }
}

void DlgCanvasSize::slotYOffsetUnitChanged(int index)
{
    updateYOffsetUIValue(m_yOffset);

    if (m_page->yOffUnit->currentText() == percentStr) {
        m_page->yOffset->setVisible(false);
        m_page->yOffsetDouble->setVisible(true);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(index);
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->yOffset->setVisible(false);
            m_page->yOffsetDouble->setVisible(true);
        } else {
            m_page->yOffset->setVisible(true);
            m_page->yOffsetDouble->setVisible(false);
        }
    }
}

void DlgCanvasSize::slotCanvasPreviewXOffsetChanged(int v)
{
    // Convert input value to selected x offset unit.
    // This will be undone later in slotXOffsetChanged (through spinboxes valueChanged signal).
    if (m_page->xOffUnit->currentText() == percentStr) {
        m_page->xOffsetDouble->setValue((v * 100.0) / m_newWidth);
    } else {
        const KoUnit pixelUnit(KoUnit::Pixel);
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->xOffUnit->currentIndex());
        //const double convertedValue = xOffsetUnit.convertFromUnitToUnit(v, pixelUnit, xOffsetUnit);

        if (selectedUnit != pixelUnit) {
            m_page->xOffsetDouble->setValue(selectedUnit.toUserValue(v / m_resolution));
        } else {
            m_page->xOffset->setValue(v);
        }
    }
}

void DlgCanvasSize::slotCanvasPreviewYOffsetChanged(int v)
{
    // Convert input value to selected y offset unit.
    // This will be undone later in slotYOffsetChanged (through spinboxes valueChanged signal).
    if (m_page->yOffUnit->currentText() == percentStr) {
        m_page->yOffsetDouble->setValue((v * 100.0) / m_newHeight);
    } else {
        const KoUnit pixelUnit(KoUnit::Pixel);
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->yOffUnit->currentIndex());
        //const double convertedValue = yOffsetUnit.convertFromUnitToUnit(v, pixelUnit, yOffsetUnit);

        if (selectedUnit != pixelUnit) {
            m_page->yOffsetDouble->setValue(selectedUnit.toUserValue(v / m_resolution));
        } else {
            m_page->yOffset->setValue(v);
        }
    }
}

void DlgCanvasSize::updateWidthUIValue(double value)
{
    if (m_page->widthUnit->currentText() == percentStr) {
        m_page->newWidthDouble->blockSignals(true);
        m_page->newWidthDouble->setValue((value * 100.0) / m_originalWidth);
        m_page->newWidthDouble->blockSignals(false);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->widthUnit->currentIndex());
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->newWidthDouble->blockSignals(true);
            m_page->newWidthDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
            m_page->newWidthDouble->blockSignals(false);
        } else {
            m_page->newWidth->blockSignals(true);
            m_page->newWidth->setValue(selectedUnit.toUserValue(value));
            m_page->newWidth->blockSignals(false);
        }
    }
}

void DlgCanvasSize::updateHeightUIValue(double value)
{
    if (m_page->heightUnit->currentText() == percentStr) {
        m_page->newHeightDouble->blockSignals(true);
        m_page->newHeightDouble->setValue((value * 100.0) / m_originalHeight);
        m_page->newHeightDouble->blockSignals(false);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->heightUnit->currentIndex());
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->newHeightDouble->blockSignals(true);
            m_page->newHeightDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
            m_page->newHeightDouble->blockSignals(false);
        } else {
            m_page->newHeight->blockSignals(true);
            m_page->newHeight->setValue(selectedUnit.toUserValue(value));
            m_page->newHeight->blockSignals(false);
        }
    }
}

void DlgCanvasSize::updateXOffsetUIValue(double value)
{
    if (m_page->xOffUnit->currentText() == percentStr) {
        m_page->xOffsetDouble->blockSignals(true);
        m_page->xOffsetDouble->setValue((value * 100.0) / m_newWidth);
        m_page->xOffsetDouble->blockSignals(false);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->xOffUnit->currentIndex());
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->xOffsetDouble->blockSignals(true);
            m_page->xOffsetDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
            m_page->xOffsetDouble->blockSignals(false);
        } else {
            m_page->xOffset->blockSignals(true);
            m_page->xOffset->setValue(qRound(selectedUnit.toUserValue(value)));
            m_page->xOffset->blockSignals(false);
        }
    }
}

void DlgCanvasSize::updateYOffsetUIValue(double value)
{
    if (m_page->yOffUnit->currentText() == percentStr) {
        m_page->yOffsetDouble->blockSignals(true);
        m_page->yOffsetDouble->setValue((value * 100.0) / m_newHeight);
        m_page->yOffsetDouble->blockSignals(false);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->yOffUnit->currentIndex());
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->yOffsetDouble->blockSignals(true);
            m_page->yOffsetDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
            m_page->yOffsetDouble->blockSignals(false);
        } else {
            m_page->yOffset->blockSignals(true);
            m_page->yOffset->setValue(qRound(selectedUnit.toUserValue(value)));
            m_page->yOffset->blockSignals(false);
        }
    }
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

    const KoUnit pixelUnit(KoUnit::Pixel);

    // update spinbox value (other widgets will be autoupdated later through valueChanged signal)
    if (m_page->xOffUnit->currentText() == percentStr) {
        m_page->xOffsetDouble->setValue(xOffset);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->xOffUnit->currentIndex());
        if (pixelUnit != selectedUnit) {
            m_page->xOffsetDouble->setValue(xOffset);
        } else {
            m_page->xOffset->setValue(qRound(xOffset));
        }
    }

    // update spinbox value (other widgets will be autoupdated later through valueChanged signal)
    if (m_page->yOffUnit->currentText() == percentStr) {
        m_page->yOffsetDouble->setValue(yOffset);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->yOffUnit->currentIndex());
        if (pixelUnit != selectedUnit) {
            m_page->yOffsetDouble->setValue(yOffset);
        } else {
            m_page->yOffset->setValue(qRound(yOffset));
        }
    }
}

void DlgCanvasSize::expectedOffset(int id, double &xOffset, double &yOffset)
{
    const double xCoeff = (id % 3) * 0.5;
    const double yCoeff = (id / 3) * 0.5;

    const int xDiff = m_newWidth - m_originalWidth;
    const int yDiff = m_newHeight - m_originalHeight;

    // use selected unit to convert expected values (the inverse will be do later)
    // so output values are now considered as if they were regular user input
    if (m_page->xOffUnit->currentText() == percentStr) {
        xOffset = (xDiff * xCoeff * 100.0) / m_newWidth;
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->xOffUnit->currentIndex());
        const double resXDiff = (selectedUnit != KoUnit(KoUnit::Pixel)) ? xDiff / m_resolution : xDiff;
        xOffset = selectedUnit.toUserValue(resXDiff * xCoeff);
    }

    if (m_page->yOffUnit->currentText() == percentStr) {
        yOffset = (yDiff * yCoeff * 100.0) / m_newHeight;
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->yOffUnit->currentIndex());
        const double resYDiff = (selectedUnit != KoUnit(KoUnit::Pixel)) ? yDiff / m_resolution : yDiff;
        yOffset = selectedUnit.toUserValue(resYDiff * yCoeff);
    }
}
