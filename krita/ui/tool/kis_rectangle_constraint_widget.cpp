/*
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

#include "kis_rectangle_constraint_widget.h"
#include "kis_tool_rectangle_base.h"

#include <kis_icon_utils.h>

KisRectangleConstraintWidget::KisRectangleConstraintWidget(QWidget *parent, KisToolRectangleBase *tool) : QWidget(parent) 
{
    m_tool = tool;
    
    setupUi(this);
    
    chkHeight->setIcon(KisIconUtils::loadIcon("height_icon"));
    chkWidth->setIcon(KisIconUtils::loadIcon("width_icon"));
    chkRatio->setIcon(KisIconUtils::loadIcon("ratio_icon"));
    
    connect(chkWidth, SIGNAL(toggled(bool)), this, SLOT(inputsChanged(void)));
    connect(chkHeight, SIGNAL(toggled(bool)), this, SLOT(inputsChanged(void)));
    connect(chkRatio, SIGNAL(toggled(bool)), this, SLOT(inputsChanged(void)));
    
    connect(intWidth,  SIGNAL(valueChanged(int)), this, SLOT(inputsChanged(void)));
    connect(intHeight, SIGNAL(valueChanged(int)), this, SLOT(inputsChanged(void)));
    connect(doubleRatio, SIGNAL(valueChanged(double)), this, SLOT(inputsChanged(void)));
  
    connect(this, SIGNAL(constraintsChanged(bool,bool,bool,float,float,float)), m_tool, SLOT(constraintsChanged(bool,bool,bool,float,float,float)));
    connect(m_tool, SIGNAL(rectangleChanged(QRectF)), this, SLOT(rectangleChanged(QRectF)));
}

void KisRectangleConstraintWidget::inputsChanged() 
{
    emit constraintsChanged(
        chkRatio->isChecked(), 
        chkWidth->isChecked(),
        chkHeight->isChecked(),
        doubleRatio->value(), 
        intWidth->value(), 
        intHeight->value()
    );
}

void KisRectangleConstraintWidget::rectangleChanged(const QRectF &rect) 
{
    intWidth->blockSignals(true);
    intHeight->blockSignals(true);
    doubleRatio->blockSignals(true);
    
    if (!chkWidth->isChecked()) intWidth->setValue(rect.width());
    if (!chkHeight->isChecked()) intHeight->setValue(rect.height());

    if (!chkRatio->isChecked() && !(rect.width() == 0 && rect.height() == 0)) {
        doubleRatio->setValue(fabs(rect.width()) / fabs(rect.height()));
    }
  
    intWidth->blockSignals(false);
    intHeight->blockSignals(false);
    doubleRatio->blockSignals(false);
}
