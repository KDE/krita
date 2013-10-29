/*
    Copyright (C) 2012  Dan Leinir Turthra Jensen <admin@leinir.dk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "kistoolcropconfigwidget.h"
#include "kis_tool_crop.h"

KisToolCropConfigWidget::KisToolCropConfigWidget(QWidget* parent, KisToolCrop* cropTool)
    : QWidget(parent)
    , m_cropTool(cropTool)
{
    setupUi(this);
    boolHeight->setIcon(KIcon("height_icon"));
    boolWidth->setIcon(KIcon("width_icon"));
    boolRatio->setIcon(KIcon("ratio_icon"));
    label_horizPos->setPixmap(KIcon("offset_horizontal").pixmap(16, 16));
    label_vertiPos->setPixmap(KIcon("offset_vertical").pixmap(16, 16));

    connect(intHeight, SIGNAL(valueChanged(int)), SIGNAL(cropHeightChanged(int)));
    connect(intWidth, SIGNAL(valueChanged(int)), SIGNAL(cropWidthChanged(int)));
    connect(cmbType, SIGNAL(currentIndexChanged(int)), SIGNAL(cropTypeChanged(int)));
    connect(intX, SIGNAL(valueChanged(int)), SIGNAL(cropXChanged(int)));
    connect(intY, SIGNAL(valueChanged(int)), SIGNAL(cropYChanged(int)));
    connect(boolHeight, SIGNAL(toggled(bool)), SIGNAL(forceHeightCHanged(bool)));
    connect(boolWidth, SIGNAL(toggled(bool)), SIGNAL(forceWidthChanged(bool)));
    connect(boolRatio, SIGNAL(toggled(bool)), SIGNAL(forceRatioChanged(bool)));
    connect(doubleRatio, SIGNAL(valueChanged(double)), SIGNAL(ratioChanged(double)));
    connect(cmbDecor, SIGNAL(currentIndexChanged(int)), SIGNAL(decorationChanged(int)));

    connect(cropTool, SIGNAL(cropHeightChanged()), SLOT(cropHeightChanged()));
    connect(cropTool, SIGNAL(cropTypeChanged()), SLOT(cropTypeChanged()));
    connect(cropTool, SIGNAL(cropTypeSelectableChanged()), SLOT(cropTypeSelectableChanged()));
    connect(cropTool, SIGNAL(cropWidthChanged()), SLOT(cropWidthChanged()));
    connect(cropTool, SIGNAL(cropXChanged()), SLOT(cropXChanged()));
    connect(cropTool, SIGNAL(cropYChanged()), SLOT(cropYChanged()));
    connect(cropTool, SIGNAL(forceHeightCHanged()), SLOT(forceHeightCHanged()));
    connect(cropTool, SIGNAL(forceRatioChanged()), SLOT(forceRatioChanged()));
    connect(cropTool, SIGNAL(forceWidthChanged()), SLOT(forceWidthChanged()));
    connect(cropTool, SIGNAL(ratioChanged()), SLOT(ratioChanged()));
    connect(cropTool, SIGNAL(decorationChanged()), SLOT(decorationChanged()));
}

void KisToolCropConfigWidget::cropHeightChanged()
{
    intHeight->blockSignals(true);
    intHeight->setValue(m_cropTool->cropHeight());
    intHeight->blockSignals(false);
}

void KisToolCropConfigWidget::cropTypeChanged()
{
    cmbType->blockSignals(true);
    cmbType->setCurrentIndex(m_cropTool->cropType());
    cmbType->blockSignals(false);
}

void KisToolCropConfigWidget::cropTypeSelectableChanged()
{
    cmbType->setEnabled(m_cropTool->cropTypeSelectable());
}

void KisToolCropConfigWidget::cropWidthChanged()
{
    intWidth->blockSignals(true);
    intWidth->setValue(m_cropTool->cropWidth());
    intWidth->blockSignals(false);
}

void KisToolCropConfigWidget::cropXChanged()
{
    intX->blockSignals(true);
    intX->setValue(m_cropTool->cropX());
    intX->blockSignals(false);
}

void KisToolCropConfigWidget::cropYChanged()
{
    intY->blockSignals(true);
    intY->setValue(m_cropTool->cropY());
    intY->blockSignals(false);
}

void KisToolCropConfigWidget::forceHeightCHanged()
{
    boolHeight->blockSignals(true);
    boolHeight->setChecked(m_cropTool->forceHeight());
    boolHeight->blockSignals(false);
}

void KisToolCropConfigWidget::forceRatioChanged()
{
    boolRatio->blockSignals(true);
    boolRatio->setChecked(m_cropTool->forceRatio());
    boolRatio->blockSignals(false);
}

void KisToolCropConfigWidget::forceWidthChanged()
{
    boolWidth->blockSignals(true);
    boolWidth->setChecked(m_cropTool->forceWidth());
    boolWidth->blockSignals(false);
}

void KisToolCropConfigWidget::ratioChanged()
{
    doubleRatio->blockSignals(true);
    doubleRatio->setValue(m_cropTool->ratio());
    doubleRatio->blockSignals(false);
}

void KisToolCropConfigWidget::decorationChanged()
{
    cmbDecor->blockSignals(true);
    cmbDecor->setCurrentIndex(m_cropTool->decoration());
    cmbDecor->blockSignals(false);
}
