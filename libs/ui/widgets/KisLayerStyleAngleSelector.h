/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISLAYERSTYLEANGLESELECTOR_H
#define KISLAYERSTYLEANGLESELECTOR_H

#include <QWidget>

#include <ui_wdgKisLayerStyleAngleSelector.h>

class KisLayerStyleAngleSelector : public QWidget
{
    Q_OBJECT

public:
    KisLayerStyleAngleSelector(QWidget* parent);

    int value();
    void setValue(int value);

    void enableGlobalLight(bool enable);

    bool useGlobalLight();
    void setUseGlobalLight(bool state);

    KisAngleSelector* angleSelector();

Q_SIGNALS:
    void valueChanged(int);
    void configChanged();
    void globalAngleChanged(int);

private Q_SLOTS:
    void slotAngleSelectorAngleChanged(qreal value);
    void slotGlobalLightToggled();

private:
    void emitChangeSignals();

    Ui_WdgKisLayerStyleAngleSelector* ui;
    
    bool m_enableGlobalLight;
};

#endif // KISLAYERSTYLEANGLESELECTOR_H
