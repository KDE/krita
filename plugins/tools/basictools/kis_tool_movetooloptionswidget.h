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


#ifndef KIS_TOOL_MOVETOOLOPTIONSWIDGET_H
#define KIS_TOOL_MOVETOOLOPTIONSWIDGET_H

#include "ui_wdgmovetool.h"
#include <kconfiggroup.h>
#include "kis_tool_move.h"

class MoveToolOptionsWidget : public QWidget, public Ui::WdgMoveTool
{
    Q_OBJECT
public:
    MoveToolOptionsWidget(QWidget *parent, int resolution, QString toolId);
    int moveStep();
    KisToolMove::MoveToolMode mode();

private Q_SLOTS:
    void on_spinMoveStep_valueChanged(double UIMoveStep);

    void on_cmbUnit_currentIndexChanged(int newUnit);

    void on_radioSelectedLayer_toggled(bool checked);

    void on_radioFirstLayer_toggled(bool checked);

    void on_radioGroup_toggled(bool checked);

private:
    void updateUIUnit(int newUnit);
    void setMoveToolMode(KisToolMove::MoveToolMode newMode);
    int m_resolution;
    int m_moveStep;
    int m_moveStepUnit;
    KisToolMove::MoveToolMode m_moveToolMode;

    KConfigGroup m_configGroup;
};

#endif // KIS_TOOL_MOVETOOLOPTIONSWIDGET_H
