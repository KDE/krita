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


#ifndef KISTOOLCROPCONFIGWIDGET_H
#define KISTOOLCROPCONFIGWIDGET_H

#include "ui_wdg_tool_crop.h"

class KisToolCrop;
class KisToolCropConfigWidget : public QWidget, public Ui::WdgToolCrop
{
    Q_OBJECT

public:
    KisToolCropConfigWidget(QWidget *parent, KisToolCrop* cropTool);

Q_SIGNALS:
    void cropTypeChanged(int cropType);
    void cropXChanged(int newX);
    void cropYChanged(int newY);
    void cropWidthChanged(int newWidth);
    void forceWidthChanged(bool newForce);
    void cropHeightChanged(int newHeight);
    void forceHeightCHanged(bool newForce);
    void ratioChanged(double newRatio);
    void forceRatioChanged(bool newForce);
    void decorationChanged(int newDecoration);
    
public Q_SLOTS:
    void cropTypeChanged();
    void cropTypeSelectableChanged();
    void cropXChanged();
    void cropYChanged();
    void cropWidthChanged();
    void forceWidthChanged();
    void cropHeightChanged();
    void forceHeightCHanged();
    void ratioChanged();
    void forceRatioChanged();
    void decorationChanged();

private:
    KisToolCrop* m_cropTool;
};

#endif // KISTOOLCROPCONFIGWIDGET_H
