/*
    Copyright (C) 2012  Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    void forceHeightChanged(bool newForce);
    void ratioChanged(double newRatio);
    void forceRatioChanged(bool newForce);
    void decorationChanged(int newDecoration);
    void allowGrowChanged(bool newForce);
    void growCenterChanged(bool newForce);

public Q_SLOTS:
    void cropTypeSelectableChanged();

private:
    KisToolCrop* m_cropTool;
};

#endif // KISTOOLCROPCONFIGWIDGET_H
