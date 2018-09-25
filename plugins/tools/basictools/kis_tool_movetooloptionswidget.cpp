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


#include "kis_tool_movetooloptionswidget.h"
#include <ksharedconfig.h>
#include <KoUnit.h>


MoveToolOptionsWidget::MoveToolOptionsWidget(QWidget *parent, int resolution, QString toolId)
  : QWidget(parent)
  , m_resolution(resolution)
  , m_showCoordinates(false)
{
    setupUi(this);
    m_configGroup = KSharedConfig::openConfig()->group(toolId);


    // load radio button
    m_moveToolMode = static_cast<KisToolMove::MoveToolMode>(m_configGroup.readEntry("moveToolMode", 0));
    if(m_moveToolMode == KisToolMove::MoveSelectedLayer)
        radioSelectedLayer->setChecked(true);
    else if (m_moveToolMode == KisToolMove::MoveFirstLayer)
        radioFirstLayer->setChecked(true);
    else
        radioGroup->setChecked(true);

    // Keyboard shortcut move step
    m_moveStep = m_configGroup.readEntry<int>("moveToolStep", 1);
    m_moveStepUnit = m_configGroup.readEntry<int>("moveToolUnit", KoUnit(KoUnit::Pixel).indexInListForUi());
    cmbUnit->addItems(KoUnit::listOfUnitNameForUi());
    cmbUnit->setCurrentIndex(m_moveStepUnit);
    updateUIUnit(m_moveStepUnit);

    // Scale for large moves
    m_moveScale = m_configGroup.readEntry<int>("moveToolScale", 10.0);
    spinMoveScale->blockSignals(true);
    spinMoveScale->setValue(m_moveScale);
    spinMoveScale->setSuffix("x");
    spinMoveScale->blockSignals(false);

    // Switch mode for showing coordinates
    m_showCoordinates = m_configGroup.readEntry("moveToolShowCoordinates", false);
    connect(chkShowCoordinates, SIGNAL(toggled(bool)), SIGNAL(showCoordinatesChanged(bool)));

    chkShowCoordinates->setChecked(m_showCoordinates);

    translateXBox->setSuffix(i18n(" px"));
    translateYBox->setSuffix(i18n(" px"));
    translateXBox->setRange(-10000, 10000);
    translateYBox->setRange(-10000, 10000);
}

void MoveToolOptionsWidget::updateUIUnit(int newUnit)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(newUnit);
    qreal valueForUI;
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spinMoveStep->setRange(0.0001, 10000.000);
        spinMoveStep->setSingleStep(.1);
        spinMoveStep->setDecimals(4);
        valueForUI = selectedUnit.toUserValue((qreal)m_moveStep / (qreal)m_resolution);
    } else {
        spinMoveStep->setRange(1, 10000);
        spinMoveStep->setSingleStep(1);
        spinMoveStep->setDecimals(0);
        valueForUI = m_moveStep;
    }

    spinMoveStep->blockSignals(true);
    spinMoveStep->setValue(valueForUI);
    spinMoveStep->blockSignals(false);

    connect(translateXBox, SIGNAL(editingFinished()), SIGNAL(sigRequestCommitOffsetChanges()));
    connect(translateYBox, SIGNAL(editingFinished()), SIGNAL(sigRequestCommitOffsetChanges()));
}

void MoveToolOptionsWidget::on_spinMoveStep_valueChanged(double UIMoveStep)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(m_moveStepUnit);
    const double scaledUiMoveStep = (selectedUnit == KoUnit(KoUnit::Pixel)) ?
      UIMoveStep : selectedUnit.fromUserValue(UIMoveStep * m_resolution);
    m_moveStep = qRound(scaledUiMoveStep);
    m_configGroup.writeEntry("moveToolStep", m_moveStep);
}

void MoveToolOptionsWidget::on_spinMoveScale_valueChanged(double UIMoveScale)
{
    m_moveScale = UIMoveScale;
    m_configGroup.writeEntry("moveToolScale", m_moveScale);
}


void MoveToolOptionsWidget::on_cmbUnit_currentIndexChanged(int newUnit)
{
    m_moveStepUnit = newUnit;
    updateUIUnit(newUnit);
    m_configGroup.writeEntry("moveToolUnit", newUnit);
}

void MoveToolOptionsWidget::on_radioSelectedLayer_toggled(bool checked)
{
    Q_UNUSED(checked);
    setMoveToolMode(KisToolMove::MoveSelectedLayer);
}

void MoveToolOptionsWidget::on_radioFirstLayer_toggled(bool checked)
{
    Q_UNUSED(checked);
    setMoveToolMode(KisToolMove::MoveFirstLayer);
}

void MoveToolOptionsWidget::on_radioGroup_toggled(bool checked)
{
    Q_UNUSED(checked);
    setMoveToolMode(KisToolMove::MoveGroup);
}

void MoveToolOptionsWidget::setMoveToolMode(KisToolMove::MoveToolMode newMode)
{
    m_moveToolMode = newMode;
    m_configGroup.writeEntry("moveToolMode", static_cast<int>(newMode));
}

void MoveToolOptionsWidget::on_chkShowCoordinates_toggled(bool checked)
{
    m_showCoordinates = checked;
    m_configGroup.writeEntry("moveToolShowCoordinates", m_showCoordinates);
}

KisToolMove::MoveToolMode MoveToolOptionsWidget::mode()
{
    return m_moveToolMode;
}

int MoveToolOptionsWidget::moveStep()
{
  return m_moveStep;
}

double MoveToolOptionsWidget::moveScale()
{
    return m_moveScale;
}

bool MoveToolOptionsWidget::showCoordinates() const
{
    return m_showCoordinates;
}

void MoveToolOptionsWidget::setShowCoordinates(bool value)
{
    chkShowCoordinates->setChecked(value);
}

void MoveToolOptionsWidget::slotSetTranslate(QPoint newPos)
{
    translateXBox->setValue(newPos.x());
    translateYBox->setValue(newPos.y());
}

void MoveToolOptionsWidget::on_translateXBox_valueChanged(int arg1)
{
    m_TranslateX = arg1;
    m_configGroup.writeEntry("moveToolChangedValueX", m_TranslateX);
    emit sigSetTranslateX(arg1);
}

void MoveToolOptionsWidget::on_translateYBox_valueChanged(int arg1)
{
    m_TranslateY = arg1;
    m_configGroup.writeEntry("moveToolChangedValueY", m_TranslateY);
    emit sigSetTranslateY(arg1);
}
