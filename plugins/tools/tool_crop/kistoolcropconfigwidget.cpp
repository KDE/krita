/*
    SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "kistoolcropconfigwidget.h"
#include "kis_tool_crop.h"
#include <kis_icon.h>
#include <kis_acyclic_signal_connector.h>


KisToolCropConfigWidget::KisToolCropConfigWidget(QWidget* parent, KisToolCrop* cropTool)
    : QWidget(parent)
    , m_cropTool(cropTool)
{
    setupUi(this);

    // update the UI based off data from crop tool
    intHeight->setValue(m_cropTool->cropHeight());
    cmbType->setCurrentIndex(m_cropTool->cropType());
    cmbType->setEnabled(m_cropTool->cropTypeSelectable());
    intWidth->setValue(m_cropTool->cropWidth());
    intX->setValue(m_cropTool->cropX());
    intY->setValue(m_cropTool->cropY());
    doubleRatio->setValue(m_cropTool->ratio());
    cmbDecor->setCurrentIndex(m_cropTool->decoration());
    boolGrow->setChecked(m_cropTool->allowGrow());
    boolCenter->setChecked(m_cropTool->growCenter());

    lockRatioButton->setChecked(m_cropTool->lockRatio());
    lockRatioButton->setIcon(KisIconUtils::loadIcon("layer-locked"));

    lockHeightButton->setChecked(m_cropTool->lockHeight());
    lockHeightButton->setIcon(KisIconUtils::loadIcon("layer-locked"));
    lockWidthButton->setChecked(m_cropTool->lockWidth());
    lockWidthButton->setIcon(KisIconUtils::loadIcon("layer-locked"));


    KisAcyclicSignalConnector *connector;
    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardBool(lockRatioButton, SIGNAL(toggled(bool)), this, SIGNAL(lockRatioChanged(bool)));
    connector->connectBackwardBool(cropTool, SIGNAL(lockRatioChanged(bool)), lockRatioButton, SLOT(setChecked(bool)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardBool(lockHeightButton, SIGNAL(toggled(bool)), this, SIGNAL(lockHeightChanged(bool)));
    connector->connectBackwardBool(cropTool, SIGNAL(lockHeightChanged(bool)), lockHeightButton, SLOT(setChecked(bool)));

    connector = new KisAcyclicSignalConnector(this);
    connector->connectForwardBool(lockWidthButton, SIGNAL(toggled(bool)), this, SIGNAL(lockWidthChanged(bool)));
    connector->connectBackwardBool(cropTool, SIGNAL(lockWidthChanged(bool)), lockWidthButton, SLOT(setChecked(bool)));

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
