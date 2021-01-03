/*
    SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>

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
    void lockWidthChanged(bool newLock);
    void cropHeightChanged(int newHeight);
    void lockHeightChanged(bool newLock);
    void ratioChanged(double newRatio);
    void lockRatioChanged(bool newLock);
    void decorationChanged(int newDecoration);
    void allowGrowChanged(bool newLock);
    void growCenterChanged(bool newLock);

public Q_SLOTS:
    void cropTypeSelectableChanged();

private:
    KisToolCrop* m_cropTool;
};

#endif // KISTOOLCROPCONFIGWIDGET_H
