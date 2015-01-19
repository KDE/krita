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
#include <KoIcon.h>
#include <kis_acyclic_signal_connector.h>


KisToolCropConfigWidget::KisToolCropConfigWidget(QWidget* parent, KisToolCrop* cropTool)
    : QWidget(parent)
    , m_cropTool(cropTool)
{
    setupUi(this);
    boolHeight->setIcon(koIcon("height_icon"));
    boolWidth->setIcon(koIcon("width_icon"));
    boolRatio->setIcon(koIcon("ratio_icon"));
    label_horizPos->setPixmap(koIcon("offset_horizontal").pixmap(16, 16));
    label_vertiPos->setPixmap(koIcon("offset_vertical").pixmap(16, 16));

    // update the UI based off data from crop tool
    intHeight->setValue(m_cropTool->cropHeight());
    cmbType->setCurrentIndex(m_cropTool->cropType());
    cmbType->setEnabled(m_cropTool->cropTypeSelectable());
    intWidth->setValue(m_cropTool->cropWidth());
    intX->setValue(m_cropTool->cropX());
    intY->setValue(m_cropTool->cropY());
    boolHeight->setChecked(m_cropTool->forceHeight());
    boolRatio->setChecked(m_cropTool->forceRatio());
    boolWidth->setChecked(m_cropTool->forceWidth());
    doubleRatio->setValue(m_cropTool->ratio());
    cmbDecor->setCurrentIndex(m_cropTool->decoration());
    boolGrow->setChecked(m_cropTool->allowGrow());
    boolCenter->setChecked(m_cropTool->growCenter());

    KisAcyclicSignalConnector *connector;

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardDouble(doubleRatio, SIGNAL(valueChanged(double)), this, SIGNAL(ratioChanged(double)));
    connector->connectBackwardDouble(cropTool, SIGNAL(ratioChanged(double)), doubleRatio, SLOT(setValue(double)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardInt(intHeight, SIGNAL(valueChanged(int)), this, SIGNAL(cropHeightChanged(int)));
    connector->connectBackwardInt(cropTool, SIGNAL(cropHeightChanged(int)), intHeight, SLOT(setValue(int)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardInt(intWidth, SIGNAL(valueChanged(int)), this, SIGNAL(cropWidthChanged(int)));
    connector->connectBackwardInt(cropTool, SIGNAL(cropWidthChanged(int)), intWidth, SLOT(setValue(int)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardInt(intX, SIGNAL(valueChanged(int)), this, SIGNAL(cropXChanged(int)));
    connector->connectBackwardInt(cropTool, SIGNAL(cropXChanged(int)), intX, SLOT(setValue(int)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardInt(intY, SIGNAL(valueChanged(int)), this, SIGNAL(cropYChanged(int)));
    connector->connectBackwardInt(cropTool, SIGNAL(cropYChanged(int)), intY, SLOT(setValue(int)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardBool(boolHeight, SIGNAL(toggled(bool)), this, SIGNAL(forceHeightChanged(bool)));
    connector->connectBackwardBool(cropTool, SIGNAL(forceHeightChanged(bool)), boolHeight, SLOT(setChecked(bool)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardBool(boolWidth, SIGNAL(toggled(bool)), this, SIGNAL(forceWidthChanged(bool)));
    connector->connectBackwardBool(cropTool, SIGNAL(forceWidthChanged(bool)), boolWidth, SLOT(setChecked(bool)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardBool(boolRatio, SIGNAL(toggled(bool)), this, SIGNAL(forceRatioChanged(bool)));
    connector->connectBackwardBool(cropTool, SIGNAL(forceRatioChanged(bool)), boolRatio, SLOT(setChecked(bool)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardInt(cmbType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(cropTypeChanged(int)));
    connector->connectBackwardInt(cropTool, SIGNAL(cropTypeChanged(int)), cmbType, SLOT(setCurrentIndex(int)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardInt(cmbDecor, SIGNAL(currentIndexChanged(int)), this, SIGNAL(decorationChanged(int)));
    connector->connectBackwardInt(cropTool, SIGNAL(decorationChanged(int)), cmbDecor, SLOT(setCurrentIndex(int)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardBool(boolGrow, SIGNAL(toggled(bool)), this, SIGNAL(allowGrowChanged(bool)));
    connector->connectBackwardBool(cropTool, SIGNAL(canGrowChanged(bool)), boolGrow, SLOT(setChecked(bool)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardBool(boolCenter, SIGNAL(toggled(bool)), this, SIGNAL(growCenterChanged(bool)));
    connector->connectBackwardBool(cropTool, SIGNAL(isCenteredChanged(bool)), boolCenter, SLOT(setChecked(bool)));

    connector = new KisAcyclicSignalConnector(this);
    //connector->connectForwardDouble();
    connector->connectBackwardVoid(cropTool, SIGNAL(cropTypeSelectableChanged()), this, SLOT(cropTypeSelectableChanged()));
}

void KisToolCropConfigWidget::cropTypeSelectableChanged()
{
    cmbType->setEnabled(m_cropTool->cropTypeSelectable());
}
