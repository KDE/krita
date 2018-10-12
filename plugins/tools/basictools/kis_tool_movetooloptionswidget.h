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
    double moveScale();
    KisToolMove::MoveToolMode mode();
    bool showCoordinates() const;

public Q_SLOTS:
    void setShowCoordinates(bool value);

    void slotSetTranslate(QPoint newPos);

private Q_SLOTS:
    void on_spinMoveStep_valueChanged(double UIMoveStep);

    void on_spinMoveScale_valueChanged(double UIMoveScale);

    void on_cmbUnit_currentIndexChanged(int newUnit);

    void on_radioSelectedLayer_toggled(bool checked);

    void on_radioFirstLayer_toggled(bool checked);

    void on_radioGroup_toggled(bool checked);

    void on_chkShowCoordinates_toggled(bool checked);

    void on_translateXBox_valueChanged(int arg1);

    void on_translateYBox_valueChanged(int arg1);

Q_SIGNALS:
    void showCoordinatesChanged(bool value);

    void sigSetTranslateX(int value);
    void sigSetTranslateY(int value);

    void sigRequestCommitOffsetChanges();

private:
    void updateUIUnit(int newUnit);
    void setMoveToolMode(KisToolMove::MoveToolMode newMode);
    int m_resolution;
    int m_moveStep;
    int m_moveStepUnit;
    qreal m_moveScale;
    KisToolMove::MoveToolMode m_moveToolMode;
    bool m_showCoordinates;

    int m_TranslateX;
    int m_TranslateY;

    KConfigGroup m_configGroup;
};

#endif // KIS_TOOL_MOVETOOLOPTIONSWIDGET_H
